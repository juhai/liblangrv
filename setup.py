from distutils.core import setup, Extension
from glob import glob

module = Extension('langrv',
                   sources = glob('src/*.cpp') + glob('src/py/*.cpp'),
                   extra_compile_args = ['-std=c++11'])

setup(name = 'langrv',
      version = '0.1.0',
      description = 'A simple implementation of Random Indexing For Language Detection.',
      ext_modules = [module])
