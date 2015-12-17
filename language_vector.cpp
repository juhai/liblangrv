#include "language_vector.hpp"
#include <vector>
#include <cmath>

namespace {

  // Utility - apply F to each pair of elements in A and B,
  // which should be 'ForwardIterable'
  template<class A, class B, class F>
  void for_each_pair(A& a, B& b, F f) {
    const auto a_end = a.end();
    const auto b_end = b.end();
    auto a_it = a.begin();
    auto b_it = b.begin();
    while (true) {
      if (a_it == a_end || b_it == b_end) { break; }
      f(*a_it, *b_it);
      ++a_it;
      ++b_it;
    }
  }

  // std::vector<float> random_vector(std::size_t size, char seed) {
  //   // TODO: construct random vector of (1,-1)
  //   return std::vector<float>(size, 0.0f);
  // }

} // namespace (anonymous)

namespace language_vector {

  struct vector_impl {
    std::vector<float> data;
  };
  vector::vector(vector_impl* _impl) : impl(_impl) { }
  vector::~vector() { delete impl; }

  vector* build(const std::string& text, std::size_t order, std::size_t) {
    for (auto i = order; i < text.size(); ++i) {
      // TODO
    }
    return new vector(new vector_impl);
  }

  void merge(vector& language, const vector& text) {
    for_each_pair(language.impl->data, text.impl->data,
                  [](float& a, float b) {
                    a += b;
                  });
  }

  float score(const vector& language, const vector& text) {
    // just return the dot product between language & text
    auto sum_ab = 0.0f;
    auto sum_aa = 0.0f;
    auto sum_bb = 0.0f;
    for_each_pair(language.impl->data, text.impl->data,
                  [&](float a, float b) {
                    sum_ab += a * b;
                    sum_aa += a * a;
                    sum_bb += b * b;
                  });
    return sum_ab / std::sqrt(sum_aa * sum_bb);
  }

} // namespace language_vector
