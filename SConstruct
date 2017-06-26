# -*- Python -*-
import os
env = Environment()

version = '0.1.1'

# Third party - unit testing framework
env.Command([File('#build/third-party/Catch/include/catch.hpp')], [],
            'rm -rf build/third-party/Catch; mkdir -p build/third-party && git clone https://github.com/philsquared/Catch.git build/third-party/Catch')

# Code
Export('env', 'version')
SConscript('SConscript', variant_dir='build/core', duplicate=0)

# Repl & functional tests
env_py = env.Clone()
env_py.PrependENVPath('PYTHONPATH', 'build/core/py')
env_py['ENV']['PYTHONIOENCODING'] = 'utf8'
pylib = File('#build/core/py/langrv.so')
interpreter = 'python3' if int(ARGUMENTS.get('profile', '0')) == 0 else 'python3 -m yep --'

# Interactive Python repl
env_py.AlwaysBuild(env_py.Alias('repl', [pylib], interpreter))

# Multilingual data - bible translations
test_data = env.Command([File('#build/data/English.txt')], File('#tools/extract_bible.py'),
                        'python3 ${SOURCE} -v build/data')
test_functional = File('#tools/test_functional.py')
test_languages = ['English', 'French', 'Spanish', 'Basque', 'German', 'Danish', 'Vietnamese']
test_threshold = 0.99
env_py.AlwaysBuild(env_py.Alias('ftest', [pylib, test_functional] + test_data,
                                '%s %s build/data -v -j2 --pretty --threshold %f --languages %s' % (
                                    interpreter,
                                    test_functional,
                                    test_threshold,
                                    ' '.join(test_languages)
                                )))

# Defaults - everything except installation
env.Default('.', 'test', 'ftest')
