#pragma once
#include <string_view>
#include <vector>

struct ParsedFrequency {
  std::string_view reading;
  int value;
  std::string display_value;
};

struct ParsedPitch {
  std::string_view reading;
  std::vector<int> pitches;
};

class YomitanJSONParser {
 public:
  YomitanJSONParser(std::string_view content);
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