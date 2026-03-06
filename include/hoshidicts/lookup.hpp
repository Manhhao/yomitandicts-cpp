#pragma once

#include <string>
#include <vector>

#include "deconjugator.hpp"
#include "query.hpp"

struct LookupResult {
  std::string matched;
  std::string deinflected;
  std::vector<std::string> process;
  TermResult term;
  int preprocessor_steps;
};

class Lookup {
 public:
  Lookup(DictionaryQuery& query, Deconjugator& deconjugator) : query_(query), deconjugator_(deconjugator) {};
  std::vector<LookupResult> lookup(const std::string& lookup_string, int max_results = 16,
                                   size_t scan_length = 16) const;

 private:
  static void filter_by_pos(std::vector<TermResult>& terms, const DeconjugationForm& form);

  DictionaryQuery& query_;
  Deconjugator& deconjugator_;
};