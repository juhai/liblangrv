# -*- Python -*-
import os
env = Environment()

# Third party - unit testing framework
env.Command([File('#build/third-party/Catch/include/catch.hpp')], [],
            'rm -rf build/third-party/Catch; mkdir -p build/third-party && git clone https://github.com/philsquared/Catch.git build/third-party/Catch')

# Data - bible translations
env.Command([File("#build/data/English.txt")], [],
            "python3 extract_bible.py -v build/data")

# Code
Export('env')
SConscript('SConscript', variant_dir='build/core', duplicate=0)
