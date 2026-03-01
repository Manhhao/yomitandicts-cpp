#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utf8.h>

#include <chrono>
#include <filesystem>
#include <print>
#include <ranges>
#include <string>

#include "../src/text_processor/text_processor.hpp"
#include "yomitandicts/deinflector.hpp"
#include "yomitandicts/importer.hpp"
#include "yomitandicts/lookup.hpp"
#include "yomitandicts/query.hpp"

void print_usage(const char* program) {
  std::println("Usage:");
  std::println("{} import <path/to/dictionary.zip>", program);
  std::println("{} deinflect <word>", program);
  std::println("{} preprocess <word>", program);
  std::println("{} query <path/to/dictionary> <word>", program);
  std::println("{} lookup <path/to/dictionary> <lookup_string>", program);
  std::println("{} freq <path/to/dictionary> <word>", program);
}

void cmd_import(const std::string& path) {
  std::filesystem::path zip_path(path);
  std::string output_dir = zip_path.parent_path().string();
  if (output_dir.empty()) {
    output_dir = ".";
  }
  ImportResult result = dictionary_importer::import(path, output_dir);

  if (result.success) {
    std::println("title: {}", result.title);
    std::println("term_count: {}", result.term_count);
    std::println("meta_count: {}", result.meta_count);
    std::println("tag_count: {}", result.tag_count);
    std::println("media_count: {}", result.media_count);
  } else {
    std::println(stderr, "could not import dictionary:");
    for (const auto& error : result.errors) {
      std::println(stderr, " {}", error);
    }
  }
}

void cmd_deinflect(const std::string& inflected) {
  Deinflector deinflector;
  auto results = deinflector.deinflect(inflected);

  std::println("deinflections for: {} length: {}", inflected, utf8::distance(inflected.begin(), inflected.end()));
  std::println("found {} candidates\n", results.size());

  for (const auto& r : results) {
    std::println("{} (conditions: {})", r.text, r.conditions);
    if (!r.trace.empty()) {
      std::print("  ");
      for (size_t i = 0; i < r.trace.size(); ++i) {
        std::print("{}{}", r.trace[i].name, i < r.trace.size() - 1 ? " -> " : "");
      }
      std::println("");
    }
  }
}

void cmd_preprocess(const std::string& text) {
  auto results = text_processor::process(text);

  std::println("preproccesing for: {} length: {}", text, utf8::distance(text.begin(), text.end()));
  std::println("found {} variants", results.size());

  for (const auto& r : results) {
    std::println("{}", r.text);
  }
}

void cmd_query(const std::string& db_path, const std::string& expression) {
  DictionaryQuery dict_query;
  dict_query.add_term_dict(db_path);
  auto result = dict_query.query(expression);

  std::println("query results for: {} length: {}", expression, utf8::distance(expression.begin(), expression.end()));
  std::println("{} entries", result.size());
  for (const auto& r : result) {
    std::println("---------------------------------------------------------------");
    std::println("{} {} {}", r.expression, r.reading, r.rules);
    std::println("{} glossary entries", r.glossaries.size());
    for (const auto& g : r.glossaries) {
      std::println("------");
      std::println("{}", g.dict_name);
      std::println("{}", g.glossary);
    }
  }
}

void cmd_freq(const std::string& path, const std::string& expression, const std::string& reading) {
  std::vector<TermResult> terms;
  terms.emplace_back(TermResult{.expression = expression, .reading = reading});

  DictionaryQuery query;
  query.add_freq_dict(path);
  query.query_freq(terms);
  std::println("frequency entries for: {}", expression);
  int count = 0;
  for (auto& freq : terms[0].frequencies) {
    std::println("dict: {}", freq.dict_name);
    for (auto& freq_entry : freq.frequencies) {
      std::println("val: {} display_val: {}", freq_entry.value, freq_entry.display_value);
      count++;
    }
  }
  std::println("count: {}", count);
}

void cmd_lookup(const std::vector<std::string>& db_paths, const std::string& lookup_string, int max_results = 8,
                int scan_length = 16) {
  DictionaryQuery dict_query;
  for (const auto& path : db_paths) {
    dict_query.add_term_dict(path);
  }
  Deinflector deinflect;
  Lookup lookup(dict_query, deinflect);
  auto result = lookup.lookup(lookup_string, max_results, scan_length);

  std::println("lookup results for: {} max_results: {} scan_length: {}", lookup_string, max_results, scan_length);
  std::println("{} results", result.size());

  for (const auto& r : result) {
    std::println("---------------------------------------------------------------");
    std::println("{}", r.matched);
    if (!r.trace.empty()) {
      std::print("  ");
      for (size_t i = 0; i < r.trace.size(); ++i) {
        std::print("{}{}", r.trace[i].name, i < r.trace.size() - 1 ? " -> " : "");
      }
      std::println("");
    }
    std::println("{} {}", r.term.expression, r.term.reading);
    for (const auto& g : r.term.glossaries) {
      std::println("------");
      std::println("{}", g.dict_name);
      std::println("{}", g.glossary);
    }
  }

  std::println("styles: ");
  for (const auto& s : dict_query.get_styles()) {
    std::println("{}", s.dict_name);
    std::println("{}", s.styles);
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    print_usage(argv[0]);
    return 1;
  }

  const auto begin = std::chrono::steady_clock::now();
  std::string_view command = argv[1];

  if (command == "import" && argc >= 3) {
    cmd_import(argv[2]);
  } else if (command == "deinflect" && argc >= 3) {
    cmd_deinflect(argv[2]);
  } else if (command == "preprocess" && argc >= 3) {
    cmd_preprocess(argv[2]);
  } else if (command == "query" && argc >= 4) {
    cmd_query(argv[2], argv[3]);
  } else if (command == "lookup" && argc >= 4) {
    auto db_paths = std::views::counted(argv + 2, argc - 3) |
                    std::views::transform([](const char* arg) { return std::string(arg); }) |
                    std::ranges::to<std::vector>();
    std::string term = argv[argc - 1];
    cmd_lookup(db_paths, term);
  } else if (command == "freq" && argc >= 5) {
    cmd_freq(argv[2], argv[3], argv[4]);
  } else {
    print_usage(argv[0]);
    return 1;
  }

  const auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> duration = end - begin;
  std::println("runtime: {}ms", duration.count());

  return 0;
}