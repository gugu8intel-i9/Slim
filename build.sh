#!/usr/bin/env bash
set -e

# Build script for Slim - a high-performance terminal editor written in Slop.
# Uses the Python bootstrap transpiler (slop_boot.py) from the Slop repo.
# This is more portable than the native slop-compiler and works on macOS
# where the native ELF backend may not build.

SLOP_REPO_URL="https://github.com/gugu8intel-i9/Slop.git"
SLOP_REPO="${SLOP_REPO:-$HOME/.local/share/slop}"
SLOP_BOOT="$SLOP_REPO/slop_boot.py"
SLOP_INCLUDE="$SLOP_REPO"

# Clone or update Slop if we don't have the bootstrap transpiler.
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
    echo "Error: Slop bootstrap transpiler not found at $SLOP_BOOT"
    exit 1
fi

if [ ! -f "$SLOP_INCLUDE/slop_rt.h" ]; then
    echo "Error: Slop runtime headers not found at $SLOP_INCLUDE/slop_rt.h"
    exit 1
fi

# macOS needs _DARWIN_C_SOURCE so the Slop runtime can see snprintf,
# _SC_NPROCESSORS_ONLN, and other platform symbols.
EXTRA_CFLAGS=""
if [ "$(uname -s)" = "Darwin" ]; then
    EXTRA_CFLAGS="-D_DARWIN_C_SOURCE"
fi

rm -f slim.c slim

echo "[Slim] Transpiling slim.slop -> slim.c ..."
python3 "$SLOP_BOOT" slim.slop slim.c

if [ ! -f "slim.c" ]; then
    echo "Error: slop_boot.py did not produce slim.c"
    exit 1
fi

echo "[Slim] Compiling slim.c -> slim ..."
CC="${CC:-$(command -v gcc || command -v clang || true)}"
if [ -z "$CC" ]; then
    echo "Error: No C compiler found. Install gcc or clang."
    exit 1
fi
"$CC" -O3 -std=gnu11 -ffast-math -flto $EXTRA_CFLAGS -I"$SLOP_INCLUDE" slim.c -o slim -lm

echo "[Slim] Build complete: ./slim <file>"
