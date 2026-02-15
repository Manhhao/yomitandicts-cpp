#include "yomitan_parser.hpp"

#include <glaze/glaze.hpp>

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
  static constexpr auto value = array(
      glz::raw_string<&T::expression>, glz::raw_string<&T::reading>, glz::raw_string<&T::definition_tags>,
      glz::raw_string<&T::rules>, &T::score,
      // this line of code is a certified c++ moment
      [](T& t) -> glz::raw_json_view& { return reinterpret_cast<glz::raw_json_view&>(t.glossary); }, &T::sequence,
      glz::raw_string<&T::term_tags>);
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