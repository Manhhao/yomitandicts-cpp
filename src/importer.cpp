#include "yomitandicts/importer.hpp"

#include <sqlite3.h>
#include <zip.h>
#include <zstd.h>

#include <filesystem>
#include <future>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "json/yomitan_parser.hpp"

namespace {
struct MediaFile {
  std::string path;
  std::vector<uint8_t> blob;
};

std::string read_file_by_index(zip_t* archive, int index) {
  if (zip_entry_openbyindex(archive, index) != 0) {
    return "";
  }

  void* raw = nullptr;
  size_t size = 0;
  ssize_t bytes_read = zip_entry_read(archive, &raw, &size);
  zip_entry_close(archive);
  if (bytes_read < 0 || !raw) {
    if (raw) {
      free(raw);
    }
    return "";
  }

  std::unique_ptr<void, decltype(&std::free)> buf(raw, &std::free);
  std::string buffer(static_cast<char*>(buf.get()), size);
  return buffer;
}

std::string read_file_by_name(zip_t* archive, const char* name) {
  if (zip_entry_open(archive, name) != 0) {
    return "";
  }

  void* raw = nullptr;
  size_t size = 0;
  ssize_t bytes_read = zip_entry_read(archive, &raw, &size);
  zip_entry_close(archive);

  if (bytes_read < 0 || !raw) {
    if (raw) {
      free(raw);
    }
    return "";
  }

  std::unique_ptr<void, decltype(&std::free)> buf(raw, &std::free);
  std::string buffer(static_cast<char*>(buf.get()), size);
  return buffer;
}

std::optional<MediaFile> read_media_by_index(zip_t* archive, int index) {
  if (zip_entry_openbyindex(archive, index) != 0) {
    return std::nullopt;
  }
  MediaFile out;

  void* raw = nullptr;
  size_t size = 0;
  ssize_t bytes_read = zip_entry_read(archive, &raw, &size);
  out.path = zip_entry_name(archive);
  zip_entry_close(archive);
  if (bytes_read < 0 || !raw) {
    if (raw) {
      free(raw);
    }
    return std::nullopt;
  }

  std::unique_ptr<void, decltype(&free)> buf(raw, free);
  auto* p = static_cast<std::uint8_t*>(buf.get());
  out.blob.assign(p, p + size);
  return out;
}

std::vector<int> get_files(zip_t* archive, std::string_view prefix) {
  std::vector<int> indices;
  ssize_t num_entries = zip_entries_total(archive);
  if (num_entries < 0) {
    return indices;
  }

  for (int i = 0; i < num_entries; ++i) {
    if (zip_entry_openbyindex(archive, i) != 0) {
      continue;
    }

    const char* raw_name = zip_entry_name(archive);
    if (raw_name != nullptr) {
      std::string_view name(raw_name);
      if (name.starts_with(prefix)) {
        indices.push_back(i);
      }
    }
    zip_entry_close(archive);
  }

  return indices;
}

std::vector<int> get_media_files(zip_t* archive) {
  std::vector<int> indices;
  ssize_t num_entries = zip_entries_total(archive);
  if (num_entries < 0) {
    return indices;
  }

  for (int i = 0; i < num_entries; ++i) {
    if (zip_entry_openbyindex(archive, i) != 0) {
      continue;
    }

    if (zip_entry_isdir(archive) == 1) {
      zip_entry_close(archive);
      continue;
    }

    const char* raw_name = zip_entry_name(archive);
    if (raw_name != nullptr) {
      std::string_view name(raw_name);
      if (!(name == "styles.css" || name == "index.json" || name.starts_with("term_bank_") ||
            name.starts_with("term_meta_bank_") || name.starts_with("tag_bank_"))) {
        indices.push_back(i);
      }
    }
    zip_entry_close(archive);
  }

  return indices;
}

std::vector<char> compress_glossary(const char* src, size_t size) {
  size_t bound = ZSTD_compressBound(size);
  std::vector<char> compressed(bound);
  size_t compressed_size = ZSTD_compress(compressed.data(), bound, src, size, 1);

  if (ZSTD_isError(compressed_size)) {
    return {};
  }
  compressed.resize(compressed_size);
  return compressed;
}

void init_db(sqlite3* db) {
  sqlite3_exec(db,
               "PRAGMA journal_mode=MEMORY;"
               "PRAGMA synchronous=OFF;"
               "PRAGMA temp_store=MEMORY;"
               "PRAGMA cache_size=-100000;",
               nullptr, nullptr, nullptr);

  sqlite3_exec(db, R"(
            CREATE TABLE info (title TEXT, revision TEXT, format INTEGER, styles TEXT);
            CREATE TABLE terms (expression TEXT, reading TEXT, definition_tags TEXT, rules TEXT, score INTEGER, glossary BLOB, sequence INTEGER, term_tags TEXT);
            CREATE TABLE term_meta (expression TEXT, mode TEXT, data TEXT);
            CREATE TABLE tags (name TEXT, category TEXT, sort_order INTEGER, notes TEXT, score INTEGER);
            CREATE TABLE media (path TEXT, data BLOB);
        )",
               nullptr, nullptr, nullptr);
}

void store_index(sqlite3* db, const Index& index, const std::string& styles) {
  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, "INSERT INTO info VALUES (?, ?, ?, ?)", -1, &stmt, nullptr);
  sqlite3_bind_text(stmt, 1, index.title.data(), static_cast<int>(index.title.size()), SQLITE_STATIC);
  sqlite3_bind_text(stmt, 2, index.revision.data(), static_cast<int>(index.revision.size()), SQLITE_STATIC);
  sqlite3_bind_int(stmt, 3, index.format);
  if (!styles.empty()) {
    sqlite3_bind_text(stmt, 4, styles.c_str(), -1, SQLITE_STATIC);
  } else {
    sqlite3_bind_null(stmt, 4);
  }
  sqlite3_step(stmt);
  sqlite3_finalize(stmt);
}

void store_terms(sqlite3* db, zip_t* archive, const std::vector<int>& files, ImportResult& result) {
  if (files.empty()) {
    return;
  }

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, "INSERT INTO terms VALUES (?, ?, ?, ?, ?, ?, ?, ?)", -1, &stmt, nullptr);

  const size_t num_threads = std::max(1U, std::thread::hardware_concurrency() - 1);
  for (int index : files) {
    std::string content = read_file_by_index(archive, index);
    if (content.empty()) {
      continue;
    }

    std::vector<Term> out;
    if (!yomitan_parser::parse_term_bank(content, out)) {
      continue;
    }

    std::vector<std::vector<char>> glossaries(out.size());
    std::vector<std::future<void>> futures;
    size_t chunk_size = std::max<size_t>(1, out.size() / num_threads);
    for (size_t i = 0; i < num_threads; i++) {
      size_t start = i * chunk_size;
      size_t end = (i == num_threads - 1) ? out.size() : std::min(out.size(), start + chunk_size);
      if (start >= out.size()) {
        break;
      }
      futures.push_back(std::async(std::launch::async, [&glossaries, &out, start, end] {
        for (size_t t = start; t < end; t++) {
          glossaries[t] = compress_glossary(out[t].glossary.str.data(), out[t].glossary.str.size());
        }
      }));
    }
    for (const auto& f : futures) {
      f.wait();
    }

    for (size_t i = 0; i < out.size(); ++i) {
      const auto& term = out[i];
      sqlite3_bind_text(stmt, 1, term.expression.data(), static_cast<int>(term.expression.size()), SQLITE_STATIC);
      if (term.reading.empty()) {
        sqlite3_bind_text(stmt, 2, term.expression.data(), static_cast<int>(term.expression.size()), SQLITE_STATIC);
      } else {
        sqlite3_bind_text(stmt, 2, term.reading.data(), static_cast<int>(term.reading.size()), SQLITE_STATIC);
      }
      sqlite3_bind_text(stmt, 3, term.definition_tags.data(), static_cast<int>(term.definition_tags.size()),
                        SQLITE_STATIC);
      sqlite3_bind_text(stmt, 4, term.rules.data(), static_cast<int>(term.rules.size()), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 5, term.score);

      sqlite3_bind_blob(stmt, 6, glossaries[i].data(), static_cast<int>(glossaries[i].size()), SQLITE_STATIC);

      sqlite3_bind_int(stmt, 7, term.sequence);
      sqlite3_bind_text(stmt, 8, term.term_tags.data(), static_cast<int>(term.term_tags.size()), SQLITE_STATIC);

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
      result.term_count++;
    }
  }
  sqlite3_finalize(stmt);
}

void store_meta(sqlite3* db, zip_t* archive, const std::vector<int>& files, ImportResult& result) {
  if (files.empty()) {
    return;
  }

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, "INSERT INTO term_meta VALUES (?, ?, ?)", -1, &stmt, nullptr);

  for (int index : files) {
    std::string content = read_file_by_index(archive, index);
    if (content.empty()) {
      continue;
    }

    std::vector<Meta> out;
    if (!yomitan_parser::parse_meta_bank(content, out)) {
      continue;
    }

    for (const auto& meta : out) {
      sqlite3_bind_text(stmt, 1, meta.expression.data(), static_cast<int>(meta.expression.size()), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, meta.mode.data(), static_cast<int>(meta.mode.size()), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 3, meta.data.str.data(), static_cast<int>(meta.data.str.size()), SQLITE_STATIC);

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
      result.meta_count++;
    }
  }
  sqlite3_finalize(stmt);
}

void store_tags(sqlite3* db, zip_t* archive, const std::vector<int>& files, ImportResult& result) {
  if (files.empty()) {
    return;
  }

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, "INSERT INTO tags VALUES (?, ?, ?, ?, ?)", -1, &stmt, nullptr);

  for (int index : files) {
    std::string content = read_file_by_index(archive, index);
    if (content.empty()) {
      continue;
    }

    std::vector<Tag> out;
    if (!yomitan_parser::parse_tag_bank(content, out)) {
      continue;
    }

    for (const auto& tag : out) {
      sqlite3_bind_text(stmt, 1, tag.name.data(), static_cast<int>(tag.name.size()), SQLITE_STATIC);
      sqlite3_bind_text(stmt, 2, tag.category.data(), static_cast<int>(tag.category.size()), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 3, tag.order);
      sqlite3_bind_text(stmt, 4, tag.notes.data(), static_cast<int>(tag.notes.size()), SQLITE_STATIC);
      sqlite3_bind_int(stmt, 5, tag.score);

      sqlite3_step(stmt);
      sqlite3_reset(stmt);
      result.tag_count++;
    }
  }
  sqlite3_finalize(stmt);
}

void store_media(sqlite3* db, zip_t* archive, const std::vector<int>& files, ImportResult& result) {
  if (files.empty()) {
    return;
  }

  sqlite3_stmt* stmt;
  sqlite3_prepare_v2(db, "INSERT INTO media VALUES (?, ?)", -1, &stmt, nullptr);

  for (int index : files) {
    auto media = read_media_by_index(archive, index);
    if (!media.has_value()) {
      continue;
    }

    sqlite3_bind_text(stmt, 1, media->path.data(), static_cast<int>(media->path.size()), SQLITE_STATIC);
    sqlite3_bind_blob(stmt, 2, media->blob.data(), static_cast<int>(media->blob.size()), SQLITE_STATIC);

    sqlite3_step(stmt);
    sqlite3_reset(stmt);
    result.media_count++;
  }
  sqlite3_finalize(stmt);
}
}

ImportResult dictionary_importer::import(const std::string& zip_path, const std::string& output_dir) {
  ImportResult result;
  zip_t* archive = nullptr;
  sqlite3* db = nullptr;
  try {
    archive = zip_open(zip_path.c_str(), 0, 'r');
    if (!archive) {
      throw std::runtime_error("failed to open zip");
    }

    std::string index_content = read_file_by_name(archive, "index.json");
    if (index_content.empty()) {
      throw std::runtime_error("could not find or read index.json");
    }

    Index index;
    if (!yomitan_parser::parse_index(index_content, index)) {
      throw std::runtime_error("failed to parse index.json");
    }

    result.title = index.title;

    std::filesystem::create_directories(output_dir);
    std::filesystem::path db_path = std::filesystem::path(output_dir) / (result.title + ".db");
    std::string path = db_path.string();

    if (std::filesystem::exists(path)) {
      std::filesystem::remove(path);
    }

    if (sqlite3_open(path.c_str(), &db) != SQLITE_OK) {
      throw std::runtime_error("error opening db");
    }

    init_db(db);

    std::string styles = read_file_by_name(archive, "styles.css");

    store_index(db, index, styles);

    auto term_banks = get_files(archive, "term_bank_");
    auto meta_banks = get_files(archive, "term_meta_bank_");
    auto tag_banks = get_files(archive, "tag_bank_");
    auto media = get_media_files(archive);

    sqlite3_exec(db, "BEGIN", nullptr, nullptr, nullptr);
    store_terms(db, archive, term_banks, result);
    store_meta(db, archive, meta_banks, result);
    store_tags(db, archive, tag_banks, result);
    store_media(db, archive, media, result);
    sqlite3_exec(db, "COMMIT", nullptr, nullptr, nullptr);
    result.success = true;

    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_terms_expression ON terms(expression);", nullptr, nullptr,
                 nullptr);
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_terms_reading ON terms(reading);", nullptr, nullptr, nullptr);
    sqlite3_exec(db, "CREATE INDEX IF NOT EXISTS idx_meta_expression_mode ON term_meta(expression, mode);", nullptr,
                 nullptr, nullptr);

    sqlite3_exec(db, "PRAGMA analyze;", nullptr, nullptr, nullptr);
  } catch (const std::exception& e) {
    result.success = false;
    result.errors.emplace_back(e.what());
  }

  if (archive) {
    zip_close(archive);
  }

  if (db) {
    sqlite3_close(db);
  }

  return result;
}