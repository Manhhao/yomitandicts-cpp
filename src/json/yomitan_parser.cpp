#include "yomitan_parser.hpp"

#include <string_view>
#include <variant>

template <>
struct glz::meta<Index> {
  using T = Index;
  static constexpr auto value = object("title", glz::raw_string<&T::title>, "revision", glz::raw_string<&T::revision>,
                                       "format", &T::format, "isUpdatable", &T::updatable, "indexUrl",
                                       glz::raw_string<&T::index_url>, "updateUrl", glz::raw_string<&T::download_url>);
};

template <>
struct glz::meta<Term> {
  using T = Term;
  static constexpr auto value =
      array(glz::raw_string<&T::expression>, glz::raw_string<&T::reading>, glz::raw_string<&T::definition_tags>,
            glz::raw_string<&T::rules>, &T::score, &T::glossary, &T::sequence, glz::raw_string<&T::term_tags>);
};

template <>
struct glz::meta<Meta> {
  using T = Meta;
  static constexpr auto value =
      array(glz::raw_string<&T::expression>, glz::raw_string<&T::mode>, glz::raw_string<&T::data>);
};

template <>
struct glz::meta<Tag> {
  using T = Tag;
  static constexpr auto value =
      array(glz::raw_string<&T::name>, glz::raw_string<&T::category>, &T::order, glz::raw_string<&T::notes>, &T::score);
};

namespace internal {
struct FrequencyValue {
  int value;
  std::string display_value;
};

struct ParsedFrequency {
  std::optional<std::string_view> reading;
  std::variant<int, FrequencyValue> frequency;
};

struct PitchesArray {
  int position = 0;
};

struct ParsedPitch {
  std::string_view reading;
  std::vector<PitchesArray> pitches;
};
};

template <>
struct glz::meta<internal::FrequencyValue> {
  using T = internal::FrequencyValue;
  static constexpr auto value = object("value", &T::value, "displayValue", &T::display_value);
};

template <>
struct glz::meta<internal::ParsedFrequency> {
  using T = internal::ParsedFrequency;
  static constexpr auto value = object("reading", &T::reading, "frequency", &T::frequency);
};

template <>
struct glz::meta<internal::PitchesArray> {
  using T = internal::PitchesArray;
  static constexpr auto value = object("position", &T::position);
};

template <>
struct glz::meta<internal::ParsedPitch> {
  using T = internal::ParsedPitch;
  static constexpr auto value = object("reading", glz::raw_string<&T::reading>, "pitches", &T::pitches);
};

bool yomitan_parser::parse_index(std::string_view content, Index& out) {
  auto error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(out, content);
  return !error;
}

bool yomitan_parser::parse_term_bank(std::string_view content, std::vector<Term>& out) {
  auto error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(out, content);
  return !error;
}

bool yomitan_parser::parse_meta_bank(std::string_view content, std::vector<Meta>& out) {
  auto error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(out, content);
  return !error;
}

bool yomitan_parser::parse_tag_bank(std::string_view content, std::vector<Tag>& out) {
  auto error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(out, content);
  return !error;
}

bool yomitan_parser::parse_frequency(std::string_view content, ParsedFrequency& out) {
  int val;
  auto error = glz::read_json(val, content);
  if (!error) {
    out.value = val;
    out.display_value = std::to_string(val);
    out.reading = "";
    return true;
  }

  internal::ParsedFrequency parsed;
  error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(parsed, content);
  if (error) {
    return false;
  }

  out.reading = parsed.reading.value_or("");
  if (std::holds_alternative<int>(parsed.frequency)) {
    int freq = std::get<int>(parsed.frequency);
    out.value = freq;
    out.display_value = std::to_string(freq);
  } else {
    auto& freq = std::get<internal::FrequencyValue>(parsed.frequency);
    out.value = freq.value;
    out.display_value = freq.display_value.empty() ? std::to_string(freq.value) : freq.display_value;
  }
  return true;
}

bool yomitan_parser::parse_pitch(std::string_view content, ParsedPitch& out) {
  internal::ParsedPitch parsed;
  auto error = glz::read<glz::opts{.error_on_unknown_keys = false, .error_on_missing_keys = false}>(parsed, content);
  if (error) {
    return false;
  }

  out.reading = parsed.reading;
  out.pitches =
      parsed.pitches | std::views::transform(&internal::PitchesArray::position) | std::ranges::to<std::vector>();
  return true;
}