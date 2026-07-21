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

1. Clone/update the Slop runtime into `~/.local/share/slop`.
2. Clone Slim to `~/.local/share/slim`.
3. Build the native `slim` binary using Slop's portable Python bootstrap transpiler.
4. Symlink it to `~/.local/bin/slim`.

Make sure `~/.local/bin` is in your `PATH`, then run:

```bash
slim myfile.txt
```

### Manual install

```bash
# 1. Clone Slop
git clone https://github.com/gugu8intel-i9/Slop.git ~/.local/share/slop

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

- Python 3
- GCC or Clang
- Git

If you cloned Slop somewhere other than `~/.local/share/slop`, point `build.sh` at it:

```bash
SLOP_REPO=/path/to/Slop ./build.sh
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
| `s` | Save file (normal mode) |
| `q` | Quit (normal mode) |
| `Ctrl+S` | Save file |
| `Ctrl+Q` | Quit |

## Syntax highlighting

Slim includes a lightweight built-in highlighter for common file types:

- **C / C++ / Slop / Rust / Go / JavaScript / TypeScript / Java** (`.c`, `.h`, `.cpp`, `.slop`, `.rs`, `.go`, `.js`, `.ts`, `.java`)
- **Python** (`.py`)
- **Shell** (`.sh`, `.bash`, `.zsh`)

Highlighted tokens:

| Token | Color |
|-------|-------|
| Keywords | magenta |
| Types | blue |
| Strings | green |
| Comments | gray |
| Numbers | yellow |
| Preprocessor | cyan |

Files with unrecognized extensions are rendered in the default terminal color.

### Supported languages

Slim recognizes and highlights **30+ language families** with a VS Code Dark+-inspired
256-color palette for broad terminal compatibility:

| Token | Color | Example |
|-------|-------|---------|
| Keywords | blue | `def`, `if`, `return` |
| Types | teal | `int`, `String`, `Vec` |
| Functions | yellow | `printf(...)`, `hello(...)` |
| Strings | orange | `"hello"` |
| Comments | green | `// comment` |
| Numbers | light green | `42`, `0xFF`, `3.14` |
| Preprocessor | purple | `#include`, `#!/bin/bash` |
| Operators | white | `+`, `=`, `->` |
| Attributes | gold | `@decorator` |

Supported languages include C, C++, Slop, Rust, Go, JavaScript, TypeScript, Java,
Python, Shell, Ruby, PHP, Lua, Perl, SQL, Swift, Kotlin, C#, HTML, XML, CSS,
JSON, YAML, TOML, INI, Makefile, Dockerfile, Lisp/Clojure, Haskell, Erlang,
Assembly, and Markdown.

### Diagnostics

Slim continuously checks the file for basic problems and shows the count in the
status bar, e.g. `[E:2 W:1]`.

Detected issues:

| Severity | Issue |
|----------|-------|
| Error | Unmatched `(`, `[`, or `{` |
| Error | Unclosed string or multi-line comment |
| Warning | Trailing whitespace |

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
- **Minimal code path**: no config files, plugins, or LSP overhead at startup.

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
- `slim_hl.c` / `slim_hl.h` — syntax highlighting engine (compiled separately
  for portability).
- `build.sh` — compiles `slim.slop` + `slim_hl.c` into a native executable.
- `install.sh` — one-line installer script.
- `benchmark.py` — reproduces the performance comparison.
- `generate_slim.py` — helper script used to produce `slim.slop`.
- `generate_slim_hl.py` — regenerates `slim_hl.c` / `slim_hl.h` from language
  definitions.

## License

MIT
