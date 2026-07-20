#!/usr/bin/env bash
set -e

SLIM_REPO="https://github.com/gugu8intel-i9/Slim.git"
SLOP_REPO_URL="https://github.com/gugu8intel-i9/Slop.git"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/share/slim}"
SLOP_DIR="${SLOP_DIR:-$HOME/.local/share/slop}"
BIN_DIR="${BIN_DIR:-$HOME/.local/bin}"

# Ensure the Slop runtime/transpiler is available. We use the Python bootstrap
# (slop_boot.py) rather than the full native Slop installer because the native
# toolchain's ELF backend does not build on macOS.
if [ ! -f "$SLOP_DIR/slop_boot.py" ]; then
    echo "[Slim] Cloning Slop into $SLOP_DIR ..."
    mkdir -p "$SLOP_DIR"
    git clone "$SLOP_REPO_URL" "$SLOP_DIR"
else
    echo "[Slim] Updating Slop at $SLOP_DIR ..."
    git -C "$SLOP_DIR" pull
fi

echo "[Slim] Cloning Slim into $INSTALL_DIR ..."
mkdir -p "$INSTALL_DIR"
if [ -d "$INSTALL_DIR/.git" ]; then
    git -C "$INSTALL_DIR" pull
else
    git clone "$SLIM_REPO" "$INSTALL_DIR"
fi

cd "$INSTALL_DIR"

echo "[Slim] Building ..."
SLOP_REPO="$SLOP_DIR" ./build.sh

echo "[Slim] Linking binary to $BIN_DIR/slim ..."
mkdir -p "$BIN_DIR"
ln -sf "$INSTALL_DIR/slim" "$BIN_DIR/slim"

echo "[Slim] Done. Make sure '$BIN_DIR' is in your PATH, then run: slim <file>"
