#!/usr/bin/env bash
set -euo pipefail

echo "[A] APT search"
apt-cache search unreal | head -n 20 || true
apt-cache search ue5 | head -n 20 || true

echo "[B] snap availability"
if command -v snap >/dev/null 2>&1; then
  snap find unreal | head -n 20 || true
else
  echo "snap: not installed"
fi

echo "[C] flatpak availability"
if command -v flatpak >/dev/null 2>&1; then
  flatpak search unreal | head -n 20 || true
else
  echo "flatpak: not installed"
fi

echo "[D] docker availability"
if command -v docker >/dev/null 2>&1; then
  docker --version
else
  echo "docker: not installed"
fi

echo "[E] EpicGames/UnrealEngine repo access test"
# UnrealEngine 官方源码仓库需要 Epic/GitHub 账户权限。
# 在无交互 token 环境下，通常会因认证失败而无法访问。
if git ls-remote https://github.com/EpicGames/UnrealEngine.git >/tmp/ue-lsremote.txt 2>/tmp/ue-lsremote.err; then
  echo "repo accessible"
  head -n 20 /tmp/ue-lsremote.txt
else
  echo "repo inaccessible without auth (expected in this environment)"
  head -n 20 /tmp/ue-lsremote.err || true
fi
