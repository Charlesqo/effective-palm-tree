#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:-native/build}"
cmake -S native -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release
cmake --build "$BUILD_DIR" -j
ctest --test-dir "$BUILD_DIR" --output-on-failure
"$BUILD_DIR/snake_native"
