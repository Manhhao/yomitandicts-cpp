#pragma once

#include <sqlite3.h>

#include <string>
#include <vector>

struct SQLiteStmt {
  sqlite3_stmt* stmt = nullptr;

  ~SQLiteStmt() {
    if (stmt) {
      sqlite3_finalize(stmt);
    }
  }

  operator sqlite3_stmt*() const { return stmt; }
  sqlite3_stmt** operator&() { return &stmt; }
};

struct ImportResult {
  bool success = false;
  std::string title;
  size_t term_count = 0;
  size_t meta_count = 0;
  size_t tag_count = 0;
  std::vector<std::string> errors;
};

namespace dictionary_importer {
ImportResult import(const std::string& zip_path);
};
