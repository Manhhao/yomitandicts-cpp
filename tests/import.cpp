#include <algorithm>
#include <chrono>
#include <filesystem>
#include <numeric>
#include <print>
#include <vector>

#include "yomitandicts/importer.hpp"

int main(int argc, char** argv) {
  if (argc < 3) {
    std::println(stderr, "{} <zip_path> <iterations>", argv[0]);
    return 1;
  }

  const std::string zip_path = argv[1];
  const int iterations = std::stoi(argv[2]);
  std::vector<double> durations;
  std::string dict_title;
  size_t term_count = 0;
  for (int i = 0; i < iterations; ++i) {
    const auto start = std::chrono::high_resolution_clock::now();
    const auto result = dictionary_importer::import(zip_path, ".");
    const auto end = std::chrono::high_resolution_clock::now();

    if (result.success) {
      if (dict_title.empty()) {
        dict_title = result.title;
      }
      if (term_count == 0) {
        term_count = result.term_count;
      }
      const std::chrono::duration<double, std::milli> elapsed = end - start;
      durations.push_back(elapsed.count());
      std::filesystem::remove(result.title + ".db");
    }
  }

  if (durations.empty()) {
    return 1;
  }

  const auto [min, max] = std::ranges::minmax_element(durations);
  const double total = std::accumulate(durations.begin(), durations.end(), 0.0);
  const double average = total / durations.size();

  std::println("dict: {} iterations: {}", dict_title, iterations);
  std::println("term_count: {}", term_count);
  std::println("total: {:.2f}ms", total);
  std::println("avg: {:.2f}ms", average);
  std::println("min: {:.2f}ms", *min);
  std::println("max: {:.2f}ms", *max);

  return 0;
}
