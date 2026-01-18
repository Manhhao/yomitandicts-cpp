#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct TransformGroup {
  std::string name;
  std::string description;
};

struct DeinflectionResult {
  std::string text;
  uint32_t conditions;
  std::vector<TransformGroup> trace;
};

class Deinflector {
 public:
  Deinflector();
  std::vector<DeinflectionResult> deinflect(const std::string& text);

 private:
  struct Rule {
    std::string from;
    std::string to;
    uint32_t conditions_in;
    uint32_t conditions_out;
    int group_id;
  };

  enum Conditions : std::uint32_t {
    NONE = 0,
    V = 1 << 0,
    V1 = 1 << 1,
    V1D = 1 << 2,
    V1P = 1 << 3,
    V5 = 1 << 4,
    V5D = 1 << 5,
    V5S = 1 << 6,
    V5SS = 1 << 7,
    V5SP = 1 << 8,
    VK = 1 << 9,
    VS = 1 << 10,
    VZ = 1 << 11,
    ADJ_I = 1 << 12,
    MASU = 1 << 13,
    MASEN = 1 << 14,
    TE = 1 << 15,
    BA = 1 << 16,
    KU = 1 << 17,
    TA = 1 << 18,
    NN = 1 << 19,
    NASAI = 1 << 20,
    YA = 1 << 21
  };

  void deinflect_recursive(const std::string& text, uint32_t conditions,
                                                      std::vector<TransformGroup>& trace,
                                                      std::vector<DeinflectionResult>& results);

  void init_transforms();

  int add_group(const TransformGroup& group);
  void add_rule(const Rule& rule);

  std::unordered_map<std::string, std::vector<Rule>> transforms_;
  std::vector<TransformGroup> groups_;
  size_t max_length_; 
};