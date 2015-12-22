#ifndef LANGUAGE_VECTOR_HPP
#define LANGUAGE_VECTOR_HPP

#include <string>
#include <memory>

namespace language_vector {

  // An opaque type - use the following methods to construct & fiddle with it.
  struct vector_impl;
  struct vector {
    explicit vector(std::unique_ptr<vector_impl>&&);
    ~vector();
    std::unique_ptr<vector_impl> impl;
  };

  // Builder for language vectors
  struct builder_impl;
  struct builder {
    // Construct a random vector representation of the given text
    vector* operator()(const std::string& text) const;

    explicit builder(std::unique_ptr<builder_impl>&&);
    ~builder();
    std::unique_ptr<builder_impl> impl;
  };

  // Create a builder, which may be used to construct language vectors
  builder* make_builder(std::size_t order, std::size_t n, std::size_t seed);

  // Accumulate the 'text' vector into language
  void merge(vector& language, const vector& text);

  // Compare 'text' with 'language':
  //   1  => perfect (best) match,
  //   -1 => worst match
  float score(const vector& language, const vector& text);

} // namespace language_vector

#endif // LANGUAGE_VECTOR_HPP
