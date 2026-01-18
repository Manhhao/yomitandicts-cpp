#include "query.hpp"

#include <sqlite3.h>
#include <zstd.h>

#include <filesystem>
#include <unordered_map>

DictionaryQuery::~DictionaryQuery() {
  for (auto& [name, db, stmt] : dicts_) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }
  for (auto& [name, db, stmt] : freq_dicts_) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }
}

void DictionaryQuery::add_dict(const std::string& db_path) {
  sqlite3* db;
  if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    return;
  }
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db,
                         "SELECT expression, reading, glossary, rules FROM terms WHERE expression = ? OR reading = ?",
                         -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  std::string name = std::filesystem::path(db_path).stem().string();
  dicts_.emplace_back(name, db, stmt);
}

void DictionaryQuery::add_freq_dict(const std::string& db_path) {
  sqlite3* db;
  if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    return;
  }
  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, "SELECT data FROM term_meta WHERE expression = ?", -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  std::string name = std::filesystem::path(db_path).stem().string();
  freq_dicts_.emplace_back(name, db, stmt);
}

std::vector<TermResult> DictionaryQuery::query(const std::string& expression) const {
  std::unordered_map<std::string, TermResult> term_map;
  for (const auto& [name, db, stmt] : dicts_) {
    sqlite3_bind_text(stmt, 1, expression.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, expression.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string expr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      std::string reading = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      std::string definition_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      std::string key = expr;
      key.append("/");
      key.append(reading);

      GlossaryEntry entry;
      entry.dict_name = name;

      const void* blob = sqlite3_column_blob(stmt, 2);
      int blob_size = sqlite3_column_bytes(stmt, 2);
      entry.glossary = decompress_glossary(blob, blob_size);

      auto [it, inserted] = term_map.try_emplace(key);
      if (inserted) {
        it->second = {.expression = expr,
                      .reading = reading,
                      .definition_tags = definition_tags,
                      .glossaries = {},
                      .frequencies = {}};
      } else {
        if (!definition_tags.empty()) {
          if (!it->second.definition_tags.empty()) {
            it->second.definition_tags += " ";
          }
          it->second.definition_tags += definition_tags;
        }
      }
      it->second.glossaries.push_back(std::move(entry));
    }
    sqlite3_reset(stmt);
  }

  std::vector<TermResult> results{};
  results.reserve(term_map.size());
  for (auto& [key, term] : term_map) {
    results.push_back(std::move(term));
  }

  query_freq(results);

  return results;
}

void DictionaryQuery::query_freq(std::vector<TermResult>& terms) const {
  for (auto& term : terms) {
    for (const auto& [name, db, stmt] : freq_dicts_) {
      sqlite3_bind_text(stmt, 1, term.expression.c_str(), -1, SQLITE_STATIC);
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* data = (const char*)sqlite3_column_text(stmt, 0);
        term.frequencies.push_back({name, data ? data : ""});
      }
      sqlite3_reset(stmt);
    }
  }
}

std::string DictionaryQuery::decompress_glossary(const void* data, size_t size) {
  if (!data || size == 0) {
    return "";
  }

  unsigned long long decompressed_size = ZSTD_getFrameContentSize(data, size);
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR || decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    return "";
  }

  std::string result;
  result.resize(decompressed_size);

  size_t actual_size = ZSTD_decompress(result.data(), result.size(), data, size);
  if (ZSTD_isError(actual_size)) {
    return "";
  }

  result.resize(actual_size);
  return result;
}