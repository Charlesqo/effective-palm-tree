#!/usr/bin/env bash
set -euo pipefail

echo "[1/3] unityhub path"
which unityhub

echo "[2/3] unityhub headless help"
xvfb-run -a unityhub -- --headless help >/tmp/unityhub-help.txt 2>/tmp/unityhub-help.err || true
head -n 30 /tmp/unityhub-help.txt || true

echo "[3/3] unity releases (json)"
xvfb-run -a unityhub -- --headless editors -r -j >/tmp/unityhub-editors.json 2>/tmp/unityhub-editors.err || true
head -n 20 /tmp/unityhub-editors.json || true

echo "stderr samples:"
head -n 20 /tmp/unityhub-help.err || true
head -n 20 /tmp/unityhub-editors.err || true
