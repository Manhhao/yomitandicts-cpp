#include "yomitandicts/query.hpp"

#include <sqlite3.h>
#include <zstd.h>

#include <filesystem>
#include <map>

#include "json/json_parser.hpp"

DictionaryQuery::~DictionaryQuery() {
  for (auto& [name, styles, db, stmt] : dicts_) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }
  for (auto& [name, styles, db, stmt] : freq_dicts_) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }
  for (auto& [name, styles, db, stmt] : pitch_dicts_) {
    sqlite3_finalize(stmt);
    sqlite3_close(db);
  }
}

void DictionaryQuery::add_dict(const std::string& db_path) {
  sqlite3* db;
  if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    return;
  }

  std::string name;
  std::string styles;
  sqlite3_stmt* info_stmt;
  if (sqlite3_prepare_v2(db, "SELECT title, styles FROM info LIMIT 1", -1, &info_stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(info_stmt) == SQLITE_ROW) {
      const char* title_text = reinterpret_cast<const char*>(sqlite3_column_text(info_stmt, 0));
      const char* style_text = reinterpret_cast<const char*>(sqlite3_column_text(info_stmt, 1));

      if (title_text) {
        name = title_text;
      }
      if (style_text) {
        styles = style_text;
      }
    }
  }
  sqlite3_finalize(info_stmt);

  if (name.empty()) {
    name = std::filesystem::path(db_path).stem().string();
  }

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db,
                         "SELECT expression, reading, definition_tags, rules, glossary, term_tags "
                         "FROM terms WHERE expression = ? OR reading = ?",
                         -1, &stmt, nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  dicts_.emplace_back(std::move(name), std::move(styles), db, stmt);
}

void DictionaryQuery::add_freq_dict(const std::string& db_path) {
  sqlite3* db;
  if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    return;
  }

  std::string name;
  sqlite3_stmt* info_stmt;
  if (sqlite3_prepare_v2(db, "SELECT title FROM info LIMIT 1", -1, &info_stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(info_stmt) == SQLITE_ROW) {
      const char* title_text = reinterpret_cast<const char*>(sqlite3_column_text(info_stmt, 0));

      if (title_text) {
        name = title_text;
      }
    }
  }
  sqlite3_finalize(info_stmt);

  if (name.empty()) {
    name = std::filesystem::path(db_path).stem().string();
  }

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, "SELECT data FROM term_meta WHERE expression = ? AND mode = 'freq'", -1, &stmt, nullptr) !=
      SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  freq_dicts_.emplace_back(std::move(name), "", db, stmt);
}

void DictionaryQuery::add_pitch_dict(const std::string& db_path) {
  sqlite3* db;
  if (sqlite3_open_v2(db_path.c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
    return;
  }

  std::string name;
  sqlite3_stmt* info_stmt;
  if (sqlite3_prepare_v2(db, "SELECT title FROM info LIMIT 1", -1, &info_stmt, nullptr) == SQLITE_OK) {
    if (sqlite3_step(info_stmt) == SQLITE_ROW) {
      const char* title_text = reinterpret_cast<const char*>(sqlite3_column_text(info_stmt, 0));

      if (title_text) {
        name = title_text;
      }
    }
  }
  sqlite3_finalize(info_stmt);

  if (name.empty()) {
    name = std::filesystem::path(db_path).stem().string();
  }

  sqlite3_stmt* stmt;
  if (sqlite3_prepare_v2(db, "SELECT data FROM term_meta WHERE expression = ? AND mode = 'pitch'", -1, &stmt,
                         nullptr) != SQLITE_OK) {
    sqlite3_close(db);
    return;
  }
  pitch_dicts_.emplace_back(std::move(name), "", db, stmt);
}

std::vector<TermResult> DictionaryQuery::query(const std::string& expression) const {
  std::map<std::pair<std::string, std::string>, TermResult> term_map;
  for (const auto& [name, styles, db, stmt] : dicts_) {
    sqlite3_bind_text(stmt, 1, expression.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, expression.c_str(), -1, SQLITE_STATIC);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      std::string expr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
      std::string reading = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
      std::string definition_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
      std::string rules = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
      std::string term_tags = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));

      GlossaryEntry entry;
      entry.dict_name = name;
      entry.definition_tags = definition_tags;
      entry.term_tags = term_tags;

      const void* blob = sqlite3_column_blob(stmt, 4);
      int blob_size = sqlite3_column_bytes(stmt, 4);
      entry.glossary = decompress_glossary(blob, blob_size);

      auto [it, inserted] = term_map.try_emplace({expr, reading});
      if (inserted) {
        it->second = {.expression = expr,
                      .reading = reading,
                      .definition_tags = definition_tags,
                      .rules = rules,
                      .glossaries = {},
                      .frequencies = {}};
      } else {
        if (!definition_tags.empty()) {
          if (!it->second.definition_tags.empty()) {
            it->second.definition_tags += " ";
          }
          it->second.definition_tags += definition_tags;
        }
        if (!rules.empty()) {
          if (!it->second.rules.empty()) {
            it->second.rules += " ";
          }
          it->second.rules += rules;
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
  query_pitch(results);

  return results;
}

void DictionaryQuery::query_freq(std::vector<TermResult>& terms) const {
  for (auto& term : terms) {
    for (const auto& [name, styles, db, stmt] : freq_dicts_) {
      sqlite3_bind_text(stmt, 1, term.expression.c_str(), -1, SQLITE_STATIC);
      std::vector<Frequency> frequencies;
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        ParsedFrequency parsed;
        YomitanJSONParser parser(data);

        if (parser.parse_frequency(parsed)) {
          if (!parsed.reading.empty() && parsed.reading != term.reading) {
            continue;
          }
          frequencies.emplace_back(
              Frequency{.value = parsed.value, .display_value = std::string(parsed.display_value)});
        }
      }
      sqlite3_reset(stmt);

      if (!frequencies.empty()) {
        term.frequencies.emplace_back(FrequencyEntry{.dict_name = name, .frequencies = std::move(frequencies)});
      }
    }
  }
}

void DictionaryQuery::query_pitch(std::vector<TermResult>& terms) const {
  for (auto& term : terms) {
    for (const auto& [name, styles, db, stmt] : pitch_dicts_) {
      sqlite3_bind_text(stmt, 1, term.expression.c_str(), -1, SQLITE_STATIC);
      std::vector<int> pitch_positions;
      while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* data = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));

        ParsedPitch parsed;
        YomitanJSONParser parser(data);

        if (parser.parse_pitch(parsed)) {
          if (!parsed.reading.empty() && parsed.reading != term.reading) {
            continue;
          }
          pitch_positions.insert(pitch_positions.end(), parsed.pitches.begin(), parsed.pitches.end());
        }
      }
      sqlite3_reset(stmt);

      if (!pitch_positions.empty()) {
        term.pitches.emplace_back(PitchEntry {.dict_name = name, .pitch_positions = std::move(pitch_positions)});
      }
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

std::vector<DictionaryStyle> DictionaryQuery::get_styles() const {
  std::vector<DictionaryStyle> styles;
  styles.reserve(dicts_.size());
  for (const auto& d : dicts_) {
    if (!d.styles.empty()) {
      styles.emplace_back(d.name, d.styles);
    }
  }
  return styles;
}

std::vector<std::string> DictionaryQuery::get_freq_dict_order() const {
  std::vector<std::string> names;
  names.reserve(freq_dicts_.size());
  for (const auto& d : freq_dicts_) {
    names.push_back(d.name);
  }
  return names;
}
