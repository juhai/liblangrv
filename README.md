# liblangrv

A simple implementation of _Language Detection Using Random Indexing_ (Joshi et. al. 2015),
in C++ with optional Python3 bindings.

## Building & testing in a container

    docker build --rm -t langrv .
    docker run -it --rm langrv /opt/langrv/tools/run_tests.sh
    docker run -it --rm -w /opt/langrv langrv scons

# Building & testing locally

First, look in [Dockerfile](Dockerfile) for build-time dependencies - you'll need
(at least most of) these. Then run our all-in-one build & tests...

    scons

## References

 - Language Detection Using Random Indexing (Joshi et. al. 2015) [[pdf](http://arxiv.org/pdf/1412.7026.pdf)]
