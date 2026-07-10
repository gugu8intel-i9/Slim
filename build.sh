#!/usr/bin/env bash
set -e

# Build script for Slim - a high-performance terminal editor written in Slop.
# Requires: python3, gcc or clang, and the Slop runtime headers.

SLOP_REPO_URL="https://github.com/gugu8intel-i9/Slop.git"
SLOP_REPO="${SLOP_REPO:-$HOME/.local/share/slop}"
SLOP_BOOT="$SLOP_REPO/slop_boot.py"
SLOP_INCLUDE="$SLOP_REPO"

# If Slop is not present, clone it automatically.
if [ ! -f "$SLOP_BOOT" ]; then
    if [ -d "$SLOP_REPO/.git" ]; then
        echo "[Slim] Updating Slop runtime at $SLOP_REPO ..."
        git -C "$SLOP_REPO" pull
    else
        echo "[Slim] Slop runtime not found; cloning into $SLOP_REPO ..."
        mkdir -p "$SLOP_REPO"
        git clone "$SLOP_REPO_URL" "$SLOP_REPO"
    fi
fi

if [ ! -f "$SLOP_BOOT" ]; then
    echo "Error: Slop boot transpiler not found at $SLOP_BOOT"
    exit 1
fi

echo "[Slim] Transpiling slim.slop -> slim.c ..."
python3 "$SLOP_BOOT" slim.slop slim.c

echo "[Slim] Compiling slim.c -> slim ..."
CC="${CC:-$(command -v gcc || command -v clang || true)}"
if [ -z "$CC" ]; then
    echo "Error: No C compiler found. Install gcc or clang."
    exit 1
fi
$CC -O3 -ffast-math -flto -I"$SLOP_INCLUDE" slim.c -o slim -lm

echo "[Slim] Build complete: ./slim <file>"
