CC="clang++ -std=c++11 -Wall -Wextra -Werror -O3 -g"

set -e
set -o xtrace

mkdir -p build
${CC} -c language_vector.cpp -o build/language_vector.o
${CC} -c test_language_vector.cpp -o build/test_language_vector.o
${CC} build/*.o -o build/test_language_vector
