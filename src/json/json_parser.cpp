#include "json_parser.hpp"

#include <string_view>

namespace {
bool is_whitespace(char c) { return static_cast<unsigned char>(c) <= ' '; }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
}

YomitanJSONParser::YomitanJSONParser(std::string_view content) : src_(content), pos_(0) {
  consume_bom();
  size_t first_char = src_.find_first_not_of(" \t\n\r", pos_);
  if (first_char != std::string_view::npos && src_[first_char] == '[') {
    pos_ = first_char + 1;
  }
}

bool YomitanJSONParser::parse_frequency(ParsedFrequency& out) {
  if (pos_ < src_.size() && is_digit(src_[pos_])) {
    out.value = parse_number();
    out.display_value = std::to_string(out.value);
    return true;
  }

  if (!expect('{')) {
    return false;
  }
  pos_++;

  int level = 1;
  while (pos_ < src_.size()) {
    if (src_[pos_] == '}') {
      pos_++;
      level--;
      if (level == 0) {
        break;
      }
      continue;
    }
    std::string_view key = parse_string();
    if (!expect(':')) {
      return false;
    }
    pos_++;
    if (key == "reading") {
      out.reading = parse_string();
      consume_comma();
    } else if (key == "frequency") {
      consume_whitespace();
      if (pos_ < src_.size() && is_digit(src_[pos_])) {
        out.value = parse_number();
        out.display_value = std::to_string(out.value);
        consume_comma();
      } else if (expect('{')) {
        pos_++;
        level++;
      } else {
        return false;
      }
    } else if (key == "value") {
      out.value = parse_number();
      consume_comma();
    } else if (key == "displayValue") {
      out.display_value = parse_string();
      consume_comma();
    }
  }
  return level == 0;
}

bool YomitanJSONParser::parse_pitch(ParsedPitch& out) {
  if (!expect('{')) {
    return false;
  }
  pos_++;

  while (pos_ < src_.size() && src_[pos_] != '}') {
    std::string_view key = parse_string();
    if (!expect(':')) {
      return false;
    }
    pos_++;

    if (key == "reading") {
      out.reading = parse_string();
    } else if (key == "pitches") {
      if (!parse_pitches_array(out.pitches)) {
        return false;
      }
    } else {
      skip();
    }
    consume_comma();
  }
  if (!expect('}')) {
    return false;
  }
  pos_++;
  return true;
}

bool YomitanJSONParser::parse_pitches_array(std::vector<int>& pitches) {
  if (!expect('[')) {
    return false;
  }
  pos_++;

  while (pos_ < src_.size() && src_[pos_] != ']') {
    int position;
    if (!parse_pitch_position(position)) {
      return false;
    }
    pitches.push_back(position);
    consume_comma();
  }
  if (!expect(']')) {
    return false;
  }
  pos_++;
  return true;
}

bool YomitanJSONParser::parse_pitch_position(int& position) {
  if (!expect('{')) {
    return false;
  }
  pos_++;

  position = 0;
  while (pos_ < src_.size() && src_[pos_] != '}') {
    std::string_view key = parse_string();
    if (!expect(':')) {
      return false;
    }
    pos_++;

    // there are a a few more keys, in the three most popular dicts (NHK 2016, 大辞泉, アクセント辞典v2) these seem to
    // be empty, will handle eventually
    if (key == "position") {
      // according to spec this can be a string as well, haven't found a dict using this yet
      position = parse_number();
    } else {
      skip();
    }
    consume_comma();
  }
  if (!expect('}')) {
    return false;
  }
  pos_++;
  return true;
}

void YomitanJSONParser::consume_bom() {
  if (src_.starts_with("\xEF\xBB\xBF")) {
    pos_ = 3;
  }
}

void YomitanJSONParser::consume_whitespace() {
  while (pos_ < src_.size() && is_whitespace(src_[pos_])) {
    pos_++;
  }
}

bool YomitanJSONParser::expect(char c) {
  consume_whitespace();
  return pos_ < src_.size() && src_[pos_] == c;
}

bool YomitanJSONParser::consume_comma() {
  consume_whitespace();
  if (pos_ < src_.size() && src_[pos_] == ',') {
    pos_++;
    return true;
  }
  return false;
}

bool YomitanJSONParser::enter_next_entry() {
  consume_whitespace();
  if (pos_ >= src_.size() || src_[pos_] == ']') {
    return false;
  }
  if (src_[pos_] == ',') {
    pos_++;
    consume_whitespace();
  }
  if (src_[pos_] != '[') {
    return false;
  }
  pos_++;
  return true;
}

void YomitanJSONParser::consume_array_end() {
  while (pos_ < src_.size() && src_[pos_] != ']') {
    pos_++;
  }
  if (pos_ < src_.size()) {
    pos_++;
  }
}

std::string_view YomitanJSONParser::parse_string() {
  consume_whitespace();
  if (pos_ >= src_.size() || src_[pos_] != '"') {
    return "";
  }

  size_t start = ++pos_;
  while (pos_ < src_.size()) {
    if (src_[pos_] == '\\') {
      pos_ += 2;
      continue;
    }
    if (src_[pos_] == '"') {
      std::string_view result = src_.substr(start, pos_ - start);
      pos_++;
      return result;
    }
    pos_++;
  }
  return "";
}

int YomitanJSONParser::parse_number() {
  consume_whitespace();

  int sign = 1;
  if (pos_ < src_.size() && src_[pos_] == '-') {
    sign = -1;
    pos_++;
  }

  int val = 0;
  while (pos_ < src_.size() && is_digit(src_[pos_])) {
    val = val * 10 + (src_[pos_] - '0');
    pos_++;
  }

  return sign * val;
}

std::string_view YomitanJSONParser::extract_single_value() {
  consume_whitespace();
  size_t start = pos_;
  skip();
  return src_.substr(start, pos_ - start);
}

void YomitanJSONParser::skip() {
  consume_whitespace();
  if (pos_ >= src_.size()) {
    return;
  }
  char c = src_[pos_];
  if (c == '"') {
    skip_string();
  } else if (c == '[' || c == '{') {
    skip_bracket();
  } else {
    while (pos_ < src_.size() && src_[pos_] != ',' && src_[pos_] != ']' && src_[pos_] != '}') {
      pos_++;
    }
  }
}

void YomitanJSONParser::skip_string() {
  pos_++;
  while (pos_ < src_.size()) {
    char c = src_[pos_++];
    if (c == '"') {
      return;
    }
    if (c == '\\' && pos_ < src_.size()) {
      pos_++;
    }
  }
}

void YomitanJSONParser::skip_bracket() {
  char open = src_[pos_];
  char close = (open == '[') ? ']' : '}';
  int level = 1;
  pos_++;

  while (pos_ < src_.size() && level > 0) {
    char c = src_[pos_];
    if (c == '"') {
      skip_string();
    } else if (c == open) {
      level++;
      pos_++;
    } else if (c == close) {
      level--;
      pos_++;
    } else {
      pos_++;
    }
  }
}
