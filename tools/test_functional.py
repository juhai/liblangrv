import langrv, codecs, time, copy, logging, json, re, glob, os, concurrent.futures
import itertools as it

def _for_lines(path, start, count, action):
    """Iterate through the file at 'path', logging timing information, and performing 'action' for each line."""
    nchars = 0
    nlines = 0
    start_time = time.time()
    with codecs.open(path, "r", "utf8") as f:
        for line in it.islice(f, start, start + count):
            line = line.rstrip('\n')
            action(line)
            nchars += len(line)
            nlines += 1
    elapsed_time = time.time() - start_time
    logging.info("%s: %d lines, %d chars at %.1e chars/s",
                 path, nlines, nchars, nchars / elapsed_time)

def _build_language(builder, language, path, start, count):
    """Build a language vector from a range of lines at the given path."""
    v = langrv.build(builder, "")
    def process_line(line):
        langrv.merge(v, langrv.build(builder, line))
    _for_lines(path, start, count, process_line)
    return v

def _classify(builder, language_vectors, text):
    """Classify the given text (single string) under the given map of language vectors."""
    text_vector = langrv.build(builder, text)
    return max(language_vectors.keys(),
               key=lambda language: langrv.score(language_vectors[language], text_vector))

def _classify_lines(builder, actual_language, language_vectors, path, start, count):
    """Classify each line in a range from the given path under the given map of language vectors."""
    class_counts = {language: 0 for language in language_vectors.keys()}
    def process_line(line):
        class_ = _classify(builder, language_vectors, line)
        if class_ != actual_language:
            logging.debug("FAIL %s (%s -> %s)", line, actual_language, class_)
        class_counts[class_] += 1
    _for_lines(path, start, count, process_line)
    return class_counts

DEFAULTS = {
    'train': 1000,
    'test': 1000,
    'order': 4,
    'threads': 1,
    'dimension': 10000,
    'languages': ['all'],
    'seed': 42,
}

def evaluate(data_path, options):
    """Evaluate langrv over the dataset at data_path."""
    opts = copy.deepcopy(DEFAULTS)
    opts.update(options)

    logging.info("1. finding set of languages to test")
    if 'all' in options['languages']:
        pattern = re.compile("(.*)\.")
        languages = {}
        for name in os.listdir(data_path):
            m = pattern.match(name)
            if m:
                languages[m.group(1)] = os.path.join(data_path, name)
    else:
        languages = {}
        for language in options['languages']:
            matches = glob.glob(os.path.join(data_path, language + '*'))
            if matches:
                languages[language] = matches[0]
            else:
                raise FileNotFoundError('Could not find a data file for language "%s" in %s' %
                                        (language, data_path))

    logging.info("2. building language vectors for each language")
    builder = langrv.make_builder(opts['order'], opts['dimension'], opts['seed'])

    def pmap_items(fn, m):
        with concurrent.futures.ThreadPoolExecutor(max_workers=opts['threads']) as executor:
            return dict(zip(m.keys(), executor.map(lambda i: fn(i[0], i[1]), m.items())))

    language_vectors = pmap_items(lambda language, path: _build_language(builder, language, path, 0, opts['train']), languages)

    logging.info("3. testing languages")
    return pmap_items(lambda language, path: _classify_lines(builder, language, language_vectors, path, opts['train'], opts['test']), languages)

def accuracy(result):
    """Return the overall accuracy of results returned from ``evaluate``."""
    correct = sum(predictions[actual_language] for actual_language, predictions in result.items())
    total = sum(count for predictions in result.values() for count in predictions.values())
    return correct / total

# Main entry point
if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(
        description="""Run a functional (quality) test for language classification using random vectors."""
    )
    parser.add_argument("data", metavar="DIR", help="directory to scan for language data files")

    parser.add_argument("-n", "--train", metavar="N", type=int,
                        help="number of lines of text to use for training")
    parser.add_argument("-t", "--test", metavar="N", type=int,
                        help="number of lines of text to use for testing")
    parser.add_argument("-o", "--order", metavar="N", type=int,
                        help="the order of ngrams to sequence")
    parser.add_argument("-d", "--dimension", metavar="N", type=int,
                        help="the number of elements to use in the ngram vector")
    parser.add_argument("-l", "--languages", metavar="LANG", nargs='+', default='all',
                        help="languages to train & validate, or 'all' for all languages in the dataset")
    parser.add_argument("-s", "--seed", metavar="N", type=int,
                        help="randomization seed to use")
    parser.add_argument("-j", "--threads", metavar="N", type=int,
                        help="generate pretty-printed human-readable output")

    parser.add_argument("-x", "--threshold", metavar="FRACTION", type=float, default=0.0,
                        help="""set a threshold - exit with failure if the overall accuracy fails
                        to exceed the threshold""")
    parser.add_argument("--pretty", action='store_true',
                        help="generate pretty-printed human-readable output")
    parser.add_argument("-v", "--verbose", action="count", default=0)

    parser.set_defaults(**DEFAULTS)
    args = parser.parse_args()

    logging.basicConfig(
        format = '%(levelname)s\t%(message)s',
        level = [logging.WARNING, logging.INFO, logging.DEBUG][args.verbose]
    )

    result = evaluate(args.data, vars(args))
    if args.pretty:
        overall_title = ":: Overall :"
        nother = 3
        just = 2 + max(len(title) for title in [overall_title] + list(result.keys()))
        print("-" * 80)
        for actual_language, predictions in sorted(result.items()):
            npredictions = sum(predictions.values())
            acc = predictions[actual_language] / npredictions
            others = (language for language in result.keys() if language != actual_language and 0 < predictions[language])
            top_others = sorted(others, key=lambda language: predictions[language], reverse=True)[:nother]
            others_accs = ((language, predictions[language] / npredictions) for language in top_others)
            others_desc = ', '.join('%s: %.1f%%' % (language, 100 * predictions[language] / npredictions)
                                    for language in top_others)
            print("%s: %.1f%%   (%s)" % (actual_language.rjust(just, ' '), 100 * acc, others_desc))
        print("-" * 80)
        print("%s: %.2f%%" % (overall_title.rjust(just, ' '), 100 * accuracy(result)))
    else:
        print(json.dumps(result))

    exit(1 if accuracy(result) < args.threshold else 0)
