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
shared_object = env.SharedLibrary('langrv', objs, SHLIBVERSION=version)

# Install target for core
install_headers = env.Install('/usr/local/include', Glob('src/*.hpp'))
install_so = env.Install('/usr/local/lib', shared_object)
env.Alias('install', install_headers + install_so)

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
env_py.SharedLibrary(
    'py/langrv',
    objs + map(build_so(env_py, "py"), Glob('src/py/*.cpp')),
    LIBPREFIX=''
)
