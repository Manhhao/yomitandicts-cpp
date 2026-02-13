# yomitandicts-cpp

This library implements a dictionary backend that works similarly to [Yomitan](https://github.com/yomidevs/yomitan). This was made for [Hoshi Reader](https://github.com/Manhhao/Hoshi-Reader) and was only tested with Japanese. Other languages might need their own deinflector or adjustments to the lookup strategy.

## Reference

### importer
```cpp
ImportResult dictionary_importer::import(const std::string& zip_path, const std::string& output_dir);
```
Imports a Yomitan `.zip` dictionary file into a SQLite database. The resulting database is stored in `output_dir/<dict_title>.db`. Glossaries are compressed using zstd. Term, frequency and pitch dictionaries were tested, though this only supports .

### query
```cpp
void DictionaryQuery::add_dict(const std::string& db_path);
```
Adds a term dictionary `.db` file to the query.

```cpp
void DictionaryQuery::add_freq_dict(const std::string& db_path);
```
Adds a frequency dictionary `.db` file to the query.

```cpp
void DictionaryQuery::add_pitch_dict(const std::string& db_path);
```
Adds a pitch dictionary `.db` file to the query.

```cpp
std::vector<TermResult> DictionaryQuery::query(const std::string& expression) const;
```
Queries all added databases for the given expression. TermResult includes glossary, frequency and pitch data in the order dictionaries were added. Glossaries are decompressed.

```cpp
std::vector<DictionaryStyle> DictionaryQuery::get_styles() const;
```
Returns CSS styles for all dictionaries, if present.

### deinflector
```cpp
std::vector<DeinflectionResult> Deinflector::deinflect(const std::string& text) const;
```
Deinflects a given Japanese string using rules from the Yomitan deinflector. As this doesn't use any dictionary data, it does not filter invalid results.

```cpp
static uint32_t Deinflector::pos_to_conditions(const std::vector<std::string>& part_of_speech);
```
Converts a vector of part-of-speech tags into a bitmask used for deinflection filtering.

### text_processor
```cpp
std::vector<TextVariant> process(const std::string& src);
```
Applies Japanese text processors to `src` and returns all variants with the number of transformations applied.

### lookup
```cpp
Lookup::Lookup(DictionaryQuery& query, Deinflector& deinflector);
```
Creates a Lookup object using a given query with dictionaries added and a deinflector.

```cpp
std::vector<LookupResult> Lookup::lookup(const std::string& lookup_string, int max_results = 16, size_t scan_length = 16) const;
```
Follows a parsing strategy similar to Yomitan. Substrings of `lookup_string` are tested from length `scan_length` down to 1. Each substring is deinflected and queried using the query object. 

Results are filtered by part-of-speech tags defined in dictionaries, or added directly if none are present. The results are sorted by matched length first, then by preprocessing steps, and then by frequency.

## Acknowledgements

- [Yomitan](https://github.com/yomidevs/yomitan): Dictionary format, Japanese deinflection rules and descriptions, Japanese preprocessor GPLv3
- [SQLite](https://sqlite.org/): Unlicensed
- [zstd](https://github.com/facebook/zstd): BSD
- [kuba--/zip](https://github.com/kuba--/zip): MIT
- [utfcpp](https://github.com/nemtrif/utfcpp): BSL-1.0

## License
See [LICENSE](LICENSE) for details.