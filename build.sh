#!/usr/bin/env bash
set -e

# Build script for Slim - a high-performance terminal editor written in Slop.
# Uses the official Slop compiler toolchain.

SLOP_BIN="${SLOP_BIN:-$HOME/.slop/bin}"
SLOP_INCLUDE="${SLOP_INCLUDE:-$HOME/.slop/include}"
SLOP_COMPILER="$SLOP_BIN/slop-compiler"

if [ ! -f "$SLOP_COMPILER" ]; then
    echo "Error: Slop compiler not found at $SLOP_COMPILER"
    echo "Install Slop first:"
    echo "  curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slop/main/install.sh | bash"
    exit 1
fi

if [ ! -f "$SLOP_INCLUDE/slop_rt.h" ]; then
    echo "Error: Slop runtime headers not found at $SLOP_INCLUDE/slop_rt.h"
    echo "Install Slop first:"
    echo "  curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slop/main/install.sh | bash"
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
"$SLOP_COMPILER" slim.slop slim.c

if [ ! -f "slim.c" ]; then
    echo "Error: slop-compiler did not produce slim.c"
    echo "Try updating Slop:"
    echo "  curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slop/main/install.sh | bash"
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
