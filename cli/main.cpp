#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

#include "../src/utf8.hpp"
#include "yomitandicts/deinflector.hpp"
#include "yomitandicts/importer.hpp"
#include "yomitandicts/lookup.hpp"
#include "yomitandicts/query.hpp"

void print_usage(const char* program) {
  std::cout << "Usage:\n";
  std::cout << program << " import <path/to/dictionary.zip>\n";
  std::cout << program << " deinflect <word>\n";
  std::cout << program << " query <path/to/database.db> <word>\n";
  std::cout << program << " lookup <path/to/database.db> <lookup_string>\n";
}

void cmd_import(const std::string& path) {
  std::filesystem::path zip_path(path);
  std::string output_dir = zip_path.parent_path().string();
  if (output_dir.empty()) {
    output_dir = ".";
  }
  ImportResult result = dictionary_importer::import(path, output_dir);

  if (result.success) {
    std::cout << "title: " << result.title << "\n";
    std::cout << "term_count: " << result.term_count << "\n";
    std::cout << "meta_count: " << result.meta_count << "\n";
    std::cout << "tag_count: " << result.tag_count << "\n";
  } else {
    std::cerr << "could not import dictionary:\n";
    for (const auto& error : result.errors) {
      std::cerr << "  " << error << "\n";
    }
  }
}

void cmd_deinflect(const std::string& inflected) {
  Deinflector deinflector;
  auto results = deinflector.deinflect(inflected);

  std::cout << "deinflections for: " << inflected << " length: " << utf8::length(inflected) << "\n";
  std::cout << "found " << results.size() << " candidates\n\n";

  for (const auto& r : results) {
    std::cout << r.text << " (conditions: " << r.conditions << ")\n";
    if (!r.trace.empty()) {
      std::cout << "  ";
      for (size_t i = 0; i < r.trace.size(); i++) {
        std::cout << r.trace[i].name;
        if (i < r.trace.size() - 1) {
          std::cout << " -> ";
        }
      }
      std::cout << "\n";
    }
  }
}

void cmd_query(const std::string& db_path, const std::string& expression) {
  DictionaryQuery dict_query();
  dict_query.add_dict(db_path);
  auto result = dict_query.query(expression);

  std::cout << "query results for: " << expression << " length: " << utf8::length(expression) << "\n";
  std::cout << result.size() << " entries\n";
  for (const auto& r : result) {
    std::cout << "---------------------------------------------------------------\n";
    std::cout << r.expression << " " << r.reading << " " << r.definition_tags << "\n";
    std::cout << r.glossaries.size() << " glossary entries\n";
    for (const auto& g : r.glossaries) {
      std::cout << "------\n";
      std::cout << g.dict_name << "\n";
      std::cout << g.glossary << "\n";
    }
  }
}

void cmd_lookup(const std::vector<std::string>& db_paths, const std::string& lookup_string, int max_results = 8,
                int scan_length = 16) {
  DictionaryQuery dict_query;
  for (const auto& path : db_paths) {
    dict_query.add_dict(path);
  }
  Deinflector deinflect;
  Lookup lookup(dict_query, deinflect);
  auto result = lookup.lookup(lookup_string, max_results, scan_length);

  std::cout << "lookup results for: " << lookup_string << " max_results: " << max_results
            << " scan_length: " << scan_length << "\n";
  std::cout << result.size() << " results\n";

  for (const auto& r : result) {
    std::cout << "---------------------------------------------------------------\n";
    std::cout << r.matched << "\n";
    if (!r.trace.empty()) {
      std::cout << "  ";
      for (size_t i = 0; i < r.trace.size(); i++) {
        std::cout << r.trace[i].name;
        if (i < r.trace.size() - 1) {
          std::cout << " -> ";
        }
      }
      std::cout << "\n";
    }
    std::cout << r.term.expression << " " << r.term.reading << "\n";
    for (const auto& g : r.term.glossaries) {
      std::cout << "------\n";
      std::cout << g.dict_name << "\n";
      std::cout << g.glossary << "\n";
    }
  }

  std::cout << "styles: \n";
  for (const auto& s : dict_query.get_styles()) {
    std::cout << s.dict_name << "\n";
    std::cout << s.styles << "\n";
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
  } else if (command == "query" && argc >= 4) {
    cmd_query(argv[2], argv[3]);
  } else if (command == "lookup" && argc >= 4) {
    std::vector<std::string> db_paths;
    std::string term = argv[argc - 1];
    for (int i = 2; i < argc - 1; ++i) {
      db_paths.emplace_back(argv[i]);
    }
    cmd_lookup(db_paths, term);
  } else {
    std::cerr << "Invalid command or insufficient arguments.\n";
    print_usage(argv[0]);
    return 1;
  }

  const auto end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> duration = end - begin;
  std::cout << "runtime: " << duration.count() << "ms\n";

  return 0;
}