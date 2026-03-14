#include "hash.hpp"

#include <cstdint>
#include <fstream>
#include <memory>
#include <stdexcept>

#ifdef _WIN32

#include <xxh3.h>

namespace hash {
struct slot {
  uint64_t hash;
  uint64_t index;
};

static uint64_t xxh3_hash(std::string_view key) {
  uint64_t h = XXH3_64bits(key.data(), key.size());
  return h == 0 ? 1 : h;
}

struct mphf::phf {
  phf_type type = phf_type::xxh3_linear;
  uint64_t capacity = 0;
  std::vector<slot> table;
};

mphf::mphf() : ptr_(std::make_unique<phf>()) {};
mphf::~mphf() = default;

uint64_t mphf::operator()(std::string_view key) const {
  uint64_t h = xxh3_hash(key);
  uint64_t pos = h % ptr_->capacity;
  while (true) {
    if (ptr_->table[pos].hash == 0) {
      return 0;
    }
    if (ptr_->table[pos].hash == h) {
      return ptr_->table[pos].index;
    }
    pos = (pos + 1) % ptr_->capacity;
  }
}

void mphf::build(const std::vector<std::string_view>& keys) {
  ptr_->capacity = std::max<uint64_t>(keys.size() * 10 / 7, 16);
  ptr_->table.resize(ptr_->capacity);

  for (uint64_t i = 0; i < keys.size(); i++) {
    uint64_t h = xxh3_hash(keys[i]);
    uint64_t pos = h % ptr_->capacity;
    while (true) {
      if (ptr_->table[pos].hash == 0) {
        ptr_->table[pos] = {.hash = h, .index = i};
        break;
      }
      pos = (pos + 1) % ptr_->capacity;
    }
  }
}

void mphf::save(const std::string& path) {
  std::ofstream out(path, std::ios::binary);
  if (!out) {
    throw std::runtime_error("failed to save hash");
  }
  out.write(reinterpret_cast<const char*>(&ptr_->capacity), sizeof(uint64_t));
  out.write(reinterpret_cast<const char*>(ptr_->table.data()),
            static_cast<std::streamsize>(ptr_->capacity * sizeof(slot)));
}

void mphf::load(const std::string& path, phf_type type) {
  ptr_->type = phf_type::xxh3_linear;
  std::ifstream in(path, std::ios::binary);
  if (!in) {
    throw std::runtime_error("failed to open hash");
  }
  in.read(reinterpret_cast<char*>(&ptr_->capacity), sizeof(uint64_t));
  ptr_->table.resize(ptr_->capacity);
  in.read(reinterpret_cast<char*>(ptr_->table.data()), static_cast<std::streamsize>(ptr_->capacity * sizeof(slot)));
}

phf_type mphf::type() const { return ptr_->type; }
}

#else
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

#endif