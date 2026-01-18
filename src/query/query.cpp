#include "query.hpp"
#include <vector>

void DictionaryQuery::add_dict(const std::string& db_path) {
    sqlite3* db;
    if (sqlite3_open(db_path.c_str(), &db) == SQLITE_OK) {
        dicts_.emplace_back(db);
    }
}

void DictionaryQuery::add_freq_dict(const std::string& db_path) {
    sqlite3* db;
    if (sqlite3_open(db_path.c_str(), &db) == SQLITE_OK) {
        freq_dicts_.emplace_back(db);
    }
}

std::vector<TermResult> query(const std::string& expression) {
    std::vector<TermResult> result{};

    return result;
}