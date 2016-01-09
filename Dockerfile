FROM ubuntu:14.04

# 1. System/building dependencies
RUN apt-get -q update        \
    && apt-get install -qy   \
            clang            \
            git              \
            google-perftools \
            python3          \
            python3-pip      \
            scons            \
            wget             \
    && apt-get -q clean      \
    && pip3 install yep

# 2. Add files
ADD . /opt/langrv

# 3. Install separately as a C++ & Python library
RUN cd /opt/langrv              \
    && scons install            \
    && python3 setup.py install \
    && rm -r build
