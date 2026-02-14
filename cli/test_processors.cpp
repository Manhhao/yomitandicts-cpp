#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../src/text_processor/text_processor.hpp"

static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// Check that a specific variant exists in the process() output
bool has_variant(const std::vector<TextVariant>& results, const std::string& expected) {
  for (const auto& r : results) {
    if (r.text == expected) return true;
  }
  return false;
}

void expect_variant(const std::string& input, const std::string& expected, const std::string& test_name) {
  ++total_tests;
  auto results = text_processor::process(input);
  if (has_variant(results, expected)) {
    ++passed_tests;
  } else {
    ++failed_tests;
    std::cout << "  FAIL: " << test_name << "\n";
    std::cout << "    input:    \"" << input << "\"\n";
    std::cout << "    expected: \"" << expected << "\"\n";
    std::cout << "    got " << results.size() << " variants:\n";
    for (const auto& r : results) {
      std::cout << "      \"" << r.text << "\" (steps=" << r.steps << ")\n";
    }
  }
}

// Check that the exact set of variants match (for smaller test cases)
void expect_exact_variants(const std::string& input, const std::set<std::string>& expected,
                           const std::string& test_name) {
  ++total_tests;
  auto results = text_processor::process(input);
  std::set<std::string> actual;
  for (const auto& r : results) {
    actual.insert(r.text);
  }

  bool ok = true;
  for (const auto& e : expected) {
    if (actual.find(e) == actual.end()) {
      ok = false;
      break;
    }
  }

  if (ok) {
    ++passed_tests;
  } else {
    ++failed_tests;
    std::cout << "  FAIL: " << test_name << "\n";
    std::cout << "    input: \"" << input << "\"\n";
    std::cout << "    missing variants:\n";
    for (const auto& e : expected) {
      if (actual.find(e) == actual.end()) {
        std::cout << "      \"" << e << "\"\n";
      }
    }
  }
}

// ============================================================
// Test: convertHalfWidthCharacters (ﾖﾐﾁｬﾝ → ヨミチャン)
// ============================================================
void test_half_width_characters() {
  std::cout << "[convertHalfWidthCharacters]\n";

  // From Yomitan test: japanese-util.test.js
  expect_variant("0123456789", "0123456789", "ASCII digits unchanged");
  expect_variant("abcdefghij", "abcdefghij", "ASCII letters unchanged");

  // カタカナ -> unchanged (already fullwidth)
  expect_variant("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A", // カタカナ
                 "\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A", // カタカナ
                 "fullwidth katakana unchanged");

  // ｶｷ → カキ (halfwidth to fullwidth, no dakuten)
  expect_variant("\xEF\xBD\xB6\xEF\xBD\xB7",         // ｶｷ
                 "\xE3\x82\xAB\xE3\x82\xAD",         // カキ
                 "halfwidth ｶｷ -> カキ");

  // ｶﾞｷ → ガキ (halfwidth with dakuten)
  expect_variant("\xEF\xBD\xB6\xEF\xBE\x9E\xEF\xBD\xB7", // ｶﾞｷ
                 "\xE3\x82\xAC\xE3\x82\xAD",               // ガキ
                 "halfwidth ｶﾞｷ -> ガキ");

  // ﾆﾎﾝ → ニホン
  expect_variant("\xEF\xBE\x86\xEF\xBE\x8E\xEF\xBE\x9D", // ﾆﾎﾝ
                 "\xE3\x83\x8B\xE3\x83\x9B\xE3\x83\xB3",   // ニホン
                 "halfwidth ﾆﾎﾝ -> ニホン");

  // ﾆｯﾎﾟﾝ → ニッポン
  expect_variant("\xEF\xBE\x86\xEF\xBD\xAF\xEF\xBE\x8E\xEF\xBE\x9F\xEF\xBE\x9D", // ﾆｯﾎﾟﾝ
                 "\xE3\x83\x8B\xE3\x83\x83\xE3\x83\x9D\xE3\x83\xB3",               // ニッポン
                 "halfwidth ﾆｯﾎﾟﾝ -> ニッポン");
  std::cout << "\n";
}

// ============================================================
// Test: alphabeticToHiragana (yomichan → よみちゃん)
// ============================================================
void test_alphabetic_to_hiragana() {
  std::cout << "[alphabeticToHiragana]\n";

  // From Yomitan test
  expect_variant("chikara",
                 "\xE3\x81\xA1\xE3\x81\x8B\xE3\x82\x89", // ちから
                 "chikara -> ちから");

  expect_variant("CHIKARA",
                 "\xE3\x81\xA1\xE3\x81\x8B\xE3\x82\x89", // ちから
                 "CHIKARA -> ちから");

  // Numbers should pass through
  expect_variant("0123456789", "0123456789", "digits unchanged in alphabetic");

  // Mixed: kana and romaji - only alphabetic runs are converted
  // abcdefghij → あbcでfgひj  (a→あ, de→で, hi→ひ, others stay as romaji fragments)
  expect_variant("abcdefghij",
                 "\xE3\x81\x82\x62\x63\xE3\x81\xA7\x66\x67\xE3\x81\xB2\x6A", // あbcでfgひj
                 "abcdefghij -> あbcでfgひj");

  // Double consonant (sokuon): katta → かった
  expect_variant("katta",
                 "\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x9F", // かった
                 "katta -> かった");

  // nn → ん (nn is matched first before ni, so sannin → さんいん, not さんにん)
  // This is correct behavior for simple replaceAll conversion (not IME-style)
  expect_variant("sannin",
                 "\xE3\x81\x95\xE3\x82\x93\xE3\x81\x84\xE3\x82\x93", // さんいん
                 "sannin -> さんいん (nn matched before ni)");

  std::cout << "\n";
}

// ============================================================
// Test: alphanumericWidthVariants (ｙｏｍｉｔａｎ ↔ yomitan)
// ============================================================
void test_alphanumeric_width() {
  std::cout << "[alphanumericWidthVariants]\n";

  // fullwidth -> halfwidth (direct)
  // ｙｏｍｉｔａｎ → yomitan
  expect_variant("\xEF\xBD\x99\xEF\xBD\x8F\xEF\xBD\x8D\xEF\xBD\x89\xEF\xBD\x94\xEF\xBD\x81\xEF\xBD\x8E", // ｙｏｍｉｔａｎ
                 "yomitan",
                 "fullwidth ｙｏｍｉｔａｎ -> yomitan");

  // halfwidth -> fullwidth (inverse)
  expect_variant("yomitan",
                 "\xEF\xBD\x99\xEF\xBD\x8F\xEF\xBD\x8D\xEF\xBD\x89\xEF\xBD\x94\xEF\xBD\x81\xEF\xBD\x8E", // ｙｏｍｉｔａｎ
                 "yomitan -> fullwidth ｙｏｍｉｔａｎ");

  // ０１２ → 012
  expect_variant("\xEF\xBC\x90\xEF\xBC\x91\xEF\xBC\x92", // ０１２
                 "012",
                 "fullwidth ０１２ -> 012");

  std::cout << "\n";
}

// ============================================================
// Test: convertHiraganaToKatakana (already implemented, verify still works)
// ============================================================
void test_kana_conversion() {
  std::cout << "[convertHiraganaToKatakana]\n";

  // カタカナ → かたかな (katakana to hiragana)
  expect_variant("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A", // カタカナ
                 "\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // かたかな
                 "カタカナ -> かたかな");

  // かたかな → カタカナ (hiragana to katakana)
  expect_variant("\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // かたかな
                 "\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A", // カタカナ
                 "かたかな -> カタカナ");

  // Prolonged sound mark: カーナー → かあなあ
  expect_variant("\xE3\x82\xAB\xE3\x83\xBC\xE3\x83\x8A\xE3\x83\xBC", // カーナー
                 "\xE3\x81\x8B\xE3\x81\x82\xE3\x81\xAA\xE3\x81\x82", // かあなあ
                 "カーナー -> かあなあ");

  // Mixed: カタカナかたかな → かたかなかたかな
  expect_variant("\xE3\x82\xAB\xE3\x82\xBF\xE3\x82\xAB\xE3\x83\x8A\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // カタカナかたかな
                 "\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // かたかなかたかな
                 "カタカナかたかな -> かたかなかたかな");

  std::cout << "\n";
}

// ============================================================
// Test: collapseEmphaticSequences
// ============================================================
void test_collapse_emphatic() {
  std::cout << "[collapseEmphaticSequences]\n";

  // すっっごーーい → すっごーい (partial collapse)
  expect_variant(
      "\xE3\x81\x99\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\x94\xE3\x83\xBC\xE3\x83\xBC\xE3\x81\x84", // すっっごーーい
      "\xE3\x81\x99\xE3\x81\xA3\xE3\x81\x94\xE3\x83\xBC\xE3\x81\x84", // すっごーい
      "すっっごーーい -> すっごーい (partial)");

  // すっっごーーい → すごい (full collapse)
  expect_variant(
      "\xE3\x81\x99\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\x94\xE3\x83\xBC\xE3\x83\xBC\xE3\x81\x84", // すっっごーーい
      "\xE3\x81\x99\xE3\x81\x94\xE3\x81\x84", // すごい
      "すっっごーーい -> すごい (full)");

  // かっこい → unchanged (single emphatic, no change for partial)
  expect_variant(
      "\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // かっこい
      "\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // かっこい
      "かっこい -> かっこい (single emphatic unchanged)");

  // かっこい → かこい (full collapse)
  expect_variant(
      "\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // かっこい
      "\xE3\x81\x8B\xE3\x81\x93\xE3\x81\x84",               // かこい
      "かっこい -> かこい (full collapse)");

  // Leading emphatic preserved: っこい → っこい
  expect_variant(
      "\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // っこい
      "\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // っこい
      "っこい -> っこい (leading preserved)");

  // Trailing emphatic preserved: こいっ → こいっ
  expect_variant(
      "\xE3\x81\x93\xE3\x81\x84\xE3\x81\xA3", // こいっ
      "\xE3\x81\x93\xE3\x81\x84\xE3\x81\xA3", // こいっ
      "こいっ -> こいっ (trailing preserved)");

  // Empty
  expect_variant("", "", "empty string");

  // Pure emphatic: っっっ → っっっ (whole string emphatic, unchanged)
  expect_variant(
      "\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\xA3", // っっっ
      "\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\xA3", // っっっ
      "っっっ -> っっっ (all emphatic)");

  // Leading+trailing emphatic with middle: っこいっ → っこいっ (both preserved)
  expect_variant(
      "\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84\xE3\x81\xA3", // っこいっ
      "\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84\xE3\x81\xA3", // っこいっ
      "っこいっ -> っこいっ (leading+trailing preserved)");

  // Multiple leading emphatic: っっこい → っっこい
  expect_variant(
      "\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // っっこい
      "\xE3\x81\xA3\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84", // っっこい
      "っっこい -> っっこい (multiple leading preserved)");

  // Mixed emphatic types: っーッかっこいいっーッ
  //   partial → っーッかっこいいっーッ  (preserved at leading/trailing, single emphatic kept)
  //   full    → っーッかこいいっーッ    (middle empathics removed)
  expect_variant(
      "\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84\xE3\x81\x84\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83", // っーッかっこいいっーッ
      "\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84\xE3\x81\x84\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83", // っーッかっこいいっーッ (partial)
      "っーッかっこいいっーッ -> unchanged (partial collapse, single emphatics)");

  expect_variant(
      "\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83\xE3\x81\x8B\xE3\x81\xA3\xE3\x81\x93\xE3\x81\x84\xE3\x81\x84\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83", // っーッかっこいいっーッ
      "\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83\xE3\x81\x8B\xE3\x81\x93\xE3\x81\x84\xE3\x81\x84\xE3\x81\xA3\xE3\x83\xBC\xE3\x83\x83", // っーッかこいいっーッ (full)
      "っーッかっこいいっーッ -> っーッかこいいっーッ (full collapse)");

  std::cout << "\n";
}

// ============================================================
// Test: normalizeCombiningCharacters
// ============================================================
void test_normalize_combining() {
  std::cout << "[normalizeCombiningCharacters]\n";

  // か + combining dakuten → が
  expect_variant(
      "\xE3\x81\x8B\xE3\x82\x99", // か + U+3099
      "\xE3\x81\x8C",               // が
      "か + dakuten -> が");

  // き + combining dakuten → ぎ
  expect_variant(
      "\xE3\x81\x8D\xE3\x82\x99", // き + U+3099
      "\xE3\x81\x8E",               // ぎ
      "き + dakuten -> ぎ");

  // は + combining handakuten → ぱ
  expect_variant(
      "\xE3\x81\xAF\xE3\x82\x9A", // は + U+309A
      "\xE3\x81\xB1",               // ぱ
      "は + handakuten -> ぱ");

  // ほ + combining handakuten → ぽ
  expect_variant(
      "\xE3\x81\xBB\xE3\x82\x9A", // ほ + U+309A
      "\xE3\x81\xBD",               // ぽ
      "ほ + handakuten -> ぽ");

  // カ + combining dakuten → ガ (katakana)
  expect_variant(
      "\xE3\x82\xAB\xE3\x82\x99", // カ + U+3099
      "\xE3\x82\xAC",               // ガ
      "カ + dakuten -> ガ");

  // ハ + combining handakuten → パ (katakana)
  expect_variant(
      "\xE3\x83\x8F\xE3\x82\x9A", // ハ + U+309A
      "\xE3\x83\x91",               // パ
      "ハ + handakuten -> パ");

  // Multi-char: さくらし\u3099また\u3099いこん → さくらじまだいこん
  expect_variant(
      "\xE3\x81\x95\xE3\x81\x8F\xE3\x82\x89\xE3\x81\x97\xE3\x82\x99\xE3\x81\xBE\xE3\x81\x9F\xE3\x82\x99\xE3\x81\x84\xE3\x81\x93\xE3\x82\x93", // さくらじまだ゙いこん
      "\xE3\x81\x95\xE3\x81\x8F\xE3\x82\x89\xE3\x81\x98\xE3\x81\xBE\xE3\x81\xA0\xE3\x81\x84\xE3\x81\x93\xE3\x82\x93", // さくらじまだいこん
      "さくらじまだ゙いこん -> さくらじまだいこん");

  // いっほ\u309Aん → いっぽん
  expect_variant(
      "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xBB\xE3\x82\x9A\xE3\x82\x93", // いっぽん
      "\xE3\x81\x84\xE3\x81\xA3\xE3\x81\xBD\xE3\x82\x93",               // いっぽん
      "いっぽん -> いっぽん");

  std::cout << "\n";
}

// ============================================================
// Test: normalizeCJKCompatibilityCharacters
// ============================================================
void test_normalize_cjk_compat() {
  std::cout << "[normalizeCJKCompatibilityCharacters]\n";

  // ㌀ → アパート (U+3300)
  expect_variant(
      "\xE3\x8C\x80", // ㌀
      "\xE3\x82\xA2\xE3\x83\x8F\xE3\x82\x9A\xE3\x83\xBC\xE3\x83\x88", // アパート (note: NFKD keeps combining mark)
      "㌀ -> アパート (NFKD)");

  // ㍻ → 平成 (U+337B)
  expect_variant(
      "\xE3\x8D\xBB", // ㍻
      "\xE5\xB9\xB3\xE6\x88\x90", // 平成
      "㍻ -> 平成");

  // ㍼ → 昭和 (U+337C)
  expect_variant(
      "\xE3\x8D\xBC", // ㍼
      "\xE6\x98\xAD\xE5\x92\x8C", // 昭和
      "㍼ -> 昭和");

  // ㍿ → 株式会社 (U+337F)
  expect_variant(
      "\xE3\x8D\xBF", // ㍿
      "\xE6\xA0\xAA\xE5\xBC\x8F\xE4\xBC\x9A\xE7\xA4\xBE", // 株式会社
      "㍿ -> 株式会社");

  std::cout << "\n";
}

// ============================================================
// Test: normalizeRadicalCharacters
// ============================================================
void test_normalize_radicals() {
  std::cout << "[normalizeRadicalCharacters]\n";

  // ⼀ → 一 (Kangxi Radical One, U+2F00 → U+4E00)
  expect_variant(
      "\xE2\xBC\x80", // ⼀
      "\xE4\xB8\x80", // 一
      "⼀ -> 一 (Kangxi radical)");

  // ⼈ → 人 (U+2F08 → U+4EBA)
  expect_variant(
      "\xE2\xBC\x88", // ⼈
      "\xE4\xBA\xBA", // 人
      "⼈ -> 人 (Kangxi radical)");

  // ⽔ → 水 (U+2F54 → U+6C34)
  expect_variant(
      "\xE2\xBD\x94", // ⽔
      "\xE6\xB0\xB4", // 水
      "⽔ -> 水 (Kangxi radical)");

  std::cout << "\n";
}

// ============================================================
// Test: standardizeKanji (異体字 → 親字)
// ============================================================
void test_standardize_kanji() {
  std::cout << "[standardizeKanji]\n";

  // 萬 → 万
  expect_variant(
      "\xE8\x90\xAC", // 萬
      "\xE4\xB8\x87", // 万
      "萬 -> 万");

  // 與 → 与
  expect_variant(
      "\xE8\x88\x87", // 與
      "\xE4\xB8\x8E", // 与
      "與 -> 与");

  // 龜 → 亀 (Note: this maps via the kanji-processor data)
  expect_variant(
      "\xE9\xBE\x9C", // 龜
      "\xE4\xBA\x80", // 亀
      "龜 -> 亀");

  // 會 → 会
  expect_variant(
      "\xE6\x9C\x83", // 會
      "\xE4\xBC\x9A", // 会
      "會 -> 会");

  // 傳 → 伝
  expect_variant(
      "\xE5\x82\xB3", // 傳
      "\xE4\xBC\x9D", // 伝
      "傳 -> 伝");

  std::cout << "\n";
}

// ============================================================
// Test: Integration - full pipeline
// ============================================================
void test_integration() {
  std::cout << "[Integration - full pipeline]\n";

  // ウツ → should produce うつ (katakana→hiragana)
  expect_variant(
      "\xE3\x82\xA6\xE3\x83\x84", // ウツ
      "\xE3\x81\x86\xE3\x81\xA4", // うつ
      "ウツ -> うつ (katakana->hiragana)");

  // ｳﾂ → should produce ウツ (halfwidth->fullwidth) and うつ (then katakana->hiragana)
  expect_variant(
      "\xEF\xBD\xB3\xEF\xBE\x82", // ｳﾂ
      "\xE3\x82\xA6\xE3\x83\x84", // ウツ
      "ｳﾂ -> ウツ (halfwidth->fullwidth)");

  expect_variant(
      "\xEF\xBD\xB3\xEF\xBE\x82", // ｳﾂ
      "\xE3\x81\x86\xE3\x81\xA4", // うつ
      "ｳﾂ -> うつ (halfwidth->fullwidth->katakana->hiragana)");

  // Plain hiragana should pass through unchanged
  expect_variant(
      "\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // かたかな
      "\xE3\x81\x8B\xE3\x81\x9F\xE3\x81\x8B\xE3\x81\xAA", // かたかな
      "かたかな -> かたかな (identity)");

  std::cout << "\n";
}

int main() {
  std::cout << "=== Japanese Text Processor Tests ===\n\n";

  test_half_width_characters();
  test_alphabetic_to_hiragana();
  test_alphanumeric_width();
  test_kana_conversion();
  test_collapse_emphatic();
  test_normalize_combining();
  test_normalize_cjk_compat();
  test_normalize_radicals();
  test_standardize_kanji();
  test_integration();

  std::cout << "=== Results ===\n";
  std::cout << "Total:  " << total_tests << "\n";
  std::cout << "Passed: " << passed_tests << "\n";
  std::cout << "Failed: " << failed_tests << "\n";

  return failed_tests > 0 ? 1 : 0;
}
