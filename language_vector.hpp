#ifndef LANGUAGE_VECTOR_HPP
#define LANGUAGE_VECTOR_HPP

#include <string>

namespace language_vector {

  // An opaque type - use the following methods to construct & fiddle with it.
  struct vector_impl;
  struct vector {
    explicit vector(vector_impl*);
    vector(const vector_impl&) = delete;
    vector& operator=(const vector_impl&) = delete;
    ~vector();
    vector_impl* impl;
  };

  // Construct a random vector representation of the given text
  vector* build(const std::string& text, std::size_t order, std::size_t n);

  // Accumulate the 'text' vector into language
  void merge(vector& language, const vector& text);

  // Compare 'text' with 'language':
  //   1  => perfect (best) match,
  //   -1 => worst match
  float score(const vector& language, const vector& text);

} // namespace language_vector

#endif // LANGUAGE_VECTOR_HPP
