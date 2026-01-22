#include "yomitandicts/lookup.hpp"

#include <algorithm>
#include <map>
#include <sstream>

#include "utf8.hpp"

namespace {
std::vector<std::string> split_whitespace(const std::string& str) {
  std::vector<std::string> result;
  std::istringstream iss(str);
  std::string token;
  while (iss >> token) {
    result.push_back(std::move(token));
  }
  return result;
}

int get_sort_freq(const TermResult& t) {
  if (t.frequencies.empty() || t.frequencies[0].frequencies.empty()) {
    return INT_MAX;
  }
  int min_freq = INT_MAX;
  for (const auto& f : t.frequencies[0].frequencies) {
    min_freq = std::min(min_freq, f.value);
  }
  return min_freq;
}
}

std::vector<LookupResult> Lookup::lookup(const std::string& lookup_string, int max_results, size_t scan_length) const {
  std::map<std::pair<std::string, std::string>, LookupResult> result_map;
  size_t text_len = utf8::length(lookup_string);
  for (size_t i = std::min(scan_length, text_len); i > 0; i--) {
    std::string search_str = lookup_string.substr(0, utf8::byte_position(lookup_string, i));

    auto deinflection_results = deinflector_.deinflect(search_str);
    for (auto& deinflection : deinflection_results) {
      auto terms = query_.query(deinflection.text);

      std::vector<TermResult> term_results = filter_by_pos(terms, deinflection);
      for (const auto& term : term_results) {
        // deduplicate glossaries
        auto key = std::make_pair(term.expression, term.reading);
        auto it = result_map.find(key);
        if (it != result_map.end()) {
          // we only need the longest matched form
          if (utf8::length(search_str) > utf8::length(it->second.matched)) {
            it->second = LookupResult{.matched = search_str,
                                      .deinflected = deinflection.text,
                                      .trace = deinflection.trace,
                                      .term = term,
                                      .sort_freq = get_sort_freq(term)};
          }
        } else {
          result_map.emplace(key, LookupResult{.matched = search_str,
                                               .deinflected = deinflection.text,
                                               .trace = deinflection.trace,
                                               .term = term,
                                               .sort_freq = get_sort_freq(term)});
        }
      }
    }
  }

  std::vector<LookupResult> results;
  results.reserve(result_map.size());
  for (auto& [key, result] : result_map) {
    results.push_back(std::move(result));
  }

  auto middle_iter = std::ranges::next(results.begin(), max_results, results.end());
  std::ranges::partial_sort(results, middle_iter, [](const auto& a, const auto& b) {
    auto len_a = utf8::length(a.matched);
    auto len_b = utf8::length(b.matched);
    if (len_a != len_b) {
      return len_a > len_b;
    }

    return a.sort_freq < b.sort_freq;
  });

  if (results.size() > max_results) {
    results.resize(max_results);
  }

  return results;
}

std::vector<TermResult> Lookup::filter_by_pos(const std::vector<TermResult>& terms, const DeinflectionResult& d) {
  std::vector<TermResult> filtered_results{};
  for (const auto& term : terms) {
    auto dict_conditions = Deinflector::pos_to_conditions(split_whitespace(term.definition_tags));
    // this should support dictionaries without deinflection support because:
    // word needs conditions -> dict has an entry but doesn't have conditions
    // => we give the dict the benefit of the doubt
    // if a dict does define conditions, we execute the normal check
    if (d.conditions != 0 && dict_conditions != 0 && (dict_conditions & d.conditions) == 0) {
      continue;
    }

    filtered_results.push_back(term);
  }
  return filtered_results;
}