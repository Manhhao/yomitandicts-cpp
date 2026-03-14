#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace hash {
enum phf_type : std::uint8_t {
  dense,
  single,
  xxh3_linear
};
class mphf {
 public:
  mphf();
  ~mphf();
  uint64_t operator()(std::string_view key) const;

  void build(const std::vector<std::string_view>& keys);
  void save(const std::string& path);
  void load(const std::string& path, phf_type type);
  phf_type type() const;
 private:
  struct phf;
  std::unique_ptr<phf> ptr_;
};
}