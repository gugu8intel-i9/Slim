#!/usr/bin/env bash
set -e

# Build script for Slim - a high-performance terminal editor written in Slop.
# Requires: python3, gcc (or clang), and the Slop runtime headers.

SLOP_REPO="${SLOP_REPO:-/home/user/slop_repo}"
SLOP_BOOT="$SLOP_REPO/slop_boot.py"
SLOP_INCLUDE="$SLOP_REPO"

if [ ! -f "$SLOP_BOOT" ]; then
    echo "Error: Slop boot transpiler not found at $SLOP_BOOT"
    echo "Set SLOP_REPO to the path of your Slop clone."
    exit 1
fi

echo "[Slim] Transpiling slim.slop -> slim.c ..."
python3 "$SLOP_BOOT" slim.slop slim.c

echo "[Slim] Compiling slim.c -> slim ..."
CC="${CC:-gcc}"
$CC -O3 -ffast-math -flto -I"$SLOP_INCLUDE" slim.c -o slim -lm

echo "[Slim] Build complete: ./slim <file>"
