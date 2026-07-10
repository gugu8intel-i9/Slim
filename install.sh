#!/usr/bin/env bash
set -e

SLIM_REPO="https://github.com/gugu8intel-i9/Slim.git"
SLOP_REPO_URL="https://github.com/gugu8intel-i9/Slop.git"
INSTALL_DIR="${INSTALL_DIR:-$HOME/.local/share/slim}"
SLOP_DIR="${SLOP_DIR:-$HOME/.local/share/slop}"
BIN_DIR="${BIN_DIR:-$HOME/.local/bin}"

echo "[Slim] Cloning Slim into $INSTALL_DIR ..."
mkdir -p "$INSTALL_DIR"
if [ -d "$INSTALL_DIR/.git" ]; then
    git -C "$INSTALL_DIR" pull
else
    git clone "$SLIM_REPO" "$INSTALL_DIR"
fi

cd "$INSTALL_DIR"

# Point build.sh at a sensible Slop location if the user hasn't set one.
if [ -z "$SLOP_REPO" ]; then
    export SLOP_REPO="$SLOP_DIR"
fi

# If Slop is missing, clone it now so the build succeeds.
if [ ! -f "$SLOP_REPO/slop_boot.py" ]; then
    if [ -d "$SLOP_REPO/.git" ]; then
        echo "[Slim] Updating Slop at $SLOP_REPO ..."
        git -C "$SLOP_REPO" pull
    else
        echo "[Slim] Slop not found; cloning into $SLOP_REPO ..."
        mkdir -p "$SLOP_REPO"
        git clone "$SLOP_REPO_URL" "$SLOP_REPO"
    fi
fi

echo "[Slim] Building with SLOP_REPO=$SLOP_REPO ..."
./build.sh

echo "[Slim] Linking binary to $BIN_DIR/slim ..."
mkdir -p "$BIN_DIR"
ln -sf "$INSTALL_DIR/slim" "$BIN_DIR/slim"

echo "[Slim] Done. Make sure '$BIN_DIR' is in your PATH, then run: slim <file>"
