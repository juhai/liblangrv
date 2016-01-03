import language_vector, collections, codecs
import itertools as it
import os.path as path

# Configuration

builder = language_vector.make_builder(3, 10000, 42)
data_dir = "build/data"
languages = ["English", "French", "German", ]#"Italian", "Latin", "Vietnamese"]
train_lines = 200 # 20000
valid_lines = 100 # 10000

def open_language(language):
    return codecs.open(path.join(data_dir, "%s.txt" % language), "r", "utf8")

def build_language(language):
    v = language_vector.build(builder, "")
    with open_language(language) as f:
        for line in it.islice(f, 0, train_lines):
            language_vector.merge(v, language_vector.build(builder, line.rstrip('\n')))
    return v

# 1. build language vectors from each language
language_vectors = collections.OrderedDict((language, build_language(language)) for language in languages)

def classify(text):
    text_vector = language_vector.build(builder, text)
    return max(language_vectors.keys(),
               key=lambda language: language_vector.score(language_vectors[language], text_vector))

# 2. classify text vectors - verse by verse
for language in languages:
    with open_language(language) as f:
        for line in it.islice(f, train_lines, train_lines + valid_lines):
            line = line.rstrip('\n')
            class_language = classify(line)
            if class_language == language:
                print("SUCCESS %s  (%s)" % (line, language))
            else:
                print("   FAIL %s  (%s -> %s)" % (line, language, class_language))
