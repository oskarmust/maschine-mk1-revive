#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
clang -O0 -g -mmacosx-version-min=14.0 \
    -framework CoreFoundation -framework IOKit -framework CoreMIDI \
    tests/led_mapping_test.c mk1-usb/mk1_device.c mk1-bridge/mk1_server.c \
    -o build/led-mapping-test
build/led-mapping-test
