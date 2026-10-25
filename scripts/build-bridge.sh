#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
CONFIGURATION=${CONFIGURATION:-Release}
mkdir -p "${ROOT}/build/${CONFIGURATION}"
clang -fblocks -O2 -arch arm64 -mmacosx-version-min=14.0 \
 -framework CoreFoundation -framework IOKit -framework CoreMIDI \
 "${ROOT}/mk1-bridge/main.c" "${ROOT}/mk1-bridge/mk1_server.c" \
 "${ROOT}/mk1-usb/mk1_device.c" "${ROOT}/mk1-ipc/mk1_ipc.c" \
 -o "${ROOT}/build/${CONFIGURATION}/mk1-bridge"
printf 'Built bridge: %s\n' "${ROOT}/build/${CONFIGURATION}/mk1-bridge"
