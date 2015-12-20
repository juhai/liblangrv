# -*- Python -*-
Import('*')
env = env.Clone()

env.Append(CXXFLAGS=['-std=c++11', '-Wall', '-Wextra', '-Werror', '-O3', '-g'])
lib = env.StaticLibrary('language_vector', Glob('src/*.cpp'))

env_test = env.Clone()
env_test.Append(CPPPATH=['#src', '#build/third-party/Catch/include'])
env_test.Append(LIBS=lib)
test = env_test.Program('test_language_vector', lib + Glob('src/test/*.cpp'))

pattern = ARGUMENTS.get("test", "")
env.AlwaysBuild(env.Alias('test', test, "%s %s" % (test[0].abspath, pattern)))
