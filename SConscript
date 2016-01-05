# -*- Python -*-
import subprocess
from os import path

def build_so(env, base):
    """Helper to build shared object files."""
    return lambda src: env.SharedObject(path.join(base, path.basename(src.srcnode().path).replace('.cpp', '.os')), src)

Import('*')
env = env.Clone()

env['CC'] = 'clang'
env['CXX'] = 'clang++'
env.Append(CXXFLAGS=['-std=c++11', '-Wall', '-Wextra', '-Werror', '-O3', '-g', '-fPIC', '-mtune=native'],
           CPPDEFINES=[('FORTIFY_SOURCE', '2')])

# Core library
objs = map(build_so(env, ""), Glob('src/*.cpp'))

# Unit tests
env_test = env.Clone()
env_test.Append(CPPPATH=['#src', '#build/third-party/Catch/include'])
test = env_test.Program('test_language_vector', objs + map(build_so(env_test, "test"), Glob('src/test/*.cpp')))
pattern = ARGUMENTS.get("test", "")
env.AlwaysBuild(env.Alias('test', test, "%s %s" % (test[0].abspath, pattern)))

# Python wrapper (repl, functional tests)
py_include_path = subprocess.check_output(
    ["python3", "-c", "import distutils.sysconfig; print(distutils.sysconfig.get_python_inc())"]
).strip()
env_py = env.Clone()
env_py.Append(CPPPATH=['#src', py_include_path])
pylib = env_py.SharedLibrary(
    'py/language_vector',
    objs + map(build_so(env_py, "py"), Glob('src/py/*.cpp')),
    LIBPREFIX=''
)
env_py.PrependENVPath("PYTHONPATH", "build/core/py")
env_py['ENV']['PYTHONIOENCODING'] = 'utf8'
env_py.AlwaysBuild(env_py.Alias('python', pylib, 'python3'))
interpreter = 'python3' if int(ARGUMENTS.get('profile', '0')) == 0 else 'python3 -m yep --'
env_py.AlwaysBuild(env_py.Alias('ftest', pylib + Glob('#src/ftest/*.py'), interpreter + ' src/ftest/test.py'))
