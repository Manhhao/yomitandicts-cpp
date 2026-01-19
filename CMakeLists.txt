cmake_minimum_required(VERSION 3.25)

project(yomitan-dict-import LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(SQLite3 REQUIRED)
find_package(zstd REQUIRED)
find_package(libzip REQUIRED)

add_library(yomitandicts
    src/importer.cpp
    src/json/json_parser.cpp
    src/deinflector.cpp
    src/query.cpp
    src/lookup.cpp
)

target_include_directories(yomitandicts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(yomitandicts PRIVATE
    libzip::zip
    SQLite::SQLite3
    zstd::libzstd_static
)

add_executable(yomitandicts-cpp
    cli/main.cpp
)

target_link_libraries(yomitandicts-cpp PRIVATE
    yomitandicts
)