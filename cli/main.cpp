#include <iostream>
#include <string>
#include <chrono>

#include "deinflector/deinflector.hpp"
#include "import/importer.hpp"
#include "query/query.hpp"

void print_usage(const char* program) {
  std::cout << "Usage:\n";
  std::cout << program << " import <path/to/dictionary.zip>\n";
  std::cout << program << " deinflect <word>\n";
  std::cout << program << " query <path/to/database.db> <word>\n";
}

void cmd_import(const std::string& path) {
  ImportResult result = dictionary_importer::import(path);
  
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

void cmd_deinflect(const std::string& word) {
  Deinflector deinflector;
  auto results = deinflector.deinflect(word);
  
  std::cout << "deinflections for: " << word << "\n";
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
  DictionaryQuery dict_query;
  dict_query.add_dict(db_path);
  auto result = dict_query.query(expression);

  std::cout << "query results for: " << expression << "\n";
  std::cout << result.size() << " entries\n";
  for (const auto& r : result) {
    std::cout << "---------------------------------------------------------------\n";
    std::cout << r.expression << " " << r.reading << " " << r.definition_tags <<"\n";
    std::cout << r.glossaries.size() << " glossary entries\n";
    for (const auto& g : r.glossaries) {
      std::cout << "------\n";
      std::cout << g.dict_name << "\n";
      std::cout << g.glossary << "\n";
    }
  }
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    print_usage(argv[0]);
    return 1;
  }

  std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

  std::string command = argv[1];
  if (command == "import") {
    cmd_import(argv[2]);
  } else if (command == "deinflect") {
    cmd_deinflect(argv[2]);
  } else if (command == "query") {
    if (argc < 4) {
      print_usage(argv[0]);
      return 1;
    }
    cmd_query(argv[2], argv[3]);
  } else {
    std::cerr << "unknown command: " << command << "\n";
    print_usage(argv[0]);
    return 1;
  }

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::chrono::duration<double, std::milli> duration = end - begin;
  std::cout << "runtime: " << duration.count() << "ms\n";
  return 0;
}