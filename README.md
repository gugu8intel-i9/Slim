# Slim

A high-performance, lightweight terminal text editor written in
[Slop](https://github.com/gugu8intel-i9/Slop).

Slim is a Vim-inspired modal editor that stays small, fast, and dependency-free
(except for the Slop runtime and a C compiler).

## Build

```bash
./build.sh
```

The build script uses the Slop Python boot transpiler to generate C, then
compiles it with GCC at `-O3 -ffast-math -flto` for native speed.

If your Slop repository is not at `/home/user/slop_repo`, set:

```bash
SLOP_REPO=/path/to/Slop ./build.sh
```

## Usage

```bash
./slim <filename>
```

If `<filename>` does not exist, Slim starts with an empty buffer and will create
it on save.

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

## Why Slim?

- **High performance**: transpiles to native C and uses Slop's arena memory model.
- **Lightweight**: a single `.slop` source file, no external UI libraries.
- **Vim-like**: modal editing with `hjkl` navigation.

## License

MIT


## Files

- `slim.slop` — the editor source code written in Slop.
- `build.sh` — compiles `slim.slop` into a native executable.
- `generate_slim.py` — helper script used to produce `slim.slop` from the
  embedded C editor core (development artifact).
