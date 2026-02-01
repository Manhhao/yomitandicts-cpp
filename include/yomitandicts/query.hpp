#pragma once

#include <string>
#include <vector>

struct sqlite3;
struct sqlite3_stmt;

struct Frequency {
  int value;
  std::string display_value;
};

struct DictionaryStyle {
  std::string dict_name;
  std::string styles;
};

struct GlossaryEntry {
  std::string dict_name;
  std::string glossary;
  std::string definition_tags;
  std::string term_tags;
};

struct FrequencyEntry {
  std::string dict_name;
  std::vector<Frequency> frequencies;
};

struct PitchEntry {
  std::string dict_name;
  std::vector<int> pitch_positions;
};

struct TermResult {
  std::string expression;
  std::string reading;
  std::string definition_tags;
  std::string rules;
  std::vector<GlossaryEntry> glossaries;
  std::vector<FrequencyEntry> frequencies;
  std::vector<PitchEntry> pitches;
};

class DictionaryQuery {
 public:
  DictionaryQuery() = default;
  ~DictionaryQuery();

  DictionaryQuery(const DictionaryQuery&) = delete;
  DictionaryQuery& operator=(const DictionaryQuery&) = delete;

  DictionaryQuery(DictionaryQuery&&) = default;
  DictionaryQuery& operator=(DictionaryQuery&&) = default;

  void add_dict(const std::string& db_path);
  void add_freq_dict(const std::string& db_path);
  void add_pitch_dict(const std::string& db_path);

  std::vector<TermResult> query(const std::string& expression) const;

  std::vector<DictionaryStyle> get_styles() const;

 private:
  struct Dictionary {
    std::string name;
    std::string styles;
    sqlite3* db;
    sqlite3_stmt* stmt;
  };

  void query_freq(std::vector<TermResult>& terms) const;
  void query_pitch(std::vector<TermResult>& terms) const;

  static std::string decompress_glossary(const void* data, size_t size);
  std::vector<Dictionary> dicts_;
  std::vector<Dictionary> freq_dicts_;
  std::vector<Dictionary> pitch_dicts_;
};
