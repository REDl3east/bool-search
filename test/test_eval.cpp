#include <gtest/gtest.h>

#include "parser.h"

void parser_eval_test(std::string_view input, std::set<std::string_view> expected_id, std::string_view search, bool expected_result) {
  Parser p(input);
  auto parse_status = p.parse();

  ASSERT_EQ(parse_status, ParseStatus::OK) << "Parse failed with input: " << input;

  auto& id_map = p.get_id_map();

  for (auto& i : expected_id) {
    ASSERT_TRUE(id_map.find(i) != id_map.end()) << "'" << i << "' is not in identifer map!";
  }

  bool actual_value;
  auto eval_status = p.eval(search, &actual_value);

  ASSERT_EQ(eval_status, EvalStatus::OK) << "Eval failed with search: " << search;
  ASSERT_EQ(expected_result, actual_value) << "Eval expected value does not match with actual value. input: " << input;
}

TEST(ParserTest, ParserEvalTest) {
  parser_eval_test(
      "dog",
      {
          "dog",
      },
      "there better be dog in this searched text",
      true /**/
  );
  parser_eval_test(
      "not dog",
      {
          "dog",
      },
      "there better be dog in this searched text",
      false /**/
  );
  parser_eval_test(
      "not dog",
      {
          "dog",
      },
      "there is no d o g in this searched text",
      true /**/
  );
  parser_eval_test(
      "dog or cat and pig",
      {
          "dog",
          "cat",
          "pig",
      },
      "there once was a cat named pig",
      true /**/
  );
  parser_eval_test(
      "dog or cat and pig",
      {
          "dog",
          "cat",
          "pig",
      },
      "there once was a cat named shark",
      false /**/
  );
  parser_eval_test(
      "not not",
      {
          "not",
      },
      "there once was a cat named shark",
      true /**/
  );
  parser_eval_test(
      "not not",
      {
          "not",
      },
      "there once was a cat named not!",
      false /**/
  );
  parser_eval_test(
      "( burger or pizza or hotdog or pasta ) and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "I am having a pizza with egg on it",
      false /**/
  );
  parser_eval_test(
      "( burger or pizza or hotdog or pasta ) and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "I am having a pizza with burger and hotdog on it",
      true /**/
  );
  parser_eval_test(
      "burger or pizza or hotdog or not pasta and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "burger is good for me. I love egg too",
      true /**/
  );
  parser_eval_test(
      "burger or pizza or hotdog or not pasta and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "pizza is good for me. I love egg too",
      true /**/
  );
  parser_eval_test(
      "burger or pizza or hotdog or not pasta and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "hotdog is good for me. I love pasta too",
      true /**/
  );
  parser_eval_test(
      "( burger or pizza or hotdog or not pasta ) and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "There is pasta in my sauce",
      false /**/
  );
  parser_eval_test(
      "( burger or pizza or hotdog or not pasta ) and not egg",
      {
          "burger",
          "pizza",
          "hotdog",
          "pasta",
          "egg",
      },
      "There is hotdog in my sauce",
      true /**/
  );
  parser_eval_test(
      "( ( ( ( ( ( ( ( ( ( ( ( hello ) ) ) ) ) ) ) ) ) ) ) )",
      {
          "hello",
      },
      "Hello, I say.",
      false /**/
  );
  parser_eval_test(
      "( ( ( ( ( ( ( ( ( ( ( ( hello ) ) ) ) ) ) ) ) ) ) ) )",
      {
          "hello",
      },
      "I say hello.",
      true /**/
  );
  parser_eval_test(
      "not ( cats or dogs )",
      {
          "cats",
          "dogs",
      },
      "I contain the word cats in this sentence.",
      false /**/
  );
  parser_eval_test(
      "not cats and not dogs",
      {
          "cats",
          "dogs",
      },
      "I contain the word dogs in this sentence.",
      false /**/
  );
  parser_eval_test(
      "not ( cats or dogs )",
      {
          "cats",
          "dogs",
      },
      "I contain the word cat in this sentence.",
      true /**/
  );
  parser_eval_test(
      "not cats and not dogs",
      {
          "cats",
          "dogs",
      },
      "I contain the word dog in this sentence.",
      true /**/
  );
  parser_eval_test(
      "not cats and not dogs",
      {
          "cats",
          "dogs",
      },
      "I contain the word dog and cats in this sentence.",
      false /**/
  );
  parser_eval_test(
      "func1() or func2()",
      {
          "func1()",
          "func2()",
      },
      "This text contains a func1(), also func2()",
      true /**/
  );
  parser_eval_test(
      "func1() and func2()",
      {
          "func1()",
          "func2()",
      },
      "This text contains a func1(), also func3()",
      false /**/
  );
  parser_eval_test(
      "and",
      {
          "and",
      },
      "I love cheese and chicken",
      true /**/
  );
  parser_eval_test(
      "or",
      {
          "or",
      },
      "I love cheese or chicken",
      true /**/
  );
  parser_eval_test(
      "not",
      {
          "not",
      },
      "I love cheese, not chicken",
      true /**/
  );
  parser_eval_test(
      ")",
      {
          ")",
      },
      "I love cheese, not chicken",
      false /**/
  );
  parser_eval_test(
      "not and",
      {
          "and",
      },
      "I love cheese or chicken",
      true /**/
  );
  parser_eval_test(
      "not or",
      {
          "or",
      },
      "I love cheese and chicken",
      true /**/
  );
  parser_eval_test(
      "not not",
      {
          "not",
      },
      "I love cheese and chicken",
      true /**/
  );
  parser_eval_test(
      "dog and cat and horse and pizza and dog or cop",
      {
          "dog",
          "cat",
          "horse",
          "pizza",
          "cop",
      },
      "I have a dog, horse, cat, and a pizza",
      true /**/
  );
  parser_eval_test(
      "dog and cat and horse and pizza and dog or cop",
      {
          "dog",
          "cat",
          "horse",
          "pizza",
          "cop",
      },
      "I am a good cop man",
      true /**/
  );
  parser_eval_test(
      "dog and cat and horse and pizza and dog or cop",
      {
          "dog",
          "cat",
          "horse",
          "pizza",
          "cop",
      },
      "This text should return false my guy!!!",
      false /**/
  );
  parser_eval_test(
      "dog or \\(",
      {
          "dog",
          "(",
      },
      "This text should return false my guy!!!",
      false /**/
  );
  parser_eval_test(
      "dog or \\( and here",
      {
          "dog",
          "(",
      },
      "there is a ( in here",
      true /**/
  );

  parser_eval_test(
      "dog and ( \\( or here )",
      {
          "dog",
          "(",
          "here",
      },
      "I love dogs :(",
      true /**/
  );

  parser_eval_test(
      "dog and \\(ddd and here",
      {
          "dog",
          "\\(ddd",
      },
      "doggy! I love you so much! here I have a \\(ddd what do i do?",
      true /**/
  );
  parser_eval_test(
      "dog and cat and fish and bear and pizza and cake or cheese and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I want a dog that is friends with a cat and fish and bear and a pizza with a cake",
      true /**/
  );
  parser_eval_test(
      "dog and cat and fish and bear and pizza and cake or cheese and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I love cheese and crackers",
      true /**/
  );
  parser_eval_test(
      "dog and cat and fish and bear and pizza and cake or cheese and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I want a lot of donuts!!!",
      false /**/
  );
  parser_eval_test(
      "( dog or cat ) and ( fish or bear or pizza ) and ( cake  or cheese ) and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I have a dog that likes pizza or cake, with a side of crackers.",
      true /**/
  );
  parser_eval_test(
      "( dog or cat ) and ( fish or bear or pizza ) and ( cake  or cheese ) and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I have a cat that likes fish and cheese, with a side of crackers.",
      true /**/
  );
  parser_eval_test(
      "( dog or cat ) and ( fish or bear or pizza ) and ( cake  or cheese ) and cracker",
      {
          "dog",
          "cat",
          "fish",
          "bear",
          "pizza",
          "cake",
          "cheese",
          "cracker",
      },
      "I have a dog that likes pizza or cake, with a side of love.",
      false /**/
  );
  parser_eval_test(
      "#$@!%",
      {
          "#$@!%",
      },
      "What the #$@!% David Blaine",
      true /**/
  );
  parser_eval_test(
      "and and and or or or or",
      {
          "and",
          "or",
      },
      "There is pizza and burgers",
      true /**/
  );
  parser_eval_test(
      "and and and or or or or",
      {
          "and",
          "or",
      },
      "There is pizza burgers",
      false /**/
  );
}