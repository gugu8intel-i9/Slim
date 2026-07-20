#!/usr/bin/env bash
set -e

SLIM_REPO="https://github.com/gugu8intel-i9/Slim.git"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/share/slim}"
BIN_DIR="${BIN_DIR:-$HOME/.local/bin}"

# Always install/update the Slop toolchain to the latest version.
echo "[Slim] Installing/updating Slop ..."
curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slop/main/install.sh | bash

echo "[Slim] Cloning Slim into $INSTALL_DIR ..."
mkdir -p "$INSTALL_DIR"
if [ -d "$INSTALL_DIR/.git" ]; then
    git -C "$INSTALL_DIR" pull
else
    git clone "$SLIM_REPO" "$INSTALL_DIR"
fi

cd "$INSTALL_DIR"

echo "[Slim] Building ..."
./build.sh

echo "[Slim] Linking binary to $BIN_DIR/slim ..."
mkdir -p "$BIN_DIR"
ln -sf "$INSTALL_DIR/slim" "$BIN_DIR/slim"

echo "[Slim] Done. Make sure '$BIN_DIR' is in your PATH, then run: slim <file>"
