#!/usr/bin/env bash

if [[ ! -d build ]]; then
    mkdir build
fi

/opt/homebrew/opt/llvm/bin/clang++ -std=c++17 -g -fsanitize=address -fno-omit-frame-pointer \
    pbb/source/*.c \
    pbb/test/*.cpp \
    -Ipbb/include \
    -lfmt -lgtest -lgtest_main -lpthread \
    -o build/pbb_tests

if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

cd build
export ASAN_OPTIONS=detect_leaks=1
./pbb_tests
