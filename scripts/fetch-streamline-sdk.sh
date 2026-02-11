#!/usr/bin/env bash
set -euo pipefail

OUT_DIR="${1:-/tmp/streamline-sdk}"
MODE="metadata"
if [[ "${2:-}" == "--download" || "${1:-}" == "--download" ]]; then
  MODE="download"
  if [[ "${1:-}" == "--download" ]]; then
    OUT_DIR="/tmp/streamline-sdk"
  fi
fi

readarray -t META < <(python - <<'PY'
import json, urllib.request
url='https://api.github.com/repos/NVIDIA-RTX/Streamline/releases/latest'
with urllib.request.urlopen(url, timeout=30) as r:
    data=json.load(r)
asset = None
for a in data.get('assets',[]):
    if a.get('name','').endswith('.zip') and 'streamline-sdk' in a.get('name',''):
        asset = a
        break
if not asset:
    raise SystemExit('No streamline-sdk zip asset found in latest release')
print(data.get('tag_name',''))
print(asset['name'])
print(asset['browser_download_url'])
PY
)

TAG="${META[0]}"
NAME="${META[1]}"
URL="${META[2]}"

echo "$TAG"
echo "$NAME"
echo "$URL"

if [[ "$MODE" == "download" ]]; then
  mkdir -p "$OUT_DIR"
  ZIP_PATH="$OUT_DIR/$NAME"

  echo "Downloading $NAME ($TAG) -> $ZIP_PATH"
  curl -fL -o "$ZIP_PATH" "$URL"
  sha256sum "$ZIP_PATH"
  echo "Top entries in archive:"
  unzip -l "$ZIP_PATH" | sed -n '1,40p'
fi
