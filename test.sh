#!/usr/bin/env bash

if [[ ! -d build ]]; then
    mkdir build
fi

CXX="g++"
SANITIZER_FLAGS=""

if [[ "$ASAN_OPTIONS" == *"detect_leaks=1"* ]]; then
    SANITIZER_FLAGS="-fsanitize=address -fno-omit-frame-pointer"
    CXX="clang++"
fi

echo "Compiling tests with: $CXX, Sanitizer Flags: $SANITIZER_FLAGS"

$CXX -std=c++17 -g \
    pbb/source/*.c \
    pbb/test/*.cpp \
    -Ipbb/include \
    -lfmt -lgtest -lgtest_main -lpthread \
    $SANITIZER_FLAGS \
    -o build/pbb_tests

if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

echo "Executing tests..."

cd build
./pbb_tests
