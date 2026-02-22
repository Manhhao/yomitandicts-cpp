#pragma once
#include <memory>
#include <string>
#include <vector>

namespace hash {
class mphf {
 public:
  mphf();
  ~mphf();
  uint64_t operator()(std::string_view key) const;

  void build(const std::vector<std::string_view>& keys);
  void save(const std::string& path);
  void load(const std::string& path);

 private:
  struct phf;
  std::unique_ptr<phf> ptr_;
};
}