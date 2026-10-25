#!/bin/sh
set -eu

SCRIPT_DIR="$(CDPATH= cd -- "$(dirname "$0")" && pwd)"
REPO_ROOT="$(CDPATH= cd -- "${SCRIPT_DIR}/.." && pwd)"

VERSION="${VERSION:-0.4.0-beta.1}"
CONFIGURATION="${CONFIGURATION:-Release}"
OUT_DIR="${REPO_ROOT}/dist"
ZIP_DEST="${OUT_DIR}/maschine-mk1-revive-v${VERSION}-macos-arm64.zip"

STAGE_DIR="$(mktemp -d "${TMPDIR:-/tmp}/mk1-bridge-release-assets.XXXXXX")"
trap 'rm -rf "${STAGE_DIR}"' EXIT INT TERM

# Command Line Tools are enough; full Xcode is optional.
CONFIGURATION="${CONFIGURATION}" sh "${REPO_ROOT}/scripts/build-bridge.sh"

CONFIGURATION="${CONFIGURATION}" bash "${REPO_ROOT}/mk1-menubar/build.sh"

# Zip just the app bundle — drag to /Applications and launch to install
cp -r "${REPO_ROOT}/build/MK1 Revive.app" "${STAGE_DIR}/MK1 Revive.app"
xattr -cr "${STAGE_DIR}/MK1 Revive.app"

mkdir -p "${OUT_DIR}"
rm -f "${ZIP_DEST}"
(
    cd "${STAGE_DIR}"
    zip -qry "${ZIP_DEST}" "MK1 Revive.app"
)

printf 'Built: %s\n' "${ZIP_DEST}"
