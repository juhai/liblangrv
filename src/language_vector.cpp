#include "language_vector.hpp"
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

// *** Helpers ***

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

  // *** PIMPL definitions ***

  struct vector_impl {
    typedef std::vector<int32_t> data_t;
    data_t data;
  };

  struct builder_impl {
    typedef std::mt19937_64 generator_t;
    std::size_t order;
    std::size_t seed;
    // permutation[i] is the source for element 'i' in the destination
    //   target[i] <- source[permutation[i]
    std::vector<std::size_t> permutation;
    // permutation_order is just 'permutation' repeated 'order' times
    std::vector<std::size_t> permutation_order;

    builder_impl(std::size_t order, std::size_t n, std::size_t seed);

    // helper method: generator of [-1, 1] vectors given a character
    void get_char_hash(vector_impl::data_t& v, char c) const;

    vector* operator()(const std::string& text) const;
  };

  // *** Core ***

  builder_impl::builder_impl(std::size_t _order, std::size_t n, std::size_t _seed)
    : order(_order), seed(_seed), permutation(n), permutation_order(n) {

    // generate (consistent) random permutation 'permutation'
    std::iota(permutation.begin(), permutation.end(), 0);
    std::shuffle(permutation.begin(), permutation.end(), generator_t(seed));

    // generate 'permutation ^ order' (for sliding window)
    for (auto i = 0u; i < permutation.size(); ++i) {
      auto permuted = i;
      for (auto n = 0u; n < order; ++n) {
        permuted = permutation[permuted];
      }
      permutation_order[i] = permuted;
    }
  }

  // define generator of [-1, 1] vectors given a character
  void builder_impl::get_char_hash(vector_impl::data_t& v, char c) const {
    auto generator = generator_t(seed + c);
    auto distribution = std::bernoulli_distribution();
    for (auto i = 0u; i < v.size(); ++i) {
      v[i] = (distribution(generator) ? 1 : -1);
    }
  }

  vector* builder_impl::operator()(const std::string& text) const {
    const size_t n = permutation.size();
    vector_impl::data_t data(n, 0);
    vector_impl::data_t tmp(n);
    vector_impl::data_t ngram(n, 0);
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

      // Cycle around the ngram buffer
      if (++buffer_it == buffer.end()) {
        buffer_it = buffer.begin();
      }
    }

    // Wrap the data up to return to caller
    return new vector(std::unique_ptr<vector_impl>(new vector_impl{data}));
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

  // *** API wrappers ***

  vector::vector(std::unique_ptr<vector_impl>&& _impl) : impl(std::move(_impl)) { }
  vector::~vector() { }

  builder::builder(std::unique_ptr<builder_impl>&& _impl) : impl(std::move(_impl)) { }
  builder::~builder() { }

  vector* builder::operator()(const std::string& text) const {
    return (*impl)(text);
  }

  builder* make_builder(std::size_t order, std::size_t n, std::size_t seed) {
    return new builder(std::unique_ptr<builder_impl>(new builder_impl(order, n, seed)));
  }

} // namespace language_vector
