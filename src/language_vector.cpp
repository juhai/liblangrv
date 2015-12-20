#include "language_vector.hpp"
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

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

  void add(std::vector<int32_t>& as, const std::vector<int32_t>& bs) {
    for_each_pair(as, bs,
                  [](int32_t& a, int32_t b) {
                    a += b;
                  });
  }

} // namespace (anonymous)

namespace language_vector {

  struct vector_impl {
    typedef std::vector<int32_t> data_t;
    data_t data;
  };
  vector::vector(vector_impl* _impl) : impl(_impl) { }
  vector::~vector() { delete impl; }

  vector* build(const std::string& text, std::size_t order,
                std::size_t n, std::size_t seed) {
    typedef std::mt19937_64 generator_t;

    // generate (consistent) random permutation 'permutation'
    // permutation[i] is the source for element 'i' in the destination
    //   target[i] <- source[permutation[i]
    std::vector<std::size_t> permutation(n);
    std::iota(permutation.begin(), permutation.end(), 0);
    generator_t generator(seed);
    std::shuffle(permutation.begin(), permutation.end(), generator);

    // generate 'permutation ^ order' (for sliding window)
    auto permutation_order = std::vector<std::size_t>(n);
    for (auto i = 0u; i < permutation.size(); ++i) {
      auto permuted = i;
      for (auto n = 0u; n < order; ++n) {
        permuted = permutation[permuted];
      }
      permutation_order[i] = permuted;
    }

    // define generator of [-1, 1] vectors given a character
    auto get_char_hash = [seed] (vector_impl::data_t& v, char c) {
      auto generator = generator_t(seed + c);
      auto distribution = std::bernoulli_distribution();
      for (auto i = 0u; i < v.size(); ++i) {
        v[i] = (distribution(generator) ? 1 : -1);
      }
    };

    // Loop to assimilate data
    vector_impl::data_t data(n, 0);
    vector_impl::data_t ngram(n, 0);
    vector_impl::data_t tmp(n);
    std::vector<vector_impl::data_t> buffer(order);
    auto buffer_it = buffer.begin();
    for (auto text_char : text) {
      // 1. permute 'data'
      for (auto i = 0u; i < data.size(); ++i) {
        tmp[i] = data[permutation[i]];
      }
      std::swap(tmp, data);

      // 2. remove the (i-order+1)'th character from 'ngram'
      if (buffer_it->empty()) {
        *buffer_it = vector_impl::data_t(n, 0);
      } else {
        for (auto i = 0u; i < buffer_it->size(); ++i) {
          ngram[i] -= (*buffer_it)[permutation_order[i]];
        }
      }

      // 3. add the current character to 'ngram'
      get_char_hash(*buffer_it, text_char);
      add(ngram, *buffer_it);

      // 3. add the current 'ngram' to 'data'
      add(data, ngram);

      if (++buffer_it == buffer.end()) {
        buffer_it = buffer.begin();
      }
    }

    return new vector(new vector_impl{data});
  }

  void merge(vector& language, const vector& text) {
    add(language.impl->data, text.impl->data);
  }

  float score(const vector& language, const vector& text) {
    // just return the dot product between language & text
    auto sum_aa = 0.0f;
    auto sum_ab = 0.0f;
    auto sum_bb = 0.0f;
    for_each_pair(language.impl->data, text.impl->data,
                  [&](float a, float b) {
                    sum_aa += static_cast<float>(a) * a;
                    sum_ab += static_cast<float>(a) * b;
                    sum_bb += static_cast<float>(b) * b;
                  });
    return sum_ab / std::sqrt(sum_aa * sum_bb + 1e-9f);
  }

} // namespace language_vector
