#include "yomitandicts/query.hpp"

#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <zstd.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <ranges>
#include <string_view>

#include "hash/hash.hpp"
#include "json/yomitan_parser.hpp"

namespace {
uint8_t read_u8(const uint8_t*& addr) { return *addr++; }

uint16_t read_u16(const uint8_t*& addr) {
  uint16_t result;
  std::memcpy(&result, addr, sizeof(uint16_t));
  addr += sizeof(uint16_t);
  return result;
}

uint32_t read_u32(const uint8_t*& addr) {
  uint32_t result;
  std::memcpy(&result, addr, sizeof(uint32_t));
  addr += sizeof(uint32_t);
  return result;
}

uint64_t read_u64(const uint8_t*& addr) {
  uint64_t result;
  std::memcpy(&result, addr, sizeof(uint64_t));
  addr += sizeof(uint64_t);
  return result;
}

std::string_view read_str(const uint8_t*& addr, uint32_t len) {
  std::string_view result(reinterpret_cast<const char*>(addr), len);
  addr += len;
  return result;
}
}

struct DictionaryQuery::DictionaryData {
  hash::mphf phf;
  uint8_t* blobs = nullptr;
  size_t blobs_size = 0;
  uint64_t* offsets = nullptr;
  size_t offsets_size = 0;

  ~DictionaryData() {
    if (blobs) {
      munmap(blobs, blobs_size);
    }
    if (offsets) {
      munmap(offsets, offsets_size);
    }
  }
};

DictionaryQuery::DictionaryQuery() = default;
DictionaryQuery::~DictionaryQuery() = default;

DictionaryQuery::DictionaryQuery(DictionaryQuery&&) noexcept = default;
DictionaryQuery& DictionaryQuery::operator=(DictionaryQuery&&) noexcept = default;

void DictionaryQuery::add_dict(const std::string& path, DictionaryType type) {
  Dictionary dict;
  Index info;
  std::string buf{};
  if (glz::read_file_json(info, path + "/info.json", buf)) {
    return;
  }

  dict.name = info.title.empty() ? std::filesystem::path(path).stem().string() : info.title;
  if (std::filesystem::exists(path + "/styles.css")) {
    std::ifstream f(path + "/styles.css");
    dict.styles = std::string(std::istreambuf_iterator<char>(f), {});
  }

  dict.data = std::make_unique<DictionaryData>();
  dict.data->phf.load(path + "/hash.mph");

  struct stat st{};
  int fd = open((path + "/offsets.bin").c_str(), O_RDONLY);
  if (fd == -1) {
    return;
  }

  if (fstat(fd, &st) != 0) {
    close(fd);
    return;
  }

  dict.data->offsets_size = st.st_size;
  dict.data->offsets = reinterpret_cast<uint64_t*>(mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
  if (dict.data->offsets == MAP_FAILED) {
    close(fd);
    return;
  }
  close(fd);

  fd = open((path + "/blobs.bin").c_str(), O_RDONLY);
  if (fd < 0) {
    return;
  }

  if (fstat(fd, &st) != 0) {
    close(fd);
    return;
  }

  dict.data->blobs_size = st.st_size;
  dict.data->blobs = reinterpret_cast<uint8_t*>(mmap(nullptr, st.st_size, PROT_READ, MAP_SHARED, fd, 0));
  if (dict.data->blobs == MAP_FAILED) {
    close(fd);
    return;
  }
  close(fd);

  switch (type) {
    case TERM:
      dicts_.push_back(std::move(dict));
      break;
    case FREQ:
      freq_dicts_.push_back(std::move(dict));
      break;
    case PITCH:
      pitch_dicts_.push_back(std::move(dict));
      break;
  }
}

void DictionaryQuery::add_term_dict(const std::string& path) { add_dict(path, DictionaryQuery::DictionaryType::TERM); }

void DictionaryQuery::add_freq_dict(const std::string& path) { add_dict(path, DictionaryQuery::DictionaryType::FREQ); }

void DictionaryQuery::add_pitch_dict(const std::string& path) {
  add_dict(path, DictionaryQuery::DictionaryType::PITCH);
}

std::vector<TermResult> DictionaryQuery::query(const std::string& expression) const {
  std::map<std::pair<std::string_view, std::string_view>, TermResult> term_map;
  for (const auto& [name, styles, data] : dicts_) {
    uint64_t hash = data->phf(expression);
    uint64_t offset_addr = data->offsets[hash];
    const uint8_t* index_addr = data->blobs + offset_addr;

    uint32_t count = read_u32(index_addr);
    for (uint32_t i = 0; i < count; i++) {
      uint64_t offset = read_u64(index_addr);
      const uint8_t* blob_addr = data->blobs + offset;

      // first byte encodes term (0) or meta (1) entry
      uint8_t type = read_u8(blob_addr);
      if (type != 0) {
        continue;
      }

      uint16_t expr_len = read_u16(blob_addr);
      std::string_view expr = read_str(blob_addr, expr_len);

      uint16_t reading_len = read_u16(blob_addr);
      std::string_view reading = read_str(blob_addr, reading_len);

      if (expr != expression && reading != expression) {
        continue;
      }

      uint32_t glossary_size = read_u32(blob_addr);
      std::string glossary = decompress_glossary(read_str(blob_addr, glossary_size).data(), glossary_size);

      uint8_t def_tags_size = read_u8(blob_addr);
      std::string_view definition_tags = read_str(blob_addr, def_tags_size);

      uint8_t rules_size = read_u8(blob_addr);
      std::string_view rules = read_str(blob_addr, rules_size);

      uint8_t term_tag_size = read_u8(blob_addr);
      std::string_view term_tags = read_str(blob_addr, term_tag_size);

      GlossaryEntry entry;
      entry.dict_name = name;
      entry.definition_tags = definition_tags;
      entry.term_tags = term_tags;
      entry.glossary = glossary;

      auto [it, inserted] = term_map.try_emplace({expr, reading});
      if (inserted) {
        it->second = {.expression = std::string(expr),
                      .reading = std::string(reading),
                      .rules = std::string(rules),
                      .glossaries = {},
                      .frequencies = {}};
      } else {
        if (!rules.empty()) {
          if (!it->second.rules.empty()) {
            it->second.rules += " ";
          }
          it->second.rules += rules;
        }
      }
      it->second.glossaries.push_back(std::move(entry));
    }
  }

  auto results = term_map | std::views::values | std::views::as_rvalue | std::ranges::to<std::vector>();
  query_freq(results);
  query_pitch(results);

  return results;
}

void DictionaryQuery::query_freq(std::vector<TermResult>& terms) const {
  for (auto& term : terms) {
    for (const auto& [name, styles, data] : freq_dicts_) {
      uint64_t hash = data->phf(term.expression);
      uint64_t offset_addr = data->offsets[hash];

      const uint8_t* index_addr = data->blobs + offset_addr;
      uint32_t count = read_u32(index_addr);

      std::vector<Frequency> frequencies;
      for (uint32_t i = 0; i < count; i++) {
        uint64_t offset = read_u64(index_addr);
        const uint8_t* blob_addr = data->blobs + offset;

        uint8_t type = read_u8(blob_addr);
        if (type != 1) {
          continue;
        }

        uint16_t expr_len = read_u16(blob_addr);
        std::string_view expr = read_str(blob_addr, expr_len);
        if (expr != term.expression) {
          continue;
        }

        uint8_t mode_len = read_u8(blob_addr);
        std::string_view mode = read_str(blob_addr, mode_len);
        if (mode != "freq") {
          continue;
        }

        uint32_t freq_data_size = read_u32(blob_addr);
        std::string_view freq_data = read_str(blob_addr, freq_data_size);

        ParsedFrequency parsed;
        if (yomitan_parser::parse_frequency(freq_data, parsed)) {
          if (!parsed.reading.empty() && parsed.reading != term.reading) {
            continue;
          }
          frequencies.emplace_back(
              Frequency{.value = parsed.value, .display_value = std::string(parsed.display_value)});
        }
      }
      if (!frequencies.empty()) {
        term.frequencies.emplace_back(FrequencyEntry{.dict_name = name, .frequencies = std::move(frequencies)});
      }
    }
  }
}

void DictionaryQuery::query_pitch(std::vector<TermResult>& terms) const {
  for (auto& term : terms) {
    for (const auto& [name, styles, data] : pitch_dicts_) {
      uint64_t hash = data->phf(term.expression);
      uint64_t offset_addr = data->offsets[hash];

      const uint8_t* index_addr = data->blobs + offset_addr;
      uint32_t count = read_u32(index_addr);

      std::vector<int> pitch_positions;
      for (uint32_t i = 0; i < count; i++) {
        uint64_t offset = read_u64(index_addr);
        const uint8_t* blob_addr = data->blobs + offset;

        uint8_t type = read_u8(blob_addr);
        if (type != 1) {
          continue;
        }

        uint16_t expr_len = read_u16(blob_addr);
        std::string_view expr = read_str(blob_addr, expr_len);
        if (expr != term.expression) {
          continue;
        }

        uint8_t mode_len = read_u8(blob_addr);
        std::string_view mode = read_str(blob_addr, mode_len);
        if (mode != "pitch") {
          continue;
        }

        uint32_t pitch_data_size = read_u32(blob_addr);
        std::string_view pitch_data = read_str(blob_addr, pitch_data_size);

        ParsedPitch parsed;
        if (yomitan_parser::parse_pitch(pitch_data, parsed)) {
          if (!parsed.reading.empty() && parsed.reading != term.reading) {
            continue;
          }
          pitch_positions.insert(pitch_positions.end(), parsed.pitches.begin(), parsed.pitches.end());
        }
      }
      if (!pitch_positions.empty()) {
        term.pitches.emplace_back(PitchEntry{.dict_name = name, .pitch_positions = std::move(pitch_positions)});
      }
    }
  }
}

std::string DictionaryQuery::decompress_glossary(const void* data, size_t size) {
  if (!data || size == 0) {
    return "";
  }

  unsigned long long decompressed_size = ZSTD_getFrameContentSize(data, size);
  if (decompressed_size == ZSTD_CONTENTSIZE_ERROR || decompressed_size == ZSTD_CONTENTSIZE_UNKNOWN) {
    return "";
  }

  std::string result;
  result.resize(decompressed_size);

  size_t actual_size = ZSTD_decompress(result.data(), result.size(), data, size);
  if (ZSTD_isError(actual_size)) {
    return "";
  }

  result.resize(actual_size);
  return result;
}

std::vector<DictionaryStyle> DictionaryQuery::get_styles() const {
  return dicts_ | std::views::filter([](const auto& d) { return !d.styles.empty(); }) |
         std::views::transform([](const auto& d) { return DictionaryStyle{d.name, d.styles}; }) |
         std::ranges::to<std::vector>();
}

std::vector<std::string> DictionaryQuery::get_freq_dict_order() const {
  return freq_dicts_ | std::views::transform([](const auto& d) { return d.name; }) | std::ranges::to<std::vector>();
}
