#include <algorithm>
#include <chrono>
#include <numeric>
#include <print>
#include <vector>

#include "yomitandicts/deinflector.hpp"
#include "yomitandicts/lookup.hpp"
#include "yomitandicts/query.hpp"

int main(int argc, char** argv) {
  if (argc < 4) {
    std::println(stderr, "{} <dict_path> <word> <iterations>", argv[0]);
    return 1;
  }

  const std::string dict_path = argv[1];
  const std::string word = argv[2];
  const int iterations = std::stoi(argv[3]);

  DictionaryQuery query;
  query.add_dict(dict_path);
  Deinflector deinflector;
  Lookup lookup(query, deinflector);

  std::vector<double> durations;
  for (int i = 0; i < iterations; ++i) {
    const auto start = std::chrono::high_resolution_clock::now();
    const auto results = lookup.lookup(word);
    const auto end = std::chrono::high_resolution_clock::now();

    const std::chrono::duration<double, std::milli> elapsed = end - start;
    durations.push_back(elapsed.count());
  }

  if (durations.empty()) {
    return 1;
  }

  const auto [min, max] = std::ranges::minmax_element(durations);
  const double total = std::accumulate(durations.begin(), durations.end(), 0.0);
  const double average = total / durations.size();

  std::println("word: {} iterations: {}", word, iterations);
  std::println("total: {:.2f}ms", total);
  std::println("avg: {:.2f}ms", average);
  std::println("min: {:.2f}ms", *min);
  std::println("max: {:.2f}ms", *max);

  return 0;
}
