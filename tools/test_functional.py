import langrv, argparse, collections, codecs, time
import itertools as it
import os.path as path

# Configuration

parser = argparse.ArgumentParser(
    description="""Run a functional (quality) test for language classification using random vectors."""
)
parser.add_argument("data", metavar="DIR", help="Directory to scan for language data files")
args = parser.parse_args()

builder = langrv.make_builder(4, 10000, 42)
languages = ["English", "French", "German", "Italian", "Latin", "Vietnamese"]
train_lines = 1000
valid_lines = 1000
verbosity = 0

def open_language(language):
    return codecs.open(path.join(args.data, "%s.txt" % language), "r", "utf8")

def for_lines(language, description, start, count, action):
    nchars = 0
    start_time = time.time()
    with open_language(language) as f:
        for line in it.islice(f, start, start + count):
            line = line.rstrip('\n')
            action(line)
            nchars += len(line)
    elapsed_time = time.time() - start_time
    print("\t%s.%s: %d chars at %.1e chars/s" % (description, language, nchars, nchars / elapsed_time))

def build_language(language):
    v = langrv.build(builder, "")
    def process_line(line):
        langrv.merge(v, langrv.build(builder, line))
    for_lines(language, "build", 0, train_lines, process_line)
    return v

# 1. build language vectors from each language
language_vectors = collections.OrderedDict((language, build_language(language)) for language in languages)

def classify(text):
    """Classify the given text (line) under the set of loaded language vectors."""
    text_vector = langrv.build(builder, text)
    return max(language_vectors.keys(),
               key=lambda language: langrv.score(language_vectors[language], text_vector))

# 2. classify text vectors - verse by verse
for language in languages:
    n = 0
    nsuccess = 0
    def process_line(line):
        global n
        global nsuccess
        n += 1
        class_language = classify(line)
        success = (language == class_language)
        nsuccess += success
        if 2 <= verbosity or (1 <= verbosity and not success):
            print("%s %s  (%s -> %s)" % (
                "SUCCESS" if success else "   FAIL", line, language, class_language)
            )
    for_lines(language, "classify", train_lines, valid_lines, process_line)
    print("%s accuracy: %.1f %%" % (language, 100 * nsuccess / float(n)))