# Slim

A high-performance, lightweight terminal text editor written in
[Slop](https://github.com/gugu8intel-i9/Slop).

Slim is a Vim-inspired modal editor that stays small, fast, and dependency-free.
It uses the official Slop compiler toolchain to transpile to native C.

## Install

### One-line curl install

```bash
curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slim/main/install.sh | bash
```

This will:

1. Install or update the Slop compiler toolchain into `~/.slop`.
2. Clone Slim to `~/.local/share/slim`.
3. Build the native `slim` binary.
4. Symlink it to `~/.local/bin/slim`.

Make sure `~/.local/bin` is in your `PATH`, then run:

```bash
slim myfile.txt
```

### Manual install

```bash
# 1. Install Slop
curl -fsSL https://raw.githubusercontent.com/gugu8intel-i9/Slop/main/install.sh | bash

# 2. Clone and build Slim
git clone https://github.com/gugu8intel-i9/Slim.git
cd Slim
./build.sh
./slim myfile.txt
```

To install the binary into `~/.local/bin` manually:

```bash
mkdir -p ~/.local/bin
cp slim ~/.local/bin/slim
```

### Build requirements

- Python 3 (only used by the Slop bootstrap fallback; prebuilt Linux x86_64
  binaries are preferred)
- GCC or Clang
- Git

If you installed Slop somewhere other than `~/.slop`, point `build.sh` at it:

```bash
SLOP_BIN=/path/to/slop/bin SLOP_INCLUDE=/path/to/slop/include ./build.sh
```

## Usage

```bash
slim <filename>
```

If `<filename>` does not exist, Slim starts with an empty buffer and creates it
on save.

## Controls

Slim starts in **insert mode** so you can type immediately.

| Key | Action |
|-----|--------|
| `Esc` | Enter **normal mode** |
| `i` | Enter **insert mode** (from normal mode) |
| `h` `j` `k` `l` | Move left / down / up / right (normal mode) |
| Arrow keys | Move cursor (both modes) |
| `0` | Jump to start of line (normal mode) |
| `$` | Jump to end of line (normal mode) |
| `PgUp` / `PgDn` | Scroll by one page (normal mode) |
| printable chars | Insert text (insert mode) |
| `Enter` | Insert new line (insert mode) |
| `Backspace` | Delete character before cursor (insert mode) |
| `Ctrl+S` | Save file |
| `Ctrl+Q` | Quit |

## Performance

Slim is designed for speed. Because it transpiles to native C through the Slop
compiler, uses Slop's arena memory model, and has no plugin system or heavy
runtime, it starts and quits faster than full-featured editors.

### Benchmark: startup + quit on a 1000-line file

Run on a standard Linux container with Vim 9.1 and Neovim 0.10.4 installed from
Debian packages. Each editor was started with a minimal configuration, opened a
1000-line text file, and immediately quit.

| Editor | Average | Median | Min | Max |
|--------|---------|--------|-----|-----|
| **Slim** | **1.52 ms** | **1.49 ms** | **1.45 ms** | **1.70 ms** |
| Vim (`vim -u NONE -es -c q`) | 2.75 ms | 2.68 ms | 2.48 ms | 3.10 ms |
| Neovim (`nvim -u NONE --headless -c q`) | 8.40 ms | 8.31 ms | 7.76 ms | 9.34 ms |

**Result:** Slim starts ~1.8× faster than Vim and ~5.5× faster than Neovim in
this minimal startup test.

### Why Slim is fast

- **Native compilation**: `slim.slop` transpiles to C and is compiled with
  `-O3 -ffast-math -flto`.
- **No garbage collector**: Slop uses arena allocation, so there are no GC
  pauses.
- **Tiny footprint**: the stripped binary is under 20 KB.
- **Minimal code path**: no config files, plugins, syntax highlighting, or LSP
  overhead at startup.

### Reproduce the benchmark

```bash
cd Slim
python3 benchmark.py
```

## Why Slim?

- **High performance**: transpiles to native C through the Slop compiler and uses
  Slop's arena memory model.
- **Lightweight**: a single `.slop` source file, no external UI libraries.
- **Vim-like**: modal editing with `hjkl` navigation.

## Files

- `slim.slop` — the editor source code written in Slop.
- `build.sh` — compiles `slim.slop` into a native executable using the Slop
  toolchain.
- `install.sh` — one-line installer script.
- `benchmark.py` — reproduces the performance comparison.
- `generate_slim.py` — helper script used to produce `slim.slop` from the
  embedded C editor core (development artifact).

## License

MIT
