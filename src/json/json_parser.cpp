#include "json_parser.hpp"

namespace {
bool is_whitespace(char c) { return (unsigned char)c <= ' '; }
bool is_digit(char c) { return c >= '0' && c <= '9'; }
}

YomitanJSONParser::YomitanJSONParser(std::string_view content) : src_(content), pos_(0) {
  consume_bom();
  size_t first_char = src_.find_first_not_of(" \t\n\r", pos_);
  if (first_char != std::string_view::npos && src_[first_char] == '[') {
    pos_ = first_char + 1;
  }
}

bool YomitanJSONParser::parse_index(Index& out) {
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

    if (key == "title") {
      out.title = parse_string();
    } else if (key == "revision") {
      out.revision = parse_string();
    } else if (key == "format") {
      out.version = parse_number();
    } else {
      skip();
    }

    consume_comma();
  }
  return true;
}

bool YomitanJSONParser::parse_term(Term& out) {
  if (!enter_next_entry()) {
    return false;
  }

  out.expression = parse_string();
  consume_comma();

  out.reading = parse_string();
  if (out.reading.empty()) {
    out.reading = out.expression;
  }
  consume_comma();

  out.definition_tags = parse_string();
  consume_comma();

  out.rules = parse_string();
  consume_comma();

  out.score = parse_number();
  consume_comma();

  out.glossary = extract_single_value();
  consume_comma();

  out.sequence = parse_number();
  consume_comma();

  out.term_tags = parse_string();

  consume_array_end();
  return true;
}

bool YomitanJSONParser::parse_meta(Meta& out) {
  if (!enter_next_entry()) {
    return false;
  }

  out.expression = parse_string();
  consume_comma();

  out.mode = parse_string();
  consume_comma();

  out.data = extract_single_value();

  consume_array_end();
  return true;
}

bool YomitanJSONParser::parse_tag(Tag& out) {
  if (!enter_next_entry()) {
    return false;
  }

  out.name = parse_string();
  consume_comma();

  out.category = parse_string();
  consume_comma();

  out.order = parse_number();
  consume_comma();

  out.notes = parse_string();
  consume_comma();

  out.score = parse_number();

  consume_array_end();
  return true;
}

void YomitanJSONParser::consume_bom() {
  if (src_.substr(0, 3) == "\xEF\xBB\xBF") {
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
