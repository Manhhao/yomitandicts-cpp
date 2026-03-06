#pragma once

#include <optional>
#include <string>
#include <unordered_set>
#include <vector>

struct DeconjugationForm {
  std::string text;
  std::string original_text;
  std::vector<std::string> tags;
  std::unordered_set<std::string> seen_text;
  std::vector<std::string> process;
};

class Deconjugator {
 public:
  Deconjugator();
  std::vector<DeconjugationForm> deconjugate(const std::string& text) const;

  struct VirtualRule {
    std::string dec_end;
    std::string con_end;
    std::string dec_tag;
    std::string con_tag;
  };

  struct Rule {
    std::string type;
    std::optional<std::string> context_rule;
    std::string detail;
    std::vector<VirtualRule> virtual_rules;
  };

 private:
  std::vector<Rule> rules_;
};
