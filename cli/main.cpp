#include <iostream>

#include "../src/import/importer.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " <path/to/dictionary.zip>" << "\n";
  }

  ImportResult result = dictionary_importer::import(argv[1]);

  if (result.success) {
    std::cout << "title: " << result.title << "\n";
    std::cout << "term_count: " << result.term_count << "\n";
    std::cout << "meta_count: " << result.meta_count << "\n";
    std::cout << "tag_count: " << result.tag_count << "\n";
  } else {
    std::cout << "could not import dictionary: " << "\n";
    for (auto &error : result.errors) {
        std::cout << error << "\n";
    }
  }
}