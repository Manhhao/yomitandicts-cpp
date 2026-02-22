#include "hash.hpp"

#include <pthash.hpp>

namespace hash {
struct xxhash64_sv {
  using hash_type = pthash::hash64;
  static pthash::hash64 hash(std::string_view s, uint64_t seed) {
    return pthash::hash64{XXH64(s.data(), s.size(), seed)};
  }
};
struct mphf::phf {
  pthash::single_phf<xxhash64_sv, pthash::skew_bucketer, pthash::compact, true> phf;
};

mphf::mphf() : ptr_(std::make_unique<phf>()) {};
mphf::~mphf() = default;
uint64_t mphf::operator()(std::string_view key) const { return ptr_->phf(key); }

void mphf::build(const std::vector<std::string_view>& keys) {
  pthash::build_configuration config;
  config.verbose = false;
  config.num_threads = std::max<size_t>(1, std::thread::hardware_concurrency());
  ptr_->phf.build_in_internal_memory(keys.begin(), keys.size(), config);
}

void mphf::save(const std::string& path) { essentials::save(ptr_->phf, path.c_str()); }

void mphf::load(const std::string& path) { essentials::load(ptr_->phf, path.c_str()); }
}