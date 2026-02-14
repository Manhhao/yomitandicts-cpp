#include "text_processor.hpp"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <utf8.h>

namespace {

// ============================================================
// TextProcessor struct
// ============================================================

struct TextProcessor {
  std::vector<int> options;
  std::function<std::u32string(const std::u32string&, int)> process;
};

// ============================================================
// Unicode constants
// ============================================================

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L21
constexpr uint32_t HIRAGANA_SMALL_TSU = 0x3063;
constexpr uint32_t KATAKANA_SMALL_TSU = 0x30c3;
constexpr uint32_t KATAKANA_SMALL_KA = 0x30f5;
constexpr uint32_t KATAKANA_SMALL_KE = 0x30f6;
constexpr uint32_t KANA_PROLONGED_SOUND_MARK = 0x30fc;

constexpr uint32_t HIRAGANA_CONVERSION_RANGE_START = 0x3041;
constexpr uint32_t HIRAGANA_CONVERSION_RANGE_END = 0x3096;

constexpr uint32_t KATAKANA_CONVERSION_RANGE_START = 0x30a1;
constexpr uint32_t KATAKANA_CONVERSION_RANGE_END = 0x30f6;

// Halfwidth katakana dakuten / handakuten combining marks
constexpr uint32_t HALFWIDTH_DAKUTEN = 0xff9e;
constexpr uint32_t HALFWIDTH_HANDAKUTEN = 0xff9f;

// Combining diacritical marks for kana
constexpr char32_t COMBINING_DAKUTEN = 0x3099;
constexpr char32_t COMBINING_HANDAKUTEN = 0x309a;

// CJK range constants
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L78
constexpr uint32_t CJK_COMPAT_RANGE_START = 0x3300;
constexpr uint32_t CJK_COMPAT_RANGE_END = 0x33ff;

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L104
constexpr uint32_t KANGXI_RADICALS_START = 0x2f00;
constexpr uint32_t KANGXI_RADICALS_END = 0x2fdf;

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L107
constexpr uint32_t CJK_RADICALS_SUPPLEMENT_START = 0x2e80;
constexpr uint32_t CJK_RADICALS_SUPPLEMENT_END = 0x2eff;

// ============================================================
// Data: Halfwidth Katakana Mapping
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L61

struct HalfwidthKatakanaEntry {
  char32_t base;       // fullwidth base form
  char32_t dakuten;    // fullwidth with dakuten (0 = invalid)
  char32_t handakuten; // fullwidth with handakuten (0 = invalid)
};

// clang-format off
const std::unordered_map<char32_t, HalfwidthKatakanaEntry> HALFWIDTH_KATAKANA_MAPPING = {
    {U'･', {U'・', 0, 0}},
    {U'ｦ', {U'ヲ', U'ヺ', 0}},
    {U'ｧ', {U'ァ', 0, 0}},
    {U'ｨ', {U'ィ', 0, 0}},
    {U'ｩ', {U'ゥ', 0, 0}},
    {U'ｪ', {U'ェ', 0, 0}},
    {U'ｫ', {U'ォ', 0, 0}},
    {U'ｬ', {U'ャ', 0, 0}},
    {U'ｭ', {U'ュ', 0, 0}},
    {U'ｮ', {U'ョ', 0, 0}},
    {U'ｯ', {U'ッ', 0, 0}},
    {U'ｰ', {U'ー', 0, 0}},
    {U'ｱ', {U'ア', 0, 0}},
    {U'ｲ', {U'イ', 0, 0}},
    {U'ｳ', {U'ウ', U'ヴ', 0}},
    {U'ｴ', {U'エ', 0, 0}},
    {U'ｵ', {U'オ', 0, 0}},
    {U'ｶ', {U'カ', U'ガ', 0}},
    {U'ｷ', {U'キ', U'ギ', 0}},
    {U'ｸ', {U'ク', U'グ', 0}},
    {U'ｹ', {U'ケ', U'ゲ', 0}},
    {U'ｺ', {U'コ', U'ゴ', 0}},
    {U'ｻ', {U'サ', U'ザ', 0}},
    {U'ｼ', {U'シ', U'ジ', 0}},
    {U'ｽ', {U'ス', U'ズ', 0}},
    {U'ｾ', {U'セ', U'ゼ', 0}},
    {U'ｿ', {U'ソ', U'ゾ', 0}},
    {U'ﾀ', {U'タ', U'ダ', 0}},
    {U'ﾁ', {U'チ', U'ヂ', 0}},
    {U'ﾂ', {U'ツ', U'ヅ', 0}},
    {U'ﾃ', {U'テ', U'デ', 0}},
    {U'ﾄ', {U'ト', U'ド', 0}},
    {U'ﾅ', {U'ナ', 0, 0}},
    {U'ﾆ', {U'ニ', 0, 0}},
    {U'ﾇ', {U'ヌ', 0, 0}},
    {U'ﾈ', {U'ネ', 0, 0}},
    {U'ﾉ', {U'ノ', 0, 0}},
    {U'ﾊ', {U'ハ', U'バ', U'パ'}},
    {U'ﾋ', {U'ヒ', U'ビ', U'ピ'}},
    {U'ﾌ', {U'フ', U'ブ', U'プ'}},
    {U'ﾍ', {U'ヘ', U'ベ', U'ペ'}},
    {U'ﾎ', {U'ホ', U'ボ', U'ポ'}},
    {U'ﾏ', {U'マ', 0, 0}},
    {U'ﾐ', {U'ミ', 0, 0}},
    {U'ﾑ', {U'ム', 0, 0}},
    {U'ﾒ', {U'メ', 0, 0}},
    {U'ﾓ', {U'モ', 0, 0}},
    {U'ﾔ', {U'ヤ', 0, 0}},
    {U'ﾕ', {U'ユ', 0, 0}},
    {U'ﾖ', {U'ヨ', 0, 0}},
    {U'ﾗ', {U'ラ', 0, 0}},
    {U'ﾘ', {U'リ', 0, 0}},
    {U'ﾙ', {U'ル', 0, 0}},
    {U'ﾚ', {U'レ', 0, 0}},
    {U'ﾛ', {U'ロ', 0, 0}},
    {U'ﾜ', {U'ワ', 0, 0}},
    {U'ﾝ', {U'ン', 0, 0}},
};
// clang-format on

// ============================================================
// Data: Romaji to Hiragana (ordered for replaceAll, longest match first)
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-kana-romaji-dicts.js#L20

struct RomajiEntry {
  std::u32string romaji;
  std::u32string kana;
};

// clang-format off
const std::vector<RomajiEntry>& get_romaji_to_hiragana() {
  static const std::vector<RomajiEntry> table = {
      // Double letters - must be matched first
      {U"qq", U"っq"}, {U"vv", U"っv"}, {U"ll", U"っl"}, {U"xx", U"っx"},
      {U"kk", U"っk"}, {U"gg", U"っg"}, {U"ss", U"っs"}, {U"zz", U"っz"},
      {U"jj", U"っj"}, {U"tt", U"っt"}, {U"dd", U"っd"}, {U"hh", U"っh"},
      {U"ff", U"っf"}, {U"bb", U"っb"}, {U"pp", U"っp"}, {U"mm", U"っm"},
      {U"yy", U"っy"}, {U"rr", U"っr"}, {U"ww", U"っw"}, {U"cc", U"っc"},

      // Length 4
      {U"hwyu", U"ふゅ"}, {U"xtsu", U"っ"}, {U"ltsu", U"っ"},

      // Length 3
      {U"vya", U"ゔゃ"}, {U"vyi", U"ゔぃ"}, {U"vyu", U"ゔゅ"}, {U"vye", U"ゔぇ"}, {U"vyo", U"ゔょ"},
      {U"kya", U"きゃ"}, {U"kyi", U"きぃ"}, {U"kyu", U"きゅ"}, {U"kye", U"きぇ"}, {U"kyo", U"きょ"},
      {U"gya", U"ぎゃ"}, {U"gyi", U"ぎぃ"}, {U"gyu", U"ぎゅ"}, {U"gye", U"ぎぇ"}, {U"gyo", U"ぎょ"},
      {U"sya", U"しゃ"}, {U"syi", U"しぃ"}, {U"syu", U"しゅ"}, {U"sye", U"しぇ"}, {U"syo", U"しょ"},
      {U"sha", U"しゃ"}, {U"shi", U"し"},   {U"shu", U"しゅ"}, {U"she", U"しぇ"}, {U"sho", U"しょ"},
      {U"zya", U"じゃ"}, {U"zyi", U"じぃ"}, {U"zyu", U"じゅ"}, {U"zye", U"じぇ"}, {U"zyo", U"じょ"},
      {U"tya", U"ちゃ"}, {U"tyi", U"ちぃ"}, {U"tyu", U"ちゅ"}, {U"tye", U"ちぇ"}, {U"tyo", U"ちょ"},
      {U"cha", U"ちゃ"}, {U"chi", U"ち"},   {U"chu", U"ちゅ"}, {U"che", U"ちぇ"}, {U"cho", U"ちょ"},
      {U"cya", U"ちゃ"}, {U"cyi", U"ちぃ"}, {U"cyu", U"ちゅ"}, {U"cye", U"ちぇ"}, {U"cyo", U"ちょ"},
      {U"dya", U"ぢゃ"}, {U"dyi", U"ぢぃ"}, {U"dyu", U"ぢゅ"}, {U"dye", U"ぢぇ"}, {U"dyo", U"ぢょ"},
      {U"tsa", U"つぁ"}, {U"tsi", U"つぃ"}, {U"tse", U"つぇ"}, {U"tso", U"つぉ"},
      {U"tha", U"てゃ"}, {U"thi", U"てぃ"}, {U"thu", U"てゅ"}, {U"the", U"てぇ"}, {U"tho", U"てょ"},
      {U"dha", U"でゃ"}, {U"dhi", U"でぃ"}, {U"dhu", U"でゅ"}, {U"dhe", U"でぇ"}, {U"dho", U"でょ"},
      {U"twa", U"とぁ"}, {U"twi", U"とぃ"}, {U"twu", U"とぅ"}, {U"twe", U"とぇ"}, {U"two", U"とぉ"},
      {U"dwa", U"どぁ"}, {U"dwi", U"どぃ"}, {U"dwu", U"どぅ"}, {U"dwe", U"どぇ"}, {U"dwo", U"どぉ"},
      {U"nya", U"にゃ"}, {U"nyi", U"にぃ"}, {U"nyu", U"にゅ"}, {U"nye", U"にぇ"}, {U"nyo", U"にょ"},
      {U"hya", U"ひゃ"}, {U"hyi", U"ひぃ"}, {U"hyu", U"ひゅ"}, {U"hye", U"ひぇ"}, {U"hyo", U"ひょ"},
      {U"bya", U"びゃ"}, {U"byi", U"びぃ"}, {U"byu", U"びゅ"}, {U"bye", U"びぇ"}, {U"byo", U"びょ"},
      {U"pya", U"ぴゃ"}, {U"pyi", U"ぴぃ"}, {U"pyu", U"ぴゅ"}, {U"pye", U"ぴぇ"}, {U"pyo", U"ぴょ"},
      {U"fya", U"ふゃ"}, {U"fyu", U"ふゅ"}, {U"fyo", U"ふょ"},
      {U"hwa", U"ふぁ"}, {U"hwi", U"ふぃ"}, {U"hwe", U"ふぇ"}, {U"hwo", U"ふぉ"},
      {U"mya", U"みゃ"}, {U"myi", U"みぃ"}, {U"myu", U"みゅ"}, {U"mye", U"みぇ"}, {U"myo", U"みょ"},
      {U"rya", U"りゃ"}, {U"ryi", U"りぃ"}, {U"ryu", U"りゅ"}, {U"rye", U"りぇ"}, {U"ryo", U"りょ"},
      {U"lyi", U"ぃ"},   {U"xyi", U"ぃ"},   {U"lye", U"ぇ"},   {U"xye", U"ぇ"},
      {U"xka", U"ヵ"},   {U"xke", U"ヶ"},   {U"lka", U"ヵ"},   {U"lke", U"ヶ"},
      {U"kwa", U"くぁ"}, {U"kwi", U"くぃ"}, {U"kwu", U"くぅ"}, {U"kwe", U"くぇ"}, {U"kwo", U"くぉ"},
      {U"gwa", U"ぐぁ"}, {U"gwi", U"ぐぃ"}, {U"gwu", U"ぐぅ"}, {U"gwe", U"ぐぇ"}, {U"gwo", U"ぐぉ"},
      {U"swa", U"すぁ"}, {U"swi", U"すぃ"}, {U"swu", U"すぅ"}, {U"swe", U"すぇ"}, {U"swo", U"すぉ"},
      {U"zwa", U"ずぁ"}, {U"zwi", U"ずぃ"}, {U"zwu", U"ずぅ"}, {U"zwe", U"ずぇ"}, {U"zwo", U"ずぉ"},
      {U"jya", U"じゃ"}, {U"jyi", U"じぃ"}, {U"jyu", U"じゅ"}, {U"jye", U"じぇ"}, {U"jyo", U"じょ"},
      {U"tsu", U"つ"},
      {U"xtu", U"っ"},   {U"ltu", U"っ"},
      {U"xya", U"ゃ"},   {U"lya", U"ゃ"},
      {U"wyi", U"ゐ"},
      {U"xyu", U"ゅ"},   {U"lyu", U"ゅ"},
      {U"wye", U"ゑ"},
      {U"xyo", U"ょ"},   {U"lyo", U"ょ"},
      {U"xwa", U"ゎ"},   {U"lwa", U"ゎ"},
      {U"wha", U"うぁ"}, {U"whi", U"うぃ"}, {U"whu", U"う"},   {U"whe", U"うぇ"}, {U"who", U"うぉ"},

      // Length 2
      {U"nn", U"ん"},   {U"n'", U"ん"},
      {U"va", U"ゔぁ"}, {U"vi", U"ゔぃ"}, {U"vu", U"ゔ"},   {U"ve", U"ゔぇ"}, {U"vo", U"ゔぉ"},
      {U"fa", U"ふぁ"}, {U"fi", U"ふぃ"}, {U"fe", U"ふぇ"}, {U"fo", U"ふぉ"},
      {U"xn", U"ん"},   {U"wu", U"う"},
      {U"xa", U"ぁ"},   {U"xi", U"ぃ"},   {U"xu", U"ぅ"},   {U"xe", U"ぇ"},   {U"xo", U"ぉ"},
      {U"la", U"ぁ"},   {U"li", U"ぃ"},   {U"lu", U"ぅ"},   {U"le", U"ぇ"},   {U"lo", U"ぉ"},
      {U"ye", U"いぇ"},
      {U"ka", U"か"},   {U"ki", U"き"},   {U"ku", U"く"},   {U"ke", U"け"},   {U"ko", U"こ"},
      {U"ga", U"が"},   {U"gi", U"ぎ"},   {U"gu", U"ぐ"},   {U"ge", U"げ"},   {U"go", U"ご"},
      {U"sa", U"さ"},   {U"si", U"し"},   {U"su", U"す"},   {U"se", U"せ"},   {U"so", U"そ"},
      {U"ca", U"か"},   {U"ci", U"し"},   {U"cu", U"く"},   {U"ce", U"せ"},   {U"co", U"こ"},
      {U"qa", U"くぁ"}, {U"qi", U"くぃ"}, {U"qu", U"く"},   {U"qe", U"くぇ"}, {U"qo", U"くぉ"},
      {U"za", U"ざ"},   {U"zi", U"じ"},   {U"zu", U"ず"},   {U"ze", U"ぜ"},   {U"zo", U"ぞ"},
      {U"ja", U"じゃ"}, {U"ji", U"じ"},   {U"ju", U"じゅ"}, {U"je", U"じぇ"}, {U"jo", U"じょ"},
      {U"ta", U"た"},   {U"ti", U"ち"},   {U"tu", U"つ"},   {U"te", U"て"},   {U"to", U"と"},
      {U"da", U"だ"},   {U"di", U"ぢ"},   {U"du", U"づ"},   {U"de", U"で"},   {U"do", U"ど"},
      {U"na", U"な"},   {U"ni", U"に"},   {U"nu", U"ぬ"},   {U"ne", U"ね"},   {U"no", U"の"},
      {U"ha", U"は"},   {U"hi", U"ひ"},   {U"hu", U"ふ"},   {U"fu", U"ふ"},   {U"he", U"へ"},   {U"ho", U"ほ"},
      {U"ba", U"ば"},   {U"bi", U"び"},   {U"bu", U"ぶ"},   {U"be", U"べ"},   {U"bo", U"ぼ"},
      {U"pa", U"ぱ"},   {U"pi", U"ぴ"},   {U"pu", U"ぷ"},   {U"pe", U"ぺ"},   {U"po", U"ぽ"},
      {U"ma", U"ま"},   {U"mi", U"み"},   {U"mu", U"む"},   {U"me", U"め"},   {U"mo", U"も"},
      {U"ya", U"や"},   {U"yu", U"ゆ"},   {U"yo", U"よ"},
      {U"ra", U"ら"},   {U"ri", U"り"},   {U"ru", U"る"},   {U"re", U"れ"},   {U"ro", U"ろ"},
      {U"wa", U"わ"},   {U"wi", U"うぃ"}, {U"we", U"うぇ"}, {U"wo", U"を"},

      // Length 1
      {U"a", U"あ"}, {U"i", U"い"}, {U"u", U"う"}, {U"e", U"え"}, {U"o", U"お"},

      // Length 1 Special/Symbols
      {U".", U"。"}, {U",", U"、"}, {U":", U"："}, {U"/", U"・"},
      {U"!", U"！"}, {U"?", U"？"}, {U"~", U"〜"}, {U"-", U"ー"},
      {U"\u2018", U"「"}, {U"\u2019", U"」"}, // ' '
      {U"\u201C", U"『"}, {U"\u201D", U"』"}, // " "
      {U"[", U"［"}, {U"]", U"］"}, {U"(", U"（"}, {U")", U"）"},
      {U"{", U"｛"}, {U"}", U"｝"}, {U" ", U"\u3000"},

      // n -> ん is a special case (must be last single letter)
      {U"n", U"ん"},
  };
  return table;
}
// clang-format on

// ============================================================
// Data: Unicode tables (CJK NFKD, Kangxi Radicals, Kanji Variants)
// ============================================================

#include "unicode_data.hpp"

// ============================================================
// Helper functions
// ============================================================

bool is_in_range(uint32_t c, uint32_t range_start, uint32_t range_end) {
  return c >= range_start && c <= range_end;
}

// Replace all non-overlapping occurrences of `from` with `to` in `str` (left to right)
std::u32string u32_replace_all(const std::u32string& str, const std::u32string& from, const std::u32string& to) {
  if (from.empty()) return str;
  std::u32string result;
  result.reserve(str.size());
  size_t pos = 0;
  while (pos < str.size()) {
    size_t found = str.find(from, pos);
    if (found == std::u32string::npos) {
      result.append(str, pos);
      break;
    }
    result.append(str, pos, found - pos);
    result.append(to);
    pos = found + from.size();
  }
  return result;
}

// ============================================================
// Kana vowel mapping (for prolonged sound mark resolution)
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L121

const std::unordered_map<char32_t, std::u32string> VOWEL_TO_KANA{
    {U'a', U"ぁあかがさざただなはばぱまゃやらゎわヵァアカガサザタダナハバパマャヤラヮワヵヷ"},
    {U'i', U"ぃいきぎしじちぢにひびぴみりゐィイキギシジチヂニヒビピミリヰヸ"},
    {U'u', U"ぅうくぐすずっつづぬふぶぷむゅゆるゥウクグスズッツヅヌフブプムュユルヴ"},
    {U'e', U"ぇえけげせぜてでねへべぺめれゑヶェエケゲセゼテデネヘベペメレヱヶヹ"},
    {U'o', U"ぉおこごそぞとどのほぼぽもょよろをォオコゴソゾトドノホボポモョヨロヲヺ"}};

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L131
std::unordered_map<char32_t, char32_t> build_kana_to_vowel_map() {
  std::unordered_map<char32_t, char32_t> map;
  for (const auto& [vowel, kana_string] : VOWEL_TO_KANA) {
    for (char32_t c : kana_string) {
      map.try_emplace(c, vowel);
    }
  }
  return map;
}

char32_t kana_to_vowel(char32_t kana) {
  static const auto KANA_TO_VOWEL = build_kana_to_vowel_map();
  auto it = KANA_TO_VOWEL.find(kana);
  if (it != KANA_TO_VOWEL.end()) {
    return it->second;
  }
  return 0;
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L155
char32_t get_prolonged_hiragana(char32_t prev) {
  switch (kana_to_vowel(prev)) {
    case U'a':
      return U'あ';
    case U'i':
      return U'い';
    case U'u':
      return U'う';
    case U'e':
      return U'え';
    case U'o':
      return U'う';
    default:
      return 0;
  }
}

// ============================================================
// 1. convertHalfWidthCharacters
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L530

std::u32string convert_half_width_kana_to_full_width(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());

  for (size_t i = 0; i < text.size(); ++i) {
    char32_t c = text[i];
    auto it = HALFWIDTH_KATAKANA_MAPPING.find(c);
    if (it == HALFWIDTH_KATAKANA_MAPPING.end()) {
      result += c;
      continue;
    }

    const auto& mapping = it->second;
    int index = 0;
    if (i + 1 < text.size()) {
      char32_t next = text[i + 1];
      if (next == HALFWIDTH_DAKUTEN) {
        index = 1;
      } else if (next == HALFWIDTH_HANDAKUTEN) {
        index = 2;
      }
    }

    char32_t mapped = 0;
    switch (index) {
      case 1:
        mapped = mapping.dakuten;
        break;
      case 2:
        mapped = mapping.handakuten;
        break;
      default:
        break;
    }

    if (index > 0 && mapped != 0) {
      result += mapped;
      ++i; // skip the combining mark
    } else {
      result += mapping.base;
    }
  }
  return result;
}

// ============================================================
// 2. alphabeticToHiragana
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-wanakana.js

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-wanakana.js#L96
// Fill gaps in sokuons that replaceAll misses due to non-iterative matching
// Example: っtっtっtっtっつ -> っっっっっっっっっつ
std::u32string fill_sokuon_gaps(const std::u32string& text) {
  // Pattern: っ[a-z](?=っ) -> っっ
  std::u32string result = text;
  bool changed = true;
  while (changed) {
    changed = false;
    for (size_t i = 0; i + 2 < result.size(); ++i) {
      if (result[i] == U'っ' && result[i + 1] >= U'a' && result[i + 1] <= U'z' && result[i + 2] == U'っ') {
        result[i + 1] = U'っ';
        changed = true;
      }
    }
  }
  // Pattern: ッ[A-Z](?=ッ) -> ッッ
  changed = true;
  while (changed) {
    changed = false;
    for (size_t i = 0; i + 2 < result.size(); ++i) {
      if (result[i] == U'ッ' && result[i + 1] >= U'A' && result[i + 1] <= U'Z' && result[i + 2] == U'ッ') {
        result[i + 1] = U'ッ';
        changed = true;
      }
    }
  }
  return result;
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-wanakana.js#L25
std::u32string convert_to_hiragana(const std::u32string& text) {
  // Lowercase
  std::u32string lowered;
  lowered.reserve(text.size());
  for (char32_t c : text) {
    if (c >= U'A' && c <= U'Z') {
      lowered += static_cast<char32_t>(c + (U'a' - U'A'));
    } else {
      lowered += c;
    }
  }

  // Replace all romaji with kana (longest match first, in order)
  for (const auto& entry : get_romaji_to_hiragana()) {
    lowered = u32_replace_all(lowered, entry.romaji, entry.kana);
  }
  return fill_sokuon_gaps(lowered);
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-wanakana.js#L117
std::u32string convert_alphabetic_to_kana(const std::u32string& text) {
  std::u32string part;
  std::u32string result;

  for (char32_t c : text) {
    uint32_t cp = static_cast<uint32_t>(c);
    char32_t normalized = 0;

    if (cp >= 0x41 && cp <= 0x5a) { // A-Z
      normalized = static_cast<char32_t>(cp + (0x61 - 0x41));
    } else if (cp >= 0x61 && cp <= 0x7a) { // a-z
      normalized = static_cast<char32_t>(cp);
    } else if (cp >= 0xff21 && cp <= 0xff3a) { // Ａ-Ｚ fullwidth
      normalized = static_cast<char32_t>(cp + (0x61 - 0xff21));
    } else if (cp >= 0xff41 && cp <= 0xff5a) { // ａ-ｚ fullwidth
      normalized = static_cast<char32_t>(cp + (0x61 - 0xff41));
    } else if (cp == 0x2d || cp == 0xff0d) { // '-' or fullwidth dash
      normalized = U'-';
    } else {
      if (!part.empty()) {
        result += convert_to_hiragana(part);
        part.clear();
      }
      result += c;
      continue;
    }
    part += normalized;
  }

  if (!part.empty()) {
    result += convert_to_hiragana(part);
  }
  return result;
}

// ============================================================
// 3. alphanumericWidthVariants
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L489
std::u32string convert_alphanumeric_to_full_width(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  for (char32_t c : text) {
    uint32_t cp = static_cast<uint32_t>(c);
    if (cp >= 0x30 && cp <= 0x39) { // 0-9
      cp += 0xff10 - 0x30;
    } else if (cp >= 0x41 && cp <= 0x5a) { // A-Z
      cp += 0xff21 - 0x41;
    } else if (cp >= 0x61 && cp <= 0x7a) { // a-z
      cp += 0xff41 - 0x61;
    }
    result += static_cast<char32_t>(cp);
  }
  return result;
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L509
std::u32string convert_full_width_alphanumeric_to_normal(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  for (char32_t c : text) {
    uint32_t cp = static_cast<uint32_t>(c);
    if (cp >= 0xff10 && cp <= 0xff19) { // ０-９
      cp -= 0xff10 - 0x30;
    } else if (cp >= 0xff21 && cp <= 0xff3a) { // Ａ-Ｚ
      cp -= 0xff21 - 0x41;
    } else if (cp >= 0xff41 && cp <= 0xff5a) { // ａ-ｚ
      cp -= 0xff41 - 0x61;
    }
    result += static_cast<char32_t>(cp);
  }
  return result;
}

// ============================================================
// 4. collapseEmphaticSequences
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L776

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L763
bool is_emphatic_code_point(uint32_t cp) {
  return cp == HIRAGANA_SMALL_TSU || cp == KATAKANA_SMALL_TSU || cp == KANA_PROLONGED_SOUND_MARK;
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L776
std::u32string collapse_emphatic_sequences(const std::u32string& text, bool full_collapse) {
  if (text.empty()) return text;

  // Find leading emphatics
  size_t left = 0;
  while (left < text.size() && is_emphatic_code_point(static_cast<uint32_t>(text[left]))) {
    ++left;
  }

  // Find trailing emphatics
  size_t right = text.size() - 1;
  while (right < text.size() && is_emphatic_code_point(static_cast<uint32_t>(text[right]))) {
    if (right == 0) break;
    --right;
  }

  // Whole string is emphatic
  if (left > right) {
    return text;
  }

  std::u32string leading = text.substr(0, left);
  std::u32string trailing = text.substr(right + 1);
  std::u32string middle;

  int32_t current_collapsed_cp = -1;
  for (size_t i = left; i <= right; ++i) {
    char32_t ch = text[i];
    uint32_t cp = static_cast<uint32_t>(ch);
    if (is_emphatic_code_point(cp)) {
      if (current_collapsed_cp != static_cast<int32_t>(cp)) {
        current_collapsed_cp = static_cast<int32_t>(cp);
        if (!full_collapse) {
          middle += ch;
          continue;
        }
      }
    } else {
      current_collapsed_cp = -1;
      middle += ch;
    }
  }

  return leading + middle + trailing;
}

// ============================================================
// 5. normalizeCombiningCharacters
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L608

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L582
bool dakuten_allowed(uint32_t cp) {
  return (cp >= 0x304B && cp <= 0x3068) || (cp >= 0x306F && cp <= 0x307B) || (cp >= 0x30AB && cp <= 0x30C8) ||
         (cp >= 0x30CF && cp <= 0x30DB);
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L596
bool handakuten_allowed(uint32_t cp) {
  return (cp >= 0x306F && cp <= 0x307B) || (cp >= 0x30CF && cp <= 0x30DB);
}

std::u32string normalize_combining_characters(const std::u32string& text) {
  if (text.size() <= 1) return text;

  std::u32string result;
  result.reserve(text.size());

  // Process from right to left (as in the JS implementation)
  // Build result in reverse, then reverse at the end
  std::u32string reversed;
  reversed.reserve(text.size());

  size_t i = text.size() - 1;
  while (i > 0) {
    if (text[i] == COMBINING_DAKUTEN) {
      uint32_t combinee = static_cast<uint32_t>(text[i - 1]);
      if (dakuten_allowed(combinee)) {
        reversed += static_cast<char32_t>(combinee + 1);
        if (i < 2) {
          // i-1 was index 0, we consumed both
          std::reverse(reversed.begin(), reversed.end());
          return reversed;
        }
        i -= 2;
        continue;
      }
    } else if (text[i] == COMBINING_HANDAKUTEN) {
      uint32_t combinee = static_cast<uint32_t>(text[i - 1]);
      if (handakuten_allowed(combinee)) {
        reversed += static_cast<char32_t>(combinee + 2);
        if (i < 2) {
          std::reverse(reversed.begin(), reversed.end());
          return reversed;
        }
        i -= 2;
        continue;
      }
    }
    reversed += text[i];
    --i;
  }
  // i === 0 when first character was not consumed by combining
  if (i == 0) {
    reversed += text[0];
  }

  std::reverse(reversed.begin(), reversed.end());
  return reversed;
}

// ============================================================
// 6. normalizeCJKCompatibilityCharacters
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L642
std::u32string normalize_cjk_compatibility_characters(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  for (char32_t c : text) {
    uint32_t cp = static_cast<uint32_t>(c);
    if (is_in_range(cp, CJK_COMPAT_RANGE_START, CJK_COMPAT_RANGE_END)) {
      auto it = CJK_COMPAT_NFKD.find(c);
      if (it != CJK_COMPAT_NFKD.end()) {
        result += it->second;
        continue;
      }
    }
    result += c;
  }
  return result;
}

// ============================================================
// 7. normalizeRadicalCharacters (from CJK-util.js)
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L104
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L107
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L123
// Note: CJK_STROKES_RANGE (L110) is also part of CJK_RADICALS_RANGES but has
// no NFKD decompositions in Unicode, so it is intentionally not handled here.

std::u32string normalize_radical_characters(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  for (char32_t c : text) {
    uint32_t cp = static_cast<uint32_t>(c);
    if (is_in_range(cp, KANGXI_RADICALS_START, KANGXI_RADICALS_END)) {
      auto it = KANGXI_RADICALS_NFKD.find(c);
      if (it != KANGXI_RADICALS_NFKD.end()) {
        result += it->second;
        continue;
      }
    } else if (is_in_range(cp, CJK_RADICALS_SUPPLEMENT_START, CJK_RADICALS_SUPPLEMENT_END)) {
      auto it = CJK_RADICALS_SUPPLEMENT_NFKD.find(c);
      if (it != CJK_RADICALS_SUPPLEMENT_NFKD.end()) {
        result += it->second;
        continue;
      }
    }
    result += c;
  }
  return result;
}

// ============================================================
// 8. standardizeKanji
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L113
// Source data: kanji-processor npm package (三省堂 全訳 漢辞海 第四版)

std::u32string convert_kanji_variants(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  for (char32_t c : text) {
    auto it = KANJI_VARIANTS.find(c);
    if (it != KANJI_VARIANTS.end()) {
      result += it->second;
    } else {
      result += c;
    }
  }
  return result;
}

// ============================================================
// Kana conversion (existing)
// ============================================================

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L472
std::u32string hiragana_to_katakana(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  const uint32_t offset = (KATAKANA_CONVERSION_RANGE_START - HIRAGANA_CONVERSION_RANGE_START);
  for (char32_t c : text) {
    if (is_in_range(c, HIRAGANA_CONVERSION_RANGE_START, HIRAGANA_CONVERSION_RANGE_END)) {
      c = static_cast<char32_t>(c + offset);
    }
    result += c;
  }
  return result;
}

// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese.js#L441
std::u32string katakana_to_hiragana(const std::u32string& text) {
  std::u32string result;
  result.reserve(text.size());
  const uint32_t offset = (HIRAGANA_CONVERSION_RANGE_START - KATAKANA_CONVERSION_RANGE_START);
  for (char32_t c : text) {
    switch (c) {
      case KATAKANA_SMALL_KA:
      case KATAKANA_SMALL_KE:
        break;
      case KANA_PROLONGED_SOUND_MARK:
        if (result.length() > 0) {
          const auto prolonged = get_prolonged_hiragana(result.at(result.length() - 1));
          if (prolonged != 0) {
            c = prolonged;
          }
        }
        break;
      default:
        if (is_in_range(c, KATAKANA_CONVERSION_RANGE_START, KATAKANA_CONVERSION_RANGE_END)) {
          c = static_cast<char32_t>(c + offset);
        }
        break;
    }
    result += c;
  }
  return result;
}

// ============================================================
// Processor registration
// ============================================================
// Order matches: https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/language-descriptors.js#L317

// Cached as static to avoid reconstructing std::function objects on every call.
const std::vector<TextProcessor>& get_japanese_processors() {
  static const std::vector<TextProcessor> processors = {
      // 1. convertHalfWidthCharacters: ﾖﾐﾁｬﾝ → ヨミチャン
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L33
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? convert_half_width_kana_to_full_width(text) : text;
       }},

      // 2. alphabeticToHiragana: yomichan → よみちゃん
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L41
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? convert_alphabetic_to_kana(text) : text;
       }},

      // 3. normalizeCombiningCharacters: ド → ド (U+30C8 U+3099 → U+30C9)
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L97
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? normalize_combining_characters(text) : text;
       }},

      // 4. normalizeCJKCompatibilityCharacters: ㌀ → アパート
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L105
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? normalize_cjk_compatibility_characters(text) : text;
       }},

      // 5. normalizeRadicalCharacters: ⼀ → 一 (U+2F00 → U+4E00)
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/CJK-util.js#L133
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? normalize_radical_characters(text) : text;
       }},

      // 6. alphanumericWidthVariants: ｙｏｍｉｔａｎ → yomitan and vice versa
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L49
      {.options = {0, 1, 2},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         switch (opt) {
           case 1:
             return convert_full_width_alphanumeric_to_normal(text);
           case 2:
             return convert_alphanumeric_to_full_width(text);
           default:
             return text;
         }
       }},

      // 7. convertHiraganaToKatakana: よみちゃん → ヨミチャン and vice versa
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L66
      {.options = {0, 1, 2},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         switch (opt) {
           case 1:
             return katakana_to_hiragana(text);
           case 2:
             return hiragana_to_katakana(text);
           default:
             return text;
         }
       }},

      // 8. collapseEmphaticSequences: すっっごーーい → すっごーい / すごい
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L83
      {.options = {0, 1, 2},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         switch (opt) {
           case 1:
             return collapse_emphatic_sequences(text, false);
           case 2:
             return collapse_emphatic_sequences(text, true);
           default:
             return text;
         }
       }},

      // 9. standardizeKanji: 萬 → 万
      // https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/ja/japanese-text-preprocessors.js#L113
      {.options = {0, 1},
       .process =
           [](const std::u32string& text, int opt) -> std::u32string {
         return opt ? convert_kanji_variants(text) : text;
       }},
  };
  return processors;
}

}  // namespace

// ============================================================
// Public API
// ============================================================
// https://github.com/yomidevs/yomitan/blob/81d17d877fb18c62ba826210bf6db2b7f4d4deed/ext/js/language/translator.js#L564

std::vector<TextVariant> text_processor::process(const std::string& src) {
  std::u32string text = utf8::utf8to32(src);
  std::map<std::u32string, int> variants = {{text, 0}};

  for (const auto& processor : get_japanese_processors()) {
    std::map<std::u32string, int> next;

    for (const auto& [variant, steps] : variants) {
      for (int option : processor.options) {
        auto processed = processor.process(variant, option);
        int new_steps = (processed == variant) ? steps : steps + 1;

        auto [it, inserted] = next.try_emplace(processed, new_steps);
        if (!inserted && new_steps < it->second) {
          it->second = new_steps;
        }
      }
    }
    variants = std::move(next);
  }

  std::vector<TextVariant> results;
  results.reserve(variants.size());
  for (const auto& [variant, steps] : variants) {
    results.emplace_back(utf8::utf32to8(variant), steps);
  }
  return results;
}
