#include "Python.h"
#include "language_vector.hpp"
#include <sstream>

namespace {

  // Some generic wrapping helpers

  template<class T>
  T* unwrap_object(PyObject* obj) {
    return reinterpret_cast<T*>(PyCapsule_GetPointer(obj, nullptr));
  }
  template<class T>
  void destroy_capsule(PyObject* obj) {
    delete unwrap_object<T>(obj);
  }
  template<class T>
  PyObject* wrap_object(T* obj) {
    return PyCapsule_New(obj, nullptr, destroy_capsule<T>);
  }

  // Wrapper functions

  PyObject* make_builder(PyObject* /*self*/, PyObject* args) {
    size_t order, n, seed;
    if (!PyArg_ParseTuple(args, "KKK", &order, &n, &seed)) {
      return nullptr;
    }
    return wrap_object(language_vector::make_builder(order, n, seed));
  }

  PyObject* build(PyObject* /*self*/, PyObject* args) {
    PyObject* pybuilder;
    const char* text;
    if (!PyArg_ParseTuple(args, "Os", &pybuilder, &text)) {
      return nullptr;
    }
    auto builder = unwrap_object<language_vector::builder>(pybuilder);
    return wrap_object((*builder)(text));
  }

  PyObject* save(PyObject* /*self*/, PyObject* args) {
    PyObject* pybuilder;
    PyObject* pylanguage;
    if (!PyArg_ParseTuple(args, "OO", &pybuilder, &pylanguage)) {
      return nullptr;
    }
    auto builder = unwrap_object<language_vector::builder>(pybuilder);
    auto language = unwrap_object<language_vector::vector>(pylanguage);
    std::ostringstream str;
    builder->save(*language, str);
    return Py_BuildValue("y", str.str().c_str());
  }

  PyObject* load(PyObject* /*self*/, PyObject* args) {
    PyObject* pybuilder;
    const char* bytes;
    if (!PyArg_ParseTuple(args, "Oy", &pybuilder, &bytes)) {
      return nullptr;
    }
    auto builder = unwrap_object<language_vector::builder>(pybuilder);
    std::istringstream str(bytes);
    return wrap_object(builder->load(str));
  }

  PyObject* merge(PyObject* /*self*/, PyObject* args) {
    PyObject* pylanguage;
    PyObject* pytext;
    if (!PyArg_ParseTuple(args, "OO", &pylanguage, &pytext)) {
      return nullptr;
    }
    auto language = unwrap_object<language_vector::vector>(pylanguage);
    auto text = unwrap_object<language_vector::vector>(pytext);
    language_vector::merge(*language, *text);
    return Py_BuildValue("");
  }

  PyObject* score(PyObject* /*self*/, PyObject* args) {
    PyObject* pylanguage;
    PyObject* pytext;
    if (!PyArg_ParseTuple(args, "OO", &pylanguage, &pytext)) {
      return nullptr;
    }
    auto language = unwrap_object<language_vector::vector>(pylanguage);
    auto text = unwrap_object<language_vector::vector>(pytext);
    auto result = language_vector::score(*language, *text);
    return Py_BuildValue("f", result);
  }

  // Module definition

  PyMethodDef LanguageVectorMethods[] = {
    { "make_builder", make_builder, METH_VARARGS,
      "Create a builder, which may be used to construct language vectors, and load them from a stream" },
    { "build", build, METH_VARARGS, "Build a language vector from a builder & a text string" },
    { "save", save, METH_VARARGS, "Save a language vector ``bytes = save(builder, vector)``" },
    { "load", load, METH_VARARGS, "Save a language vector ``vector = load(builder, bytes)``" },
    { "merge", merge, METH_VARARGS, "Merge two language vector" },
    { "score", score, METH_VARARGS, "Compare two language vectors" },
    { nullptr, nullptr, 0, nullptr }
  };

  struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT,
    "language_vector",
    "Simple, fast random indexed language vectors",
    -1,
    LanguageVectorMethods,
    nullptr,
    nullptr,
    nullptr,
    nullptr
  };

} // namespace (anonymous)

PyMODINIT_FUNC PyInit_langrv() {
  return PyModule_Create(&module);
}
