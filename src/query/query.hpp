#pragma once

#include <sqlite3.h>

#include <string>
#include <vector>

struct GlossaryEntry {
  std::string dict_name;
  std::string glossary;
};

struct FrequencyEntry {
  std::string dict_name;
  std::string data;
};

struct TermResult {
  std::string expression;
  std::string reading;
  std::string definition_tags;
  std::vector<GlossaryEntry> glossaries;
  std::vector<FrequencyEntry> frequencies;
};

class DictionaryQuery {
 public:
  ~DictionaryQuery();

  void add_dict(const std::string& db_path);
  void add_freq_dict(const std::string& db_path);

  std::vector<TermResult> query(const std::string& expression);

  void close();

 private:
  struct Dictionary {
    std::string name;
    sqlite3* db;
    sqlite3_stmt* stmt;
  };
  static std::string decompress_glossary(const void* data, size_t size);
  std::vector<Dictionary> dicts_;
  std::vector<Dictionary> freq_dicts_;
};