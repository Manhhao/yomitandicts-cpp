#include "text_processor.hpp"

#include <utf8.h>

#include <string_view>

namespace {
constexpr char32_t KATAKANA_SMALL_KA = U'ヵ';
constexpr char32_t KATAKANA_SMALL_KE = U'ヶ';
constexpr char32_t KANA_PROLONGED_SOUND_MARK = U'ー';

constexpr char32_t HIRAGANA_START = U'ぁ';
constexpr char32_t HIRAGANA_END = U'ゖ';

constexpr char32_t KATAKANA_START = U'ァ';
constexpr char32_t KATAKANA_END = U'ヶ';

constexpr int32_t KANA_OFFSET = KATAKANA_START - HIRAGANA_START;

constexpr std::u32string_view HIRAGANA_A_ROW = U"ぁあかがさざただなはばぱまゃやらゎわゕ";
constexpr std::u32string_view HIRAGANA_I_ROW = U"ぃいきぎしじちぢにひびぴみりゐ";
constexpr std::u32string_view HIRAGANA_U_ROW = U"ぅうくぐすずっつづぬふぶぷむゅゆるゔ";
constexpr std::u32string_view HIRAGANA_E_ROW = U"ぇえけげせぜてでねへべぺめれゑゖ";
constexpr std::u32string_view HIRAGANA_O_ROW = U"ぉおこごそぞとどのほぼぽもょよろを";

char32_t get_prolonged_hiragana(char32_t c) {
  if (HIRAGANA_A_ROW.contains(c)) {
    return U'あ';
  }
  if (HIRAGANA_I_ROW.contains(c)) {
    return U'い';
  }
  if (HIRAGANA_U_ROW.contains(c)) {
    return U'う';
  }
  if (HIRAGANA_E_ROW.contains(c)) {
    return U'え';
  }
  if (HIRAGANA_O_ROW.contains(c)) {
    return U'う';
  }
  return 0;
}

bool is_in_range(char32_t c, char32_t start, char32_t end) { return c >= start && c <= end; }

std::u32string hiragana_to_katakana(const std::u32string& text) {
  std::u32string result;
  for (char32_t c : text) {
    if (is_in_range(c, HIRAGANA_START, HIRAGANA_END)) {
      c = static_cast<char32_t>(c + KANA_OFFSET);
    }
    result += c;
  }
  return result;
}

std::u32string katakana_to_hiragana(const std::u32string& text) {
  std::u32string result;
  for (char32_t c : text) {
    if (c == KANA_PROLONGED_SOUND_MARK && !result.empty()) {
      const char32_t prolonged = get_prolonged_hiragana(result.back());
      if (prolonged != 0) {
        result += prolonged;
        continue;
      }
    }

    if (c != KATAKANA_SMALL_KA && c != KATAKANA_SMALL_KE && is_in_range(c, KATAKANA_START, KATAKANA_END)) {
      c = static_cast<char32_t>(c - KANA_OFFSET);
    }

    result += c;
  }
  return result;
}
}

std::vector<TextVariant> text_processor::process(const std::string& src) {
  const std::u32string text = utf8::utf8to32(src);
  const std::u32string hiragana = katakana_to_hiragana(text);
  const std::u32string katakana = hiragana_to_katakana(text);

  std::vector<TextVariant> result;
  result.push_back({src, 0});

  if (hiragana != text) {
    result.push_back({utf8::utf32to8(hiragana), 1});
  }

  if (katakana != text && katakana != hiragana) {
    result.push_back({utf8::utf32to8(katakana), 1});
  }

  return result;
}
