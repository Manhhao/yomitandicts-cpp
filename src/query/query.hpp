#pragma once

#include <sqlite3.h>

#include <string>
#include <vector>

struct GlossaryEntry {
  std::string dict_name;
  std::string glossary;
};

struct TermResult {
  std::string expression;
  std::string reading;
  std::vector<GlossaryEntry> glossaries;
  int frequency;
};

class DictionaryQuery {
 public:
  ~DictionaryQuery();

  void add_dict(const std::string& db_path);
  void add_freq_dict(const std::string& db_path);

  std::vector<TermResult> query(const std::string& expression);

  void close();

 private:
  std::vector<sqlite3*> dicts_;
  std::vector<sqlite3*> freq_dicts_;
};