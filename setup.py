from setuptools import setup, Extension
from glob import glob

module = Extension('langrv',
                   sources=glob('src/*.cpp') + glob('src/py/*.cpp'),
                   extra_compile_args=['-std=c++11'],
                   include_dirs=['src/', ])

setup(name='langrv',
      version='0.1.3',
      description='A simple implementation of Random Indexing For Language Detection.',  # noqa
      ext_modules=[module])
