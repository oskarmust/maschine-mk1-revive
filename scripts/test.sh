#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build/tests
for name in idle_pong idle_games idle_pad_layout idle_game_leds studio_games; do
 clang -O1 -fsanitize=undefined "tests/test_${name}.c" -lm -o "build/tests/$name"
 "build/tests/$name"
done
clang -O1 -framework CoreFoundation tests/test_input_profile.c -o build/tests/input_profile
build/tests/input_profile
sh tests/run-pad-release-test.sh
sh tests/run-led-mapping-test.sh
python3 tests/test_display_updates.py
python3 tests/test_freedoom_keys.py
python3 tests/test_freedoom_runtime.py
