#pragma once
#include <string_view>
#include <vector>
#include <string>

struct Index {
  std::string_view title;
  std::string_view revision;
  bool is_freq_dict;
  int version = 3;
};

struct Term {
  std::string_view expression;
  std::string_view reading;
  std::string_view definition_tags;
  std::string_view rules;
  int score = 0;
  std::string_view glossary;
  int sequence = 0;
  std::string_view term_tags;
};

struct Meta {
  std::string_view expression;
  std::string_view mode;
  std::string_view data;
};

struct Tag {
  std::string_view name;
  std::string_view category;
  int order = 0;
  std::string_view notes;
  int score = 0;
};

struct ParsedFrequency {
  std::string_view reading;
  int value;
  std::string_view display_value;
};

struct ParsedPitch {
  std::string_view reading;
  std::vector<int> pitches;
};

class YomitanJSONParser {
 public:
  YomitanJSONParser(std::string_view content);
  bool parse_index(Index& out);
  bool parse_term(Term& out);
  bool parse_meta(Meta& out);
  bool parse_tag(Tag& out);
  bool parse_frequency(ParsedFrequency& out);
  bool parse_pitch(ParsedPitch& out);

 private:
  void consume_bom();
  void consume_whitespace();
  bool consume_comma();
  bool enter_next_entry();
  void consume_array_end();
  bool expect(char c);
  std::string_view parse_string();
  int parse_number();
  std::string_view extract_single_value();
  void skip();
  void skip_string();
  void skip_bracket();
  bool parse_pitch_position(int& position);
  bool parse_pitches_array(std::vector<int>& pitches);

  std::string_view src_;
  size_t pos_;
};