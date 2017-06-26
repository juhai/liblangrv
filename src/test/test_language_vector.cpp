#include "language_vector.hpp"
#include <memory>
#include <iostream>
#include <catch.hpp>

namespace {

  std::unique_ptr<language_vector::builder> make_builder(size_t order = 3) {
    return std::unique_ptr<language_vector::builder>{language_vector::make_builder(order, 10000, 42)};
  }

  std::unique_ptr<language_vector::vector> build(const std::string& text, size_t order = 3,
                                                 bool addSpace = true) {
    auto builder = make_builder(order);
    return std::unique_ptr<language_vector::vector>{(*builder)(text, addSpace)};
  }
  std::unique_ptr<language_vector::vector> builds(const std::vector<std::string>& lines,
                                                  size_t order = 3,
                                                  bool addSpace = true) {
    auto builder = make_builder(order);
    return std::unique_ptr<language_vector::vector>{(*builder)(lines, addSpace)};
  }

} // namespace (anonymous)

using Catch::Detail::Approx;

TEST_CASE("Language vectors can be built and scored", "") {
  auto easy = build("abc 123");
  // full overlap
  REQUIRE(language_vector::score(*easy, *easy) == Approx(1));
  REQUIRE(language_vector::score(*easy, *build("abc 123")) == Approx(1));

  // partial overlap
  REQUIRE(language_vector::score(*easy, *build("abc")) < 0.99f);
  REQUIRE(language_vector::score(*easy, *build("123 abc")) < 0.99f);

  // there is no overlap here (just random coincidence of ngram vectors)
  REQUIRE(language_vector::score(*build("vwxyz"), *build("abcde")) < 0.01f);
}

TEST_CASE("Language vectors can be built from a vector of strings", "") {
  std::vector<std::string> data =
    {"In the beginning God created the heaven and the earth.",
     "And the earth was without form, and void; and darkness was upon the face of the deep. And the Spirit of God moved upon the face of the waters.",
     "And God said, Let there be light: and there was light.",
     "And God saw the light, that it was good: and God divided the light from the darkness.",
     "And God called the light Day, and the darkness he called Night. And the evening and the morning were the first day.",
     "And God said, Let there be a firmament in the midst of the waters, and let it divide the waters from the waters.",
    };

  // Default behaviour is to add space after each line to ensure final end
  // of word is triggered. Here we need to init non_batch with empty string
  // and not add space
  auto batch = builds(data, 3, true);
  auto non_batch = build("", 3, false);
  for (auto it=data.begin();it < data.end();++it) {
    merge(*non_batch, *build(*it));
  };

  // Batch and non-batch should return the same language vector
  REQUIRE(language_vector::score(*batch, *batch) == Approx(1));
  REQUIRE(language_vector::score(*non_batch, *non_batch) == Approx(1));
  REQUIRE(language_vector::score(*batch, *non_batch) == Approx(1));
}

TEST_CASE("Language vectors obey ngram invariance", "[ngram]") {
  // bigrams should be agnostic to the following, as the ngrams are both
  // .a a. .b b.
  REQUIRE(language_vector::score(*build(".a.b.", 2), *build(".b.a.", 2)) == Approx(1));
  REQUIRE(language_vector::score(*build(".a.b."), *build(".b.a.")) < 0.99f);

  // trigrams should be agnostic to the following, as the ngrams are both
  // ..a .a. a.. ..b .b. b..
  REQUIRE(language_vector::score(*build("..a..b.."), *build("..b..a..")) == Approx(1));
}

TEST_CASE("Default space padding should work") {
  // By default, each string has 0x20 appended to it before processing
  // Test that disabling it creates a distinguishable vector

  // Check that default equals same as when true given
  REQUIRE(language_vector::score(*build("a", 2),
                                 *build("a", 2, true)) == Approx(1));
  // Giving false should result in small cosine similarity
  REQUIRE(language_vector::score(*build("a", 2, true),
                                 *build("a", 2, false)) < 0.99f);
}

TEST_CASE("Language vectors can be merged", "") {
  // merge two identical documents in different orders - should be the same
  auto a = build("abcdef");
  language_vector::merge(*a, *build("12345"));
  auto b = build("12345");
  language_vector::merge(*b, *build("abcdef"));
  REQUIRE(language_vector::score(*a, *b) == Approx(1));

  // compare a 'doubled up' merged language vector to a single vector
  auto once = build("abc def");
  auto twice = build("abc def");
  language_vector::merge(*twice, *build("abc def"));
  REQUIRE(language_vector::score(*once, *twice) == Approx(1));

  // merging should move a language vector away from the original
  auto x = build("abcdef");
  language_vector::merge(*x, *build("fedcba"));
  REQUIRE(language_vector::score(*x, *build("abcdef")) < 0.99f);

  // Weighted merging should be order dependent, i.e. the result not similar
  auto wa = build("abcdef");
  language_vector::wmerge(*wa, *build("12345"), 2);
  auto wb = build("12345");
  language_vector::wmerge(*wb, *build("abcdef"), 2);
  REQUIRE(language_vector::score(*wa, *wb) < 0.99f);

  // Weighted merging with unity weight should give same result
  auto wua = build("abcdef");
  language_vector::wmerge(*wua, *build("12345"), 1);
  auto wub = build("12345");
  language_vector::wmerge(*wub, *build("abcdef"), 1);
  REQUIRE(language_vector::score(*wua, *wub) == Approx(1));
}

TEST_CASE("Language vectors can be serialized/deserialized", "[io]") {
  auto builder = make_builder();
  auto round_trip = [&builder](const language_vector::vector& original) {
    std::stringstream stream;
    builder->save(original, stream);
    return std::unique_ptr<language_vector::vector>{builder->load(stream)};
  };

  auto x = (*builder)("This is some text");
  auto y = (*builder)("C'est autres text");

  auto x_reload = round_trip(*x);
  auto y_reload = round_trip(*y);

  REQUIRE(language_vector::score(*x, *x_reload) == Approx(1));
  REQUIRE(language_vector::score(*y, *y_reload) == Approx(1));
  REQUIRE(language_vector::score(*x_reload, *y_reload) < 0.99f);
}

TEST_CASE("Example of larger language vectors", "") {
  auto en = build("this is an impossibly small amount of text, written in English");
  auto en_more = build("another document, also written in the Queen's language");
  language_vector::merge(*en, *en_more);
  auto fr = build("c'est le premiere heure depuis minuit, non?");

  REQUIRE(language_vector::score(*en, *en) == Approx(1));
  REQUIRE(language_vector::score(*fr, *fr) == Approx(1));
  REQUIRE(language_vector::score(*en, *fr) == Approx(language_vector::score(*fr, *en)));
  REQUIRE(language_vector::score(*en, *fr) < 0.99f);
}
