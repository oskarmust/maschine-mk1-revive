#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
clang -O0 -g -framework CoreFoundation -framework IOKit \
    tests/pad_release_test.c -o build/pad-release-test
build/pad-release-test
