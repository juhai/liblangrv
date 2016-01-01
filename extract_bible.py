import argparse, tempfile, subprocess, gzip, logging, re, os, codecs
import xml.etree.ElementTree as ET
import os.path as path

parser = argparse.ArgumentParser(
    description="""Download & extract out verses from XML-formatted bible(s) from http://homepages.inf.ed.ac.uk/s0787820/bible/."""
)
parser.add_argument("dest", metavar="DIR", help="Destination to place downloaded files")
parser.add_argument("--remote",
                    default="http://homepages.inf.ed.ac.uk/s0787820/bible/XML_Bibles.tar.gz",
                    help="Source to fetch the data from")
parser.add_argument("-d", "--dry-run", action="store_true")
parser.add_argument("-v", "--verbose", action="count", default=0)
args = parser.parse_args()

logging.basicConfig(
    format = '%(levelname)s\t%(message)s',
    level = [logging.WARNING, logging.INFO][args.verbose]
)

def sh(*cmd):
    """Run a shell command, possibly eliding it in dry-run mode."""
    if args.dry_run:
        print("$ %s" % ' '.join(cmd))
    else:
        logging.info("$ %s", ' '.join(cmd))
        if subprocess.call(cmd) != 0:
            exit(1)

# Prepare dirs
tmp = tempfile.mkdtemp()
if not path.isdir(args.dest): os.makedirs(args.dest)

# Download & extract data
tmp_archive = path.join(tmp, "archive.tar.gz")
sh("wget", "-qO", tmp_archive, args.remote)
sh("tar", "-C", tmp, "-xf", tmp_archive)

# Extract the text
pattern = re.compile("""^(.+)\.xml\.gz$""")
for f in os.listdir(tmp):
    m = pattern.match(f)
    if m == None:
        logging.info("Skipping %s", f)
    else:
        logging.info("Converting %s", f)
        name = m.group(1)
        with gzip.open(path.join(tmp, f)) as input_file:
            with codecs.open(path.join(args.dest, "%s.txt" % name), "w", "utf8") as output_file:
                for x in ET.fromstring(input_file.read().decode('utf8')).iter("seg"):
                    if x.text != None:
                        output_file.write(x.text.strip() + "\n")

# Clean up
sh("rm", "-r", tmp)
