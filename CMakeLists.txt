cmake_minimum_required(VERSION 3.25)

project(yomitan-dict-import LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(SQLite3 REQUIRED)
find_package(zstd REQUIRED)

add_subdirectory(external/zip)
add_subdirectory(external/glaze)

target_compile_definitions(zip PRIVATE MINIZ_DISABLE_ZIP_READER_CRC32_CHECKS)

add_library(yomitandicts
    src/importer.cpp
    src/json/yomitan_parser.cpp
    src/text_processor/text_processor.cpp
    src/deinflector.cpp
    src/query.cpp
    src/lookup.cpp
)

target_include_directories(yomitandicts PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/utfcpp/source
)

target_link_libraries(yomitandicts PRIVATE
    zip
    glaze::glaze
    SQLite::SQLite3
    zstd::libzstd_static
)

add_executable(yomitandicts-cpp
    cli/main.cpp
)

target_link_libraries(yomitandicts-cpp PRIVATE
    glaze::glaze
    yomitandicts
)

add_executable(benchmark-import
    tests/import.cpp
)

target_link_libraries(benchmark-import PRIVATE
    yomitandicts
)