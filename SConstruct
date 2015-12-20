# -*- Python -*-
import os
env = Environment()

env.Command([File('#build/third-party/Catch/include/catch.hpp')], [],
            'rm -rf build/third-party/Catch; mkdir -p build/third-party && git clone https://github.com/philsquared/Catch.git build/third-party/Catch')

Export('env')
SConscript('SConscript', variant_dir='build/core', duplicate=0)
