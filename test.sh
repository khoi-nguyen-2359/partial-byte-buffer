#!/usr/bin/env bash

TEST_FILES="pbb/test/*.cpp"
CAPACITY_GROWTH_MODE=0

while getopts "t:m:s:" opt; do
    case "$opt" in
        t) TEST_FILES="$OPTARG" ;;
        m) CAPACITY_GROWTH_MODE="$OPTARG" ;;
        *) echo "Usage: $0 [-t test_files] [-m mode]"; exit 1 ;;
    esac
done

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
echo "Test files: $TEST_FILES"
echo "Capacity Growth Mode: $CAPACITY_GROWTH_MODE"

$CXX -std=c++17 -g \
    pbb/source/*.c \
    $TEST_FILES \
    -Ipbb/include \
    -lfmt -lgtest -lgtest_main -lpthread \
    $SANITIZER_FLAGS \
    -DCAPACITY_GROWTH_MODE=$CAPACITY_GROWTH_MODE \
    -o build/pbb_tests

if [ $? -ne 0 ]; then
    echo "Compilation failed. Exiting."
    exit 1
fi

echo "Executing tests..."

cd build
./pbb_tests
