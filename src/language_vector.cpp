#include "language_vector.hpp"
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>
#include <cuchar>
#include <cassert>

// *** Helpers ***

namespace {

  // Utility - apply F to each pair of elements in A and B,
  // which should be 'ForwardIterable'
  template<class A, class B, class F>
  void for_each_pair(A& a, B& b, F f) {
    const auto a_end = std::end(a);
    const auto b_end = std::end(b);
    auto a_it = std::begin(a);
    auto b_it = std::begin(b);
    while (true) {
      if (a_it == a_end || b_it == b_end) { break; }
      f(*a_it, *b_it);
      ++a_it;
      ++b_it;
    }
  }

  template<class A, class B, class C, class F>
  void for_each_triple(A& a, B& b, C& c, F f) {
    const auto a_end = std::end(a);
    const auto b_end = std::end(b);
    auto a_it = std::begin(a);
    auto b_it = std::begin(b);
    while (true) {
      if (a_it == a_end || b_it == b_end) { break; }
      f(*a_it, c * *b_it);
      ++a_it;
      ++b_it;
    }
  }

} // namespace (anonymous)


namespace language_vector {

  // *** PIMPL definitions ***

  struct vector_impl {
    typedef std::vector<int64_t> data_t;
    data_t data;
  };

  struct builder_impl {
    typedef std::mt19937_64 generator_t;
    static constexpr auto generator_bits = 64;
    std::size_t order;
    std::size_t seed;

    // permutation[i] is the source for element 'i' in the destination
    //   target[i] <- source[permutation[i]
    std::vector<std::size_t> permutation;

    // permutation_order is just 'permutation' repeated 'order' times
    std::vector<std::size_t> permutation_order;

    builder_impl(std::size_t order, std::size_t n, std::size_t seed);

    vector* operator()(const std::string& text, const bool addSpace) const;
    vector* operator()(const std::vector<std::string>& lines,
                       const bool addSpace) const;
  };

  // *** Core ***

  builder_impl::builder_impl(std::size_t _order, std::size_t n, std::size_t _seed)
    : order{_order}, seed{_seed}, permutation(n), permutation_order(n) {

    // generate (consistent) random permutation 'permutation'
    std::iota(std::begin(permutation), std::end(permutation), 0);
    std::shuffle(std::begin(permutation), std::end(permutation), generator_t(seed));

    // generate 'permutation ^ order' (for sliding window)
    for (auto i = 0u; i < permutation.size(); ++i) {
      auto permuted = i;
      for (auto n = 0u; n < order; ++n) {
        permuted = permutation[permuted];
      }
      permutation_order[i] = permuted;
    }
  }

  vector* builder_impl::operator()(const std::string& text,
                                   const bool addSpace=true) const {
    const size_t n = permutation.size();
    vector_impl::data_t result(n, 0);

    // Working data - space for ngrams, temporary/scratch space,
    // and for memorized character vectors
    vector_impl::data_t ngram(n, 1);
    vector_impl::data_t tmp_ngram(n);
    std::vector<vector_impl::data_t> buffer(order+1, vector_impl::data_t(n, 1));
    auto buffer_it = std::begin(buffer);

    // Add a space at the end of line to make sure the final context is used.
    std::string eval_text = text;
    if (addSpace) {
      eval_text += " ";
    }
    std::mbstate_t state{}; // zero-initialized to initial state
    char32_t c32;
    const char *ptr = eval_text.c_str(), *end = eval_text.c_str() + eval_text.size() + 1;
    while (int rc = std::mbrtoc32(&c32, ptr, end - ptr, &state)) {
      assert(rc != -3);
      if (rc <= 0) {
        std::cerr << "Error processing input: " << eval_text << std::endl;
        break;
      }
      // Increment pointer by amount of bytes for current UTF32 value
      ptr += rc;
      // The oldest character should be removed from the ngram
      auto oldest_buffer_it = buffer_it + 1;
      if (oldest_buffer_it == std::end(buffer)) {
        oldest_buffer_it = std::begin(buffer);
      }
      const auto& old_char = *oldest_buffer_it;
      auto& new_char = *buffer_it;

      // We can do all computation in a single loop (as long as we're careful not to read
      // and write to the same vector)
      auto generator = generator_t{seed + c32};
      for (auto i = 0u; i < n; i += generator_bits) {
        auto gen = generator();
        for (auto j = 0u; j < std::min<size_t>(generator_bits, n - i); ++j, gen >>= 1) {
          const auto idx = i+j;

          // Generate a random element for the current character,
          // and save the character's pattern into the buffer (so it can be removed lated)
          const auto char_element = (gen & 1 ? 1 : -1);
          new_char[idx] = char_element;

          // Compute and save the updated ngram
          const auto ngram_element = ngram[permutation[idx]] * old_char[permutation_order[idx]] * char_element;
          tmp_ngram[idx] = ngram_element;

          // Accumulate the computed ngram into the result
          // Note that this 'incorrectly' adds leading ngrams (but these can be viewed
          // as representing start-of-sequence markers)
          result[idx] += ngram_element;
        }
      }

      // Swap should avoid copying/allocation
      swap(ngram, tmp_ngram);

      // Move to the next element in the buffer
      buffer_it = oldest_buffer_it;
    }

    // Wrap the result up to return to caller
    return new vector{std::unique_ptr<vector_impl>{new vector_impl{std::move(result)}}};
  }

  vector* builder_impl::operator()(const std::vector<std::string>& lines,
                                   const bool addSpace=true) const {
    const size_t n = permutation.size();
    vector_impl::data_t result(n, 0);

    for (auto text : lines) {
      // Working data - space for ngrams, temporary/scratch space,
      // and for memorized character vectors
      vector_impl::data_t ngram(n, 1);
      vector_impl::data_t tmp_ngram(n);
      std::vector<vector_impl::data_t> buffer(order+1, vector_impl::data_t(n, 1));
      auto buffer_it = std::begin(buffer);

      // Add a space at the end of line to make sure the final context is used.
      std::string eval_text = text;
      if (addSpace) {
        eval_text += " ";
      }
      std::mbstate_t state{}; // zero-initialized to initial state
      char32_t c32;
      const char *ptr = eval_text.c_str(), *end = eval_text.c_str() + eval_text.size() + 1;
      while (int rc = std::mbrtoc32(&c32, ptr, end - ptr, &state)) {
        assert(rc != -3);
        if (rc <= 0) {
          std::cerr << "Error processing input: " << eval_text << std::endl;
          break;
        }
        // Increment pointer by amount of bytes for current UTF32 value
        ptr += rc;
        // The oldest character should be removed from the ngram
        auto oldest_buffer_it = buffer_it + 1;
        if (oldest_buffer_it == std::end(buffer)) {
          oldest_buffer_it = std::begin(buffer);
        }
        const auto& old_char = *oldest_buffer_it;
        auto& new_char = *buffer_it;

        // We can do all computation in a single loop (as long as we're careful not to read
        // and write to the same vector)
        auto generator = generator_t{seed + c32};
        for (auto i = 0u; i < n; i += generator_bits) {
          auto gen = generator();
          for (auto j = 0u; j < std::min<size_t>(generator_bits, n - i); ++j, gen >>= 1) {
            const auto idx = i+j;

            // Generate a random element for the current character,
            // and save the character's pattern into the buffer (so it can be removed lated)
            const auto char_element = (gen & 1 ? 1 : -1);
            new_char[idx] = char_element;

            // Compute and save the updated ngram
            const auto ngram_element = ngram[permutation[idx]] * old_char[permutation_order[idx]] * char_element;
            tmp_ngram[idx] = ngram_element;

            // Accumulate the computed ngram into the result
            // Note that this 'incorrectly' adds leading ngrams (but these can be viewed
            // as representing start-of-sequence markers)
            result[idx] += ngram_element;
          }
        }

        // Swap should avoid copying/allocation
        swap(ngram, tmp_ngram);

        // Move to the next element in the buffer
        buffer_it = oldest_buffer_it;
      }
    }
    // Wrap the result up to return to caller
    return new vector{std::unique_ptr<vector_impl>{new vector_impl{std::move(result)}}};
  }

  void merge(vector& language, const vector& text) {
    for_each_pair(language.impl->data, text.impl->data,
                  [](int64_t& a, int64_t b) {
                    a += b;
                  });
  }

  void wmerge(vector& language, const vector& text, int64_t weight) {
    for_each_triple(language.impl->data, text.impl->data, weight,
                  [](int64_t& a, int64_t b) {
                    a += b;
                  });
  }

  float score(const vector& language, const vector& text) {
    // just return the dot product between language & text
    auto sum_aa = 0.0f;
    auto sum_ab = 0.0f;
    auto sum_bb = 0.0f;
    for_each_pair(language.impl->data, text.impl->data,
                  [&] (float a, float b) {
                    sum_aa += static_cast<float>(a) * a;
                    sum_ab += static_cast<float>(a) * b;
                    sum_bb += static_cast<float>(b) * b;
                  });
    return sum_ab / std::sqrt(sum_aa * sum_bb + 1e-9f);
  }

  // *** API wrappers ***

  vector::vector(std::unique_ptr<vector_impl>&& _impl) : impl{std::move(_impl)} { }
  vector::~vector() { }

  builder::builder(std::unique_ptr<builder_impl>&& _impl) : impl{std::move(_impl)} { }
  builder::~builder() { }

  vector* builder::operator()(const std::string& text,
                              const bool addSpace) const {
    return (*impl)(text, addSpace);
  }

  vector* builder::operator()(const std::vector<std::string>& lines,
                              const bool addSpace) const {
    return (*impl)(lines, addSpace);
  }

  void builder::save(const vector& language, std::ostream& out) const {
    // write out in a very simple line-delimited text format
    for (auto x : language.impl->data) {
      out << x << "\n";
    }
  }

  vector* builder::load(std::istream& in) const {
    vector_impl::data_t data;
    auto n = this->impl->permutation.size();
    data.reserve(n);
    for (auto i = 0u; i < n; ++i) {
      vector_impl::data_t::value_type value;
      in >> value;
      data.push_back(value);
    }
    return new vector{std::unique_ptr<vector_impl>{new vector_impl{std::move(data)}}};
  }

  builder* make_builder(std::size_t order, std::size_t n, std::size_t seed) {
    return new builder{std::unique_ptr<builder_impl>{new builder_impl{order, n, seed}}};
  }

} // namespace language_vector
