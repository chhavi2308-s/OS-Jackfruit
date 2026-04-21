#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

ok() { echo "[OK] $1"; }
fail() { echo "[FAIL] $1" >&2; exit 1; }

echo "== Supervised Runtime Project Preflight =="

if [[ -r /etc/os-release ]]; then
    source /etc/os-release
    [[ "${ID:-}" == "ubuntu" ]] || fail "Use Ubuntu 22.04/24.04 VM."
else
    fail "Cannot read /etc/os-release."
fi

if grep -qi microsoft /proc/version 2>/dev/null; then
    fail "WSL detected. This project does not support WSL."
fi

KBUILD_DIR="/lib/modules/$(uname -r)/build"
[[ -d "$KBUILD_DIR" ]] || fail "Kernel headers missing."

echo "Building..."
if make all >/dev/null 2>&1; then
    ok "Build succeeded."
else
    fail "Build failed."
fi

[[ "$(id -u)" -eq 0 ]] || fail "Run as root (sudo ./environment-check.sh)"

insmod ./monitor.ko && ok "insmod succeeded."
[[ -e /dev/container_monitor ]] && ok "/dev/container_monitor exists."
rmmod monitor && ok "rmmod succeeded."

echo "Preflight passed."


