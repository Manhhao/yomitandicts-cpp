cmake_minimum_required(VERSION 3.25)

project(yomitan-dict-import LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) 

find_package(SQLite3 REQUIRED)
find_package(zstd REQUIRED)
find_package(libzip REQUIRED)

add_executable(yomitandicts-cpp
    src/import/importer.cpp
    src/parser/parser.cpp
    src/deinflector/deinflector.cpp
    cli/main.cpp
)

target_include_directories(yomitandicts-cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(yomitandicts-cpp PRIVATE
    libzip::zip
    SQLite::SQLite3
    zstd::libzstd_static
)