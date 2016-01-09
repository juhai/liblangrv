#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
DATA=/tmp/langrv_data
python3 "${DIR}/extract_bible.py" -v ${DATA}
python3 "${DIR}/test_functional.py" -v -j2 --train 10 --test 10 --pretty ${DATA}
