#include "deinflector.hpp"

#include <algorithm>

#include "utf8.hpp"
Deinflector::Deinflector() { init_transforms(); }

// rules and descriptions adopted from
// https://github.com/yomidevs/yomitan/blob/master/ext/js/language/ja/japanese-transforms.js
void Deinflector::init_transforms() {
  int id =
      add_group({.name = "-ば",
                 .description = "1. Conditional form; shows that the previous stated condition\'s establishment is the "
                                "condition for the latter stated condition to occur.\n"
                                "2. Shows a trigger for a latter stated perception or judgment.\n"
                                "Usage: Attach ば to the hypothetical form (仮定形) of verbs and i-adjectives."});
  add_rule({.from = "ければ", .to = "い", .conditions_in = BA, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "えば", .to = "う", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "けば", .to = "く", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "げば", .to = "ぐ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "せば", .to = "す", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "てば", .to = "つ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ねば", .to = "ぬ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "べば", .to = "ぶ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "めば", .to = "む", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "れば", .to = "る", .conditions_in = BA, .conditions_out = V1 | V5 | VK | VS | VZ, .group_id = id});
  add_rule({.from = "れば", .to = "", .conditions_in = BA, .conditions_out = MASU, .group_id = id});

  id = add_group({.name = "-ゃ", .description = "Contraction of -ば."});
  add_rule({.from = "けりゃ", .to = "ければ", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "きゃ", .to = "ければ", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "や", .to = "えば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "きゃ", .to = "けば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "ぎゃ", .to = "げば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "しゃ", .to = "せば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "ちゃ", .to = "てば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "にゃ", .to = "ねば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "びゃ", .to = "べば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "みゃ", .to = "めば", .conditions_in = YA, .conditions_out = BA, .group_id = id});
  add_rule({.from = "りゃ", .to = "れば", .conditions_in = YA, .conditions_out = BA, .group_id = id});

  id = add_group({.name = "-ちゃ",
                  .description = "Contraction of ～ては.\n"
                                 "1. Explains how something always happens under the condition that it marks.\n"
                                 "2. Expresses the repetition (of a series of) actions.\n"
                                 "3. Indicates a hypothetical situation in which the speaker gives a (negative) "
                                 "evaluation about the other party\'s intentions.\n"
                                 "4. Used in \"Must Not\" patterns like ～てはいけない.\n"
                                 "Usage: Attach は after the て-form of verbs, contract ては into ちゃ."});
  add_rule({.from = "ちゃ", .to = "る", .conditions_in = V5, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いじゃ", .to = "ぐ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いちゃ", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しちゃ", .to = "す", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃ", .to = "う", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃ", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃ", .to = "つ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃ", .to = "る", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃ", .to = "ぬ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃ", .to = "ぶ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃ", .to = "む", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じちゃ", .to = "ずる", .conditions_in = V5, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しちゃ", .to = "する", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ちゃ", .to = "為る", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きちゃ", .to = "くる", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ちゃ", .to = "来る", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ちゃ", .to = "來る", .conditions_in = V5, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ちゃう",
                  .description = "Contraction of -しまう.\n"
                                 "Usage: Attach しまう after the て-form of verbs, contract てしまう into ちゃう."});
  add_rule({.from = "ちゃう", .to = "る", .conditions_in = V5, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いじゃう", .to = "ぐ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いちゃう", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しちゃう", .to = "す", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃう", .to = "う", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃう", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃう", .to = "つ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちゃう", .to = "る", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃう", .to = "ぬ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃう", .to = "ぶ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじゃう", .to = "む", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じちゃう", .to = "ずる", .conditions_in = V5, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しちゃう", .to = "する", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ちゃう", .to = "為る", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きちゃう", .to = "くる", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ちゃう", .to = "来る", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ちゃう", .to = "來る", .conditions_in = V5, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ちまう",
                  .description = "Contraction of -しまう.\n"
                                 "Usage: Attach しまう after the て-form of verbs, contract てしまう into ちまう."});
  add_rule({.from = "ちまう", .to = "る", .conditions_in = V5, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いじまう", .to = "ぐ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いちまう", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しちまう", .to = "す", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちまう", .to = "う", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちまう", .to = "く", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちまう", .to = "つ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "っちまう", .to = "る", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじまう", .to = "ぬ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじまう", .to = "ぶ", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んじまう", .to = "む", .conditions_in = V5, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じちまう", .to = "ずる", .conditions_in = V5, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しちまう", .to = "する", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ちまう", .to = "為る", .conditions_in = V5, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きちまう", .to = "くる", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ちまう", .to = "来る", .conditions_in = V5, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ちまう", .to = "來る", .conditions_in = V5, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-しまう",
                  .description =
                      "1. Shows a sense of regret/surprise when you did have volition in doing something, but it "
                      "turned out to be bad to do.\n"
                      "2. Shows perfective/punctual achievement. This shows that an action has been completed.\n"
                      "3. Shows unintentional action–“accidentally”.\n"
                      "Usage: Attach しまう after the て-form of verbs."});
  add_rule({.from = "てしまう", .to = "て", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "でしまう", .to = "で", .conditions_in = V5, .conditions_out = TE, .group_id = id});

  id = add_group({.name = "-なさい",
                  .description = "Polite imperative suffix.\n"
                                 "Usage: Attach なさい after the continuative form (連用形) of verbs."});
  add_rule({.from = "なさい", .to = "る", .conditions_in = NASAI, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いなさい", .to = "う", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "きなさい", .to = "く", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎなさい", .to = "ぐ", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しなさい", .to = "す", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ちなさい", .to = "つ", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "になさい", .to = "ぬ", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "びなさい", .to = "ぶ", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "みなさい", .to = "む", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "りなさい", .to = "る", .conditions_in = NASAI, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じなさい", .to = "ずる", .conditions_in = NASAI, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しなさい", .to = "する", .conditions_in = NASAI, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為なさい", .to = "為る", .conditions_in = NASAI, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きなさい", .to = "くる", .conditions_in = NASAI, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来なさい", .to = "来る", .conditions_in = NASAI, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來なさい", .to = "來る", .conditions_in = NASAI, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-そう",
                  .description =
                      "Appearing that; looking like.\n"
                      "Usage: Attach そう to the continuative form (連用形) of verbs, or to the stem of adjectives."});
  add_rule({.from = "そう", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "そう", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いそう", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "きそう", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎそう", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しそう", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ちそう", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "にそう", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "びそう", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "みそう", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "りそう", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じそう", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しそう", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為そう", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きそう", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来そう", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來そう", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id =
      add_group({.name = "-すぎる",
                 .description =
                     "Shows something \"is too...\" or someone is doing something \"too much\".\n"
                     "Usage: Attach すぎる to the continuative form (連用形) of verbs, or to the stem of adjectives."});
  add_rule({.from = "すぎる", .to = "い", .conditions_in = V1, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "すぎる", .to = "る", .conditions_in = V1, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いすぎる", .to = "う", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "きすぎる", .to = "く", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎすぎる", .to = "ぐ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "しすぎる", .to = "す", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ちすぎる", .to = "つ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "にすぎる", .to = "ぬ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "びすぎる", .to = "ぶ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "みすぎる", .to = "む", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "りすぎる", .to = "る", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じすぎる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しすぎる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為すぎる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きすぎる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来すぎる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來すぎる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id =
      add_group({.name = "-過ぎる",
                 .description =
                     "Shows something \"is too...\" or someone is doing something \"too much\".\n"
                     "Usage: Attach すぎる to the continuative form (連用形) of verbs, or to the stem of adjectives."});
  add_rule({.from = "過ぎる", .to = "い", .conditions_in = V1, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "過ぎる", .to = "る", .conditions_in = V1, .conditions_out = V1, .group_id = id});
  add_rule({.from = "い過ぎる", .to = "う", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "き過ぎる", .to = "く", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎ過ぎる", .to = "ぐ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "し過ぎる", .to = "す", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ち過ぎる", .to = "つ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "に過ぎる", .to = "ぬ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "び過ぎる", .to = "ぶ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "み過ぎる", .to = "む", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "り過ぎる", .to = "る", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じ過ぎる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "し過ぎる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為過ぎる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "き過ぎる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来過ぎる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來過ぎる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id = add_group(
      {.name = "-たい",
       .description =
           "1. Expresses the feeling of desire or hope.\n"
           "2. Used in ...たいと思います, an indirect way of saying what the speaker intends to do.\n"
           "Usage: Attach たい to the continuative form (連用形) of verbs. たい itself conjugates as i-adjective."});
  add_rule({.from = "たい", .to = "る", .conditions_in = ADJ_I, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いたい", .to = "う", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "きたい", .to = "く", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎたい", .to = "ぐ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "したい", .to = "す", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ちたい", .to = "つ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "にたい", .to = "ぬ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "びたい", .to = "ぶ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "みたい", .to = "む", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "りたい", .to = "る", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じたい", .to = "ずる", .conditions_in = ADJ_I, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "したい", .to = "する", .conditions_in = ADJ_I, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為たい", .to = "為る", .conditions_in = ADJ_I, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きたい", .to = "くる", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来たい", .to = "来る", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來たい", .to = "來る", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-たら",
                  .description = "1. Denotes the latter stated event is a continuation of the previous stated event.\n"
                                 "2. Assumes that a matter has been completed or concluded.\n"
                                 "Usage: Attach たら to the continuative form (連用形) of verbs after euphonic change "
                                 "form, かったら to the stem of i-adjectives."});
  add_rule({.from = "かったら", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "たら", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いたら", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いだら", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "したら", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったら", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったら", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったら", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだら", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだら", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだら", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じたら", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "したら", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為たら", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きたら", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来たら", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來たら", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "いったら", .to = "いく", .conditions_in = NONE, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "行ったら", .to = "行く", .conditions_in = NONE, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "ましたら", .to = "ます", .conditions_in = NONE, .conditions_out = MASU, .group_id = id});

  id = add_group({.name = "-たり",
                  .description =
                      "1. Shows two actions occurring back and forth (when used with two verbs).\n"
                      "2. Shows examples of actions and states (when used with multiple verbs and adjectives).\n"
                      "Usage: Attach たり to the continuative form (連用形) of verbs after euphonic change form, "
                      "かったり to the stem of i-adjectives"});
  add_rule({.from = "かったり", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "たり", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いたり", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いだり", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "したり", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったり", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったり", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ったり", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだり", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだり", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだり", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じたり", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "したり", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為たり", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きたり", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来たり", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來たり", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "いったり", .to = "いく", .conditions_in = NONE, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "行ったり", .to = "行く", .conditions_in = NONE, .conditions_out = V5S, .group_id = id});

  id = add_group(
      {.name = "-て",
       .description =
           "て-form.\n"
           "It has a myriad of meanings. Primarily, it is a conjunctive particle that connects two clauses together.\n"
           "Usage: Attach て to the continuative form (連用形) of verbs after euphonic change form, くて to the stem "
           "of i-adjectives."});
  add_rule({.from = "くて", .to = "い", .conditions_in = TE, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "て", .to = "る", .conditions_in = TE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いて", .to = "く", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いで", .to = "ぐ", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "して", .to = "す", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "って", .to = "う", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "って", .to = "つ", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "って", .to = "る", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んで", .to = "ぬ", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んで", .to = "ぶ", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んで", .to = "む", .conditions_in = TE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じて", .to = "ずる", .conditions_in = TE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "して", .to = "する", .conditions_in = TE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為て", .to = "為る", .conditions_in = TE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きて", .to = "くる", .conditions_in = TE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来て", .to = "来る", .conditions_in = TE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來て", .to = "來る", .conditions_in = TE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "いって", .to = "いく", .conditions_in = TE, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "行って", .to = "行く", .conditions_in = TE, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "まして", .to = "ます", .conditions_in = NONE, .conditions_out = MASU, .group_id = id});

  id = add_group({.name = "-ず",
                  .description = "1. Negative form of verbs.\n"
                                 "2. Continuative form (連用形) of the particle ぬ (nu).\n"
                                 "Usage: Attach ず to the irrealis form (未然形) of verbs."});
  add_rule({.from = "ず", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かず", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がず", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さず", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たず", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なず", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばず", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まず", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らず", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わず", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜず", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せず", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ず", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こず", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ず", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ず", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ぬ",
                  .description = "Negative form of verbs.\n"
                                 "Usage: Attach ぬ to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せぬ"});
  add_rule({.from = "ぬ", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かぬ", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がぬ", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さぬ", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たぬ", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なぬ", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばぬ", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まぬ", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らぬ", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わぬ", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜぬ", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せぬ", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ぬ", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こぬ", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ぬ", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ぬ", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ん",
                  .description = "Negative form of verbs; a sound change of ぬ.\n"
                                 "Usage: Attach ん to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せん"});
  add_rule({.from = "ん", .to = "る", .conditions_in = NN, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かん", .to = "く", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がん", .to = "ぐ", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さん", .to = "す", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たん", .to = "つ", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なん", .to = "ぬ", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばん", .to = "ぶ", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まん", .to = "む", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らん", .to = "る", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わん", .to = "う", .conditions_in = NN, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜん", .to = "ずる", .conditions_in = NN, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せん", .to = "する", .conditions_in = NN, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ん", .to = "為る", .conditions_in = NN, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こん", .to = "くる", .conditions_in = NN, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ん", .to = "来る", .conditions_in = NN, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ん", .to = "來る", .conditions_in = NN, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-んばかり",
                  .description =
                      "Shows an action or condition is on the verge of occurring, or an excessive/extreme degree.\n"
                      "Usage: Attach んばかり to the irrealis form (未然形) of verbs.\n"
                      "する becomes せんばかり"});
  add_rule({.from = "んばかり", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かんばかり", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がんばかり", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さんばかり", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たんばかり", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なんばかり", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばんばかり", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まんばかり", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らんばかり", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わんばかり", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜんばかり", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せんばかり", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為んばかり", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こんばかり", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来んばかり", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來んばかり", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-んとする",
                  .description = "1. Shows the speaker's will or intention.\n"
                                 "2. Shows an action or condition is on the verge of occurring.\n"
                                 "Usage: Attach んとする to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せんとする"});
  add_rule({.from = "んとする", .to = "る", .conditions_in = VS, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かんとする", .to = "く", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がんとする", .to = "ぐ", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さんとする", .to = "す", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たんとする", .to = "つ", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なんとする", .to = "ぬ", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばんとする", .to = "ぶ", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まんとする", .to = "む", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らんとする", .to = "る", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わんとする", .to = "う", .conditions_in = VS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜんとする", .to = "ずる", .conditions_in = VS, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せんとする", .to = "する", .conditions_in = VS, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為んとする", .to = "為る", .conditions_in = VS, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こんとする", .to = "くる", .conditions_in = VS, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来んとする", .to = "来る", .conditions_in = VS, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來んとする", .to = "來る", .conditions_in = VS, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-む",
                  .description = "Archaic.\n"
                                 "1. Shows an inference of a certain matter.\n"
                                 "2. Shows speaker's intention.\n"
                                 "Usage: Attach む to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せむ"});
  add_rule({.from = "む", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かむ", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がむ", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さむ", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たむ", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なむ", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばむ", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まむ", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らむ", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わむ", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜむ", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せむ", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為む", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こむ", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来む", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來む", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ざる",
                  .description = "Negative form of verbs.\n"
                                 "Usage: Attach ざる to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せざる"});
  add_rule({.from = "ざる", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かざる", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がざる", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さざる", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たざる", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なざる", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばざる", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まざる", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らざる", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わざる", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜざる", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せざる", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ざる", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こざる", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ざる", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ざる", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-ねば",
                  .description = "1. Shows a hypothetical negation; if not ...\n"
                                 "2. Shows a must. Used with or without ならぬ.\n"
                                 "Usage: Attach ねば to the irrealis form (未然形) of verbs.\n"
                                 "する becomes せねば"});
  add_rule({.from = "ねば", .to = "る", .conditions_in = BA, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かねば", .to = "く", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がねば", .to = "ぐ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さねば", .to = "す", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たねば", .to = "つ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なねば", .to = "ぬ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばねば", .to = "ぶ", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まねば", .to = "む", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らねば", .to = "る", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わねば", .to = "う", .conditions_in = BA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぜねば", .to = "ずる", .conditions_in = BA, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せねば", .to = "する", .conditions_in = BA, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ねば", .to = "為る", .conditions_in = BA, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こねば", .to = "くる", .conditions_in = BA, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ねば", .to = "来る", .conditions_in = BA, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ねば", .to = "來る", .conditions_in = BA, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-く", .description = "Adverbial form of i-adjectives."});
  add_rule({.from = "く", .to = "い", .conditions_in = KU, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "causative",
                  .description = "Describes the intention to make someone do something.\n"
                                 "Usage: Attach させる to the irrealis form (未然形) of ichidan verbs and くる.\n"
                                 "Attach せる to the irrealis form (未然形) of godan verbs and する.\n"
                                 "It itself conjugates as an ichidan verb."});
  add_rule({.from = "させる", .to = "る", .conditions_in = V1, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かせる", .to = "く", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がせる", .to = "ぐ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "させる", .to = "す", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たせる", .to = "つ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なせる", .to = "ぬ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばせる", .to = "ぶ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ませる", .to = "む", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らせる", .to = "る", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わせる", .to = "う", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じさせる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "ぜさせる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "させる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為せる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "せさせる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為させる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こさせる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来させる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來させる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "short causative",
                  .description = "Contraction of the causative form.\n"
                                 "Describes the intention to make someone do something.\n"
                                 "Usage: Attach す to the irrealis form (未然形) of godan verbs.\n"
                                 "Attach さす to the dictionary form (終止形) of ichidan verbs.\n"
                                 "する becomes さす, くる becomes こさす.\n"
                                 "It itself conjugates as an godan verb."});
  add_rule({.from = "さす", .to = "る", .conditions_in = V5SS, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かす", .to = "く", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がす", .to = "ぐ", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さす", .to = "す", .conditions_in = V5SS, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たす", .to = "つ", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なす", .to = "ぬ", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばす", .to = "ぶ", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ます", .to = "む", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らす", .to = "る", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わす", .to = "う", .conditions_in = V5SP, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じさす", .to = "ずる", .conditions_in = V5SS, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "ぜさす", .to = "ずる", .conditions_in = V5SS, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "さす", .to = "する", .conditions_in = V5SS, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為す", .to = "為る", .conditions_in = V5SS, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こさす", .to = "くる", .conditions_in = V5SS, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来さす", .to = "来る", .conditions_in = V5SS, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來さす", .to = "來る", .conditions_in = V5SS, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "imperative",
                  .description =
                      "1. To give orders.\n"
                      "2. (As あれ) Represents the fact that it will never change no matter the circumstances.\n"
                      "3. Express a feeling of hope."});
  add_rule({.from = "ろ", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "よ", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "え", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "け", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "げ", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "せ", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "て", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ね", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "べ", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "め", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "れ", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じろ", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "ぜよ", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しろ", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "せよ", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ろ", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為よ", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こい", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来い", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來い", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "continuative",
                  .description =
                      "Used to indicate actions that are (being) carried out.\n"
                      "Refers to 連用形, the part of the verb after conjugating with -ます and dropping ます."});
  add_rule({.from = "い", .to = "いる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "え", .to = "える", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "き", .to = "きる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "ぎ", .to = "ぎる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "け", .to = "ける", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "げ", .to = "げる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "じ", .to = "じる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "せ", .to = "せる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "ぜ", .to = "ぜる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "ち", .to = "ちる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "て", .to = "てる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "で", .to = "でる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "に", .to = "にる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "ね", .to = "ねる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "ひ", .to = "ひる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "び", .to = "びる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "へ", .to = "へる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "べ", .to = "べる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "み", .to = "みる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "め", .to = "める", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "り", .to = "りる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "れ", .to = "れる", .conditions_in = NONE, .conditions_out = V1D, .group_id = id});
  add_rule({.from = "い", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "き", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぎ", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "し", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ち", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "に", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "び", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "み", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "り", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "き", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "し", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "来", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "negative",
                  .description = "1. Negative form of verbs.\n"
                                 "2. Expresses a feeling of solicitation to the other party.\n"
                                 "Usage: Attach ない to the irrealis form (未然形) of verbs, くない to the stem of "
                                 "i-adjectives. ない itself conjugates as i-adjective. ます becomes ません."});
  add_rule({.from = "くない", .to = "い", .conditions_in = ADJ_I, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "ない", .to = "る", .conditions_in = ADJ_I, .conditions_out = V1, .group_id = id});
  add_rule({.from = "かない", .to = "く", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がない", .to = "ぐ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "さない", .to = "す", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "たない", .to = "つ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なない", .to = "ぬ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばない", .to = "ぶ", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まない", .to = "む", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "らない", .to = "る", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "わない", .to = "う", .conditions_in = ADJ_I, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じない", .to = "ずる", .conditions_in = ADJ_I, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しない", .to = "する", .conditions_in = ADJ_I, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ない", .to = "為る", .conditions_in = ADJ_I, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こない", .to = "くる", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ない", .to = "来る", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ない", .to = "來る", .conditions_in = ADJ_I, .conditions_out = VK, .group_id = id});
  add_rule({.from = "ません", .to = "ます", .conditions_in = MASEN, .conditions_out = MASU, .group_id = id});

  id = add_group({.name = "-さ",
                  .description = "Nominalizing suffix of i-adjectives indicating nature, state, mind or degree.\n"
                                 "Usage: Attach さ to the stem of i-adjectives."});
  add_rule({.from = "さ", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "passive",
                  .description = "1. Expression of being affected by an action (Passive).\n"
                                 "2. Respectful expression (Honorific).\n"
                                 "3. Spontaneity.\n"
                                 "4. Potential.\n"
                                 "Usage: Attach れる to the irrealis form (未然形) of godan verbs."});
  add_rule({.from = "かれる", .to = "く", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "がれる", .to = "ぐ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "される", .to = "す", .conditions_in = V1, .conditions_out = V5D | V5SP, .group_id = id});
  add_rule({.from = "たれる", .to = "つ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "なれる", .to = "ぬ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ばれる", .to = "ぶ", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "まれる", .to = "む", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "われる", .to = "う", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "られる", .to = "る", .conditions_in = V1, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じされる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "ぜされる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "される", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為れる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こられる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来られる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來られる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "-た",
                  .description =
                      "1. Indicates a reality that has happened in the past.\n"
                      "2. Indicates the completion of an action.\n"
                      "3. Indicates the confirmation of a matter.\n"
                      "4. Indicates the speaker's confidence that the action will definitely be fulfilled.\n"
                      "5. Indicates the events that occur before the main clause are represented as relative past.\n"
                      "6. Indicates a mild imperative/command.\n"
                      "Usage: Attach た to the continuative form (連用形) of verbs after euphonic change form, かった "
                      "to the stem of i-adjectives."});
  add_rule({.from = "かった", .to = "い", .conditions_in = TA, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "た", .to = "る", .conditions_in = TA, .conditions_out = V1, .group_id = id});
  add_rule({.from = "いた", .to = "く", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "いだ", .to = "ぐ", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "した", .to = "す", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "った", .to = "う", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "った", .to = "つ", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "った", .to = "る", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだ", .to = "ぬ", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだ", .to = "ぶ", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "んだ", .to = "む", .conditions_in = TA, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じた", .to = "ずる", .conditions_in = TA, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "した", .to = "する", .conditions_in = TA, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為た", .to = "為る", .conditions_in = TA, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きた", .to = "くる", .conditions_in = TA, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来た", .to = "来る", .conditions_in = TA, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來た", .to = "來る", .conditions_in = TA, .conditions_out = VK, .group_id = id});
  add_rule({.from = "いった", .to = "いく", .conditions_in = TA, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "行った", .to = "行く", .conditions_in = TA, .conditions_out = V5S, .group_id = id});
  add_rule({.from = "ました", .to = "ます", .conditions_in = TA, .conditions_out = MASU, .group_id = id});
  add_rule({.from = "でした", .to = "", .conditions_in = TA, .conditions_out = MASEN, .group_id = id});
  add_rule({.from = "かった", .to = "", .conditions_in = TA, .conditions_out = MASEN | NN, .group_id = id});

  id = add_group({.name = "-ます",
                  .description = "Polite conjugation of verbs and adjectives.\n"
                                 "Usage: Attach ます to the continuative form (連用形) of verbs."});
  add_rule({.from = "ます", .to = "る", .conditions_in = MASU, .conditions_out = V1, .group_id = id});
  add_rule({.from = "います", .to = "う", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "きます", .to = "く", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "ぎます", .to = "ぐ", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "します", .to = "す", .conditions_in = MASU, .conditions_out = V5D | V5S, .group_id = id});
  add_rule({.from = "ちます", .to = "つ", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "にます", .to = "ぬ", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "びます", .to = "ぶ", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "みます", .to = "む", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "ります", .to = "る", .conditions_in = MASU, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "じます", .to = "ずる", .conditions_in = MASU, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "します", .to = "する", .conditions_in = MASU, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為ます", .to = "為る", .conditions_in = MASU, .conditions_out = VS, .group_id = id});
  add_rule({.from = "きます", .to = "くる", .conditions_in = MASU, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来ます", .to = "来る", .conditions_in = MASU, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來ます", .to = "來る", .conditions_in = MASU, .conditions_out = VK, .group_id = id});
  add_rule({.from = "くあります", .to = "い", .conditions_in = MASU, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "potential",
                  .description = "Indicates a state of being (naturally) capable of doing an action.\n"
                                 "Usage: Attach (ら)れる to the irrealis form (未然形) of ichidan verbs.\n"
                                 "Attach る to the imperative form (命令形) of godan verbs.\n"
                                 "する becomes できる, くる becomes こ(ら)れる"});
  add_rule({.from = "れる", .to = "る", .conditions_in = V1, .conditions_out = V1 | V5D, .group_id = id});
  add_rule({.from = "える", .to = "う", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "ける", .to = "く", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "げる", .to = "ぐ", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "せる", .to = "す", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "てる", .to = "つ", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "ねる", .to = "ぬ", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "べる", .to = "ぶ", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "める", .to = "む", .conditions_in = V1, .conditions_out = V5D, .group_id = id});
  add_rule({.from = "できる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "出来る", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "これる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来れる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來れる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id = add_group({.name = "potential or passive",
                  .description = "1. Expression of being affected by an action (Passive).\n"
                                 "2. Respectful expression (Honorific).\n"
                                 "3. Spontaneity.\n"
                                 "4. Potential.\n"
                                 "5. Indicates a state of being (naturally) capable of doing an action.\n"
                                 "Usage: Attach られる to the irrealis form (未然形) of ichidan verbs.\n"
                                 "する becomes せられる, くる becomes こられる"});
  add_rule({.from = "られる", .to = "る", .conditions_in = V1, .conditions_out = V1, .group_id = id});
  add_rule({.from = "ざれる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "ぜられる", .to = "ずる", .conditions_in = V1, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "せられる", .to = "する", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為られる", .to = "為る", .conditions_in = V1, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こられる", .to = "くる", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来られる", .to = "来る", .conditions_in = V1, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來られる", .to = "來る", .conditions_in = V1, .conditions_out = VK, .group_id = id});

  id = add_group(
      {.name = "volitional",
       .description =
           "1. Expresses speaker\'s will or intention.\n"
           "2. Expresses an invitation to the other party.\n"
           "3. (Used in …ようとする) Indicates being on the verge of initiating an action or transforming a state.\n"
           "4. Indicates an inference of a matter.\n"
           "Usage: Attach よう to the irrealis form (未然形) of ichidan verbs.\n"
           "Attach う to the irrealis form (未然形) of godan verbs after -o euphonic change form.\n"
           "Attach かろう to the stem of i-adjectives (4th meaning only)."});
  add_rule({.from = "よう", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "おう", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "こう", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ごう", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "そう", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "とう", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "のう", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぼう", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "もう", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ろう", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じよう", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しよう", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為よう", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こよう", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来よう", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來よう", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "ましょう", .to = "ます", .conditions_in = NONE, .conditions_out = MASU, .group_id = id});
  add_rule({.from = "かろう", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "volitional slang",
                  .description = "Contraction of volitional form + か\n"
                                 "1. Expresses speaker's will or intention.\n"
                                 "2. Expresses an invitation to the other party.\n"
                                 "Usage: Replace final う with っ of volitional form then add か.\n"
                                 "For example: 行こうか -> 行こっか."});
  add_rule({.from = "よっか", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "おっか", .to = "う", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "こっか", .to = "く", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ごっか", .to = "ぐ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "そっか", .to = "す", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "とっか", .to = "つ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "のっか", .to = "ぬ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ぼっか", .to = "ぶ", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "もっか", .to = "む", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "ろっか", .to = "る", .conditions_in = NONE, .conditions_out = V5, .group_id = id});
  add_rule({.from = "じよっか", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しよっか", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為よっか", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こよっか", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来よっか", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來よっか", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "ましょっか", .to = "ます", .conditions_in = NONE, .conditions_out = MASU, .group_id = id});

  id = add_group({.name = "-まい",
                  .description = "Negative volitional form of verbs.\n"
                                 "1. Expresses speaker's assumption that something is likely not true.\n"
                                 "2. Expresses speaker's will or intention not to do something.\n"
                                 "Usage: Attach まい to the dictionary form (終止形) of verbs.\n"
                                 "Attach まい to the irrealis form (未然形) of ichidan verbs.\n"
                                 "する becomes しまい, くる becomes こまい"});
  add_rule({.from = "まい", .to = "", .conditions_in = NONE, .conditions_out = V, .group_id = id});
  add_rule({.from = "まい", .to = "る", .conditions_in = NONE, .conditions_out = V1, .group_id = id});
  add_rule({.from = "じまい", .to = "ずる", .conditions_in = NONE, .conditions_out = VZ, .group_id = id});
  add_rule({.from = "しまい", .to = "する", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "為まい", .to = "為る", .conditions_in = NONE, .conditions_out = VS, .group_id = id});
  add_rule({.from = "こまい", .to = "くる", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "来まい", .to = "来る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "來まい", .to = "來る", .conditions_in = NONE, .conditions_out = VK, .group_id = id});
  add_rule({.from = "まい", .to = "", .conditions_in = NONE, .conditions_out = MASU, .group_id = id});

  id =
      add_group({.name = "-おく",
                 .description = "To do certain things in advance in preparation (or in anticipation) of latter needs.\n"
                                "Usage: Attach おく to the て-form of verbs.\n"
                                "Attach でおく after ない negative form of verbs.\n"
                                "Contracts to とく・どく in speech."});
  add_rule({.from = "ておく", .to = "て", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "でおく", .to = "で", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "とく", .to = "て", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "どく", .to = "で", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "ないでおく", .to = "ない", .conditions_in = V5, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "ないどく", .to = "ない", .conditions_in = V5, .conditions_out = ADJ_I, .group_id = id});

  id =
      add_group({.name = "-いる",
                 .description =
                     "1. Indicates an action continues or progresses to a point in time.\n"
                     "2. Indicates an action is completed and remains as is.\n"
                     "3. Indicates a state or condition that can be taken to be the result of undergoing some change.\n"
                     "Usage: Attach いる to the て-form of verbs. い can be dropped in speech.\n"
                     "Attach でいる after ない negative form of verbs.\n"
                     "(Slang) Attach おる to the て-form of verbs. Contracts to とる・でる in speech."});
  add_rule({.from = "ている", .to = "て", .conditions_in = V1, .conditions_out = TE, .group_id = id});
  add_rule({.from = "ておる", .to = "て", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "てる", .to = "て", .conditions_in = V1P, .conditions_out = TE, .group_id = id});
  add_rule({.from = "でいる", .to = "で", .conditions_in = V1, .conditions_out = TE, .group_id = id});
  add_rule({.from = "でおる", .to = "で", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "でる", .to = "で", .conditions_in = V1P, .conditions_out = TE, .group_id = id});
  add_rule({.from = "とる", .to = "て", .conditions_in = V5, .conditions_out = TE, .group_id = id});
  add_rule({.from = "ないでいる", .to = "ない", .conditions_in = V1, .conditions_out = ADJ_I, .group_id = id});

  id = add_group(
      {.name = "-き",
       .description = "Attributive form (連体形) of i-adjectives. An archaic form that remains in modern Japanese."});
  add_rule({.from = "き", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "-げ",
                  .description = "Describes a person's appearance. Shows feelings of the person.\n"
                                 "Usage: Attach げ or 気 to the stem of i-adjectives"});
  add_rule({.from = "げ", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});
  add_rule({.from = "気", .to = "い", .conditions_in = NONE, .conditions_out = ADJ_I, .group_id = id});

  id = add_group({.name = "-がる",
                  .description =
                      "1. Shows subject’s feelings contrast with what is thought/known about them.\n"
                      "2. Indicates subject's behavior (stands out).\n"
                      "Usage: Attach がる to the stem of i-adjectives. It itself conjugates as a godan verb."});
  add_rule({.from = "がる", .to = "い", .conditions_in = V5, .conditions_out = ADJ_I, .group_id = id});

  // slang from line 1459 onwards missing
}

int Deinflector::add_group(const TransformGroup& group) {
  auto id = static_cast<int>(groups_.size());
  groups_.emplace_back(group);
  return id;
}

void Deinflector::add_rule(const Rule& rule) {
  transforms_[rule.from].emplace_back(rule);
  max_length_ = std::max(utf8::length(rule.from), max_length_);
}

std::vector<DeinflectionResult> Deinflector::deinflect(const std::string& text) const {
  std::vector<DeinflectionResult> result{};
  std::vector<TransformGroup> trace{};
  deinflect_recursive(text, NONE, trace, result);

  return result;
}

uint32_t Deinflector::pos_to_conditions(const std::vector<std::string>& parts_of_speech) {
  uint32_t result = 0;
  for (const auto& p : parts_of_speech) {
    if (p.starts_with("v")) {
      result |= V;
    }
    if (p.starts_with("v1")) {
      result |= V1;
    }
    if (p.starts_with("v5")) {
      result |= V5;
    }
    if (p == "vk") {
      result |= VK;
    }
    if (p == "vs") {
      result |= VS;
    }
    if (p == "vz") {
      result |= VZ;
    }
    if (p == "adj-i") {
      result |= ADJ_I;
    }
  }
  return result;
}

void Deinflector::deinflect_recursive(const std::string& text, uint32_t conditions, std::vector<TransformGroup>& trace,
                                      std::vector<DeinflectionResult>& results) const {
  results.emplace_back(text, conditions, trace);
  if (text.empty()) {
    return;
  }

  size_t text_len = utf8::length(text);
  for (size_t i = std::min(max_length_, text_len); i > 1; i--) {
    size_t prefix_chars = text_len - i;
    size_t prefix_bytes = utf8::byte_position(text, prefix_chars);

    std::string suffix = text.substr(prefix_bytes);
    auto it = transforms_.find(suffix);
    if (it == transforms_.end()) {
      continue;
    }

    std::string prefix = text.substr(0, prefix_bytes);
    for (const auto& rule : it->second) {
      if (conditions != NONE && !(conditions & rule.conditions_in)) {
        continue;
      }

      std::string transformed = prefix + rule.to;

      trace.emplace_back(groups_[rule.group_id]);
      deinflect_recursive(transformed, rule.conditions_out, trace, results);
      trace.pop_back();
    }
  }
}