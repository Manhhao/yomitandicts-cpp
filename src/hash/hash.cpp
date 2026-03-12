#include "hash.hpp"

#include <pthash.hpp>
#include <variant>

namespace hash {
struct xxhash64_sv {
  using hash_type = pthash::hash64;
  static pthash::hash64 hash(std::string_view s, uint64_t seed) {
    return pthash::hash64{XXH64(s.data(), s.size(), seed)};
  }
};

using dense_phf = pthash::dense_partitioned_phf<xxhash64_sv, pthash::skew_bucketer, pthash::C_int, true>;
using single_phf = pthash::single_phf<xxhash64_sv, pthash::skew_bucketer, pthash::compact, true>;
struct mphf::phf {
  phf_type type = phf_type::dense;
  std::variant<dense_phf, single_phf> phf;
};

mphf::mphf() : ptr_(std::make_unique<phf>()) {};
mphf::~mphf() = default;
uint64_t mphf::operator()(std::string_view key) const {
  return std::visit([&](auto const& phf) { return phf(key); }, ptr_->phf);
}

void mphf::build(const std::vector<std::string_view>& keys) {
  pthash::build_configuration config;
  config.verbose = false;
  config.num_threads = std::max<size_t>(1, std::thread::hardware_concurrency());
  if (keys.size() >= 4096) {
    ptr_->type = phf_type::dense;
    auto& phf = ptr_->phf.emplace<dense_phf>();
    phf.build_in_internal_memory(keys.begin(), keys.size(), config);
  } else {
    ptr_->type = phf_type::single;
    auto& phf = ptr_->phf.emplace<single_phf>();
    phf.build_in_internal_memory(keys.begin(), keys.size(), config);
  }
}

void mphf::save(const std::string& path) {
  std::visit([&](auto const& phf) { essentials::save(phf, path.c_str()); }, ptr_->phf);
}

void mphf::load(const std::string& path, phf_type type) {
  ptr_->type = type;
  if (type == phf_type::dense) {
    auto& phf = ptr_->phf.emplace<dense_phf>();
    essentials::load(phf, path.c_str());
  } else {
    auto& phf = ptr_->phf.emplace<single_phf>();
    essentials::load(phf, path.c_str());
  }
}

phf_type mphf::type() const { return ptr_->type; }
}