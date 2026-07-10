#!/usr/bin/env python3
"""Generate slim.slop from the embedded C editor core."""

C_CODE = r'''
#include <termios.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct termios orig_termios;
tcgetattr(STDIN_FILENO, &orig_termios);
struct termios raw = orig_termios;
raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
raw.c_oflag &= ~(OPOST);
raw.c_cflag |= (CS8);
raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
raw.c_cc[VMIN] = 0;
raw.c_cc[VTIME] = 1;
tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);

struct winsize ws;
int screenrows = 24, screencols = 80;
if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0) { screenrows = ws.ws_row; screencols = ws.ws_col; }
if (screenrows < 3) screenrows = 3;
if (screencols < 10) screencols = 10;

int cx = 0, cy = 0, rowoff = 0, coloff = 0, running = 1, mode = 1;

// If split produced a trailing empty line because file ended with newline, drop it.
if (lines.length > 0) {
    SlopString* last = (SlopString*)slop_array_get(lines, lines.length - 1);
    if (last->length == 0) lines.length--;
}
if (lines.length == 0) {
    SlopString* empty = (SlopString*)slop_arena_alloc(local_arena, sizeof(SlopString));
    empty->data = "";
    empty->length = 0;
    slop_array_push(local_arena, &lines, empty);
}

while (running) {
    // hide cursor and home
    write(STDOUT_FILENO, "\x1b[?25l", 6);
    write(STDOUT_FILENO, "\x1b[H", 3);

    int draw_rows = screenrows - 1;
    for (int y = 0; y < draw_rows; y++) {
        int filerow = y + rowoff;
        if (filerow >= (int)lines.length) {
            write(STDOUT_FILENO, "~", 1);
        } else {
            SlopString* line = (SlopString*)slop_array_get(lines, filerow);
            int start = coloff;
            int len = (int)line->length - start;
            if (len < 0) len = 0;
            if (len > screencols) len = screencols;
            if (len > 0) write(STDOUT_FILENO, line->data + start, len);
        }
        write(STDOUT_FILENO, "\x1b[K", 3);
        if (y < draw_rows - 1) write(STDOUT_FILENO, "\r\n", 2);
    }

    // status bar
    char status[256];
    const char* fname = filename.length ? filename.data : "[No Name]";
    int status_len = snprintf(status, sizeof(status),
        " %s %.20s | %dL %s | %d:%d ",
        mode ? "[INSERT]" : "[NORMAL]", fname, (int)lines.length, dirty ? "[+]" : "",
        cy + 1, cx + 1);
    if (status_len < 0) status_len = 0;
    write(STDOUT_FILENO, "\r\n", 2);
    write(STDOUT_FILENO, status, status_len);
    write(STDOUT_FILENO, "\x1b[K", 3);

    // position cursor
    char buf[32];
    int cursor_y = cy - rowoff + 1;
    int cursor_x = cx - coloff + 1;
    int bl = snprintf(buf, sizeof(buf), "\x1b[%d;%dH", cursor_y, cursor_x);
    write(STDOUT_FILENO, buf, bl);
    write(STDOUT_FILENO, "\x1b[?25h", 6);

    // read key
    char c;
    int n = read(STDIN_FILENO, &c, 1);
    if (n != 1) continue;

    if (c == 17) { running = 0; continue; } // Ctrl+Q
    if (c == 19) { // Ctrl+S
        if (filename.length == 0) continue;
        FILE* f = fopen(filename.data, "wb");
        if (f) {
            for (int i = 0; i < (int)lines.length; i++) {
                SlopString* line = (SlopString*)slop_array_get(lines, i);
                fwrite(line->data, 1, line->length, f);
                if (i < (int)lines.length - 1) fwrite("\n", 1, 1, f);
            }
            fclose(f);
            dirty = 0;
        }
        continue;
    }

    if (mode == 0) {
        if (c == 'i' || c == 'I') { mode = 1; continue; }
        if (c == 'h') { if (cx > 0) cx--; continue; }
        if (c == 'j') { if (cy < (int)lines.length - 1) cy++; continue; }
        if (c == 'k') { if (cy > 0) cy--; continue; }
        if (c == 'l') { cx++; continue; }
        if (c == '0') { cx = 0; continue; }
        if (c == '$') { if (cy < (int)lines.length) { SlopString* ln = (SlopString*)slop_array_get(lines, cy); cx = (int)ln->length; } continue; }
        continue;
    }

    if (c == 27) {
        char seq[3];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) { mode = 0; continue; }
        if (read(STDIN_FILENO, &seq[1], 1) != 1) { mode = 0; continue; }
        if (seq[0] == '[') {
            if (seq[1] == 'A') { if (cy > 0) cy--; }
            else if (seq[1] == 'B') { if (cy < (int)lines.length - 1) cy++; }
            else if (seq[1] == 'C') { cx++; }
            else if (seq[1] == 'D') { if (cx > 0) cx--; }
            else if (seq[1] == 'H') { cx = 0; }
            else if (seq[1] == 'F') { if (cy < (int)lines.length) { SlopString* ln = (SlopString*)slop_array_get(lines, cy); cx = (int)ln->length; } }
            else if (seq[1] == '5') { char tilde; read(STDIN_FILENO, &tilde, 1); cy -= (screenrows - 1); if (cy < 0) cy = 0; }
            else if (seq[1] == '6') { char tilde; read(STDIN_FILENO, &tilde, 1); cy += (screenrows - 1); if (cy >= (int)lines.length) cy = (int)lines.length - 1; }
            else { mode = 0; continue; }
        } else {
            mode = 0; continue;
        }
        continue;
    }

    if (c == 127) {
        if (cx > 0 && cy < (int)lines.length) {
            SlopString* old = (SlopString*)slop_array_get(lines, cy);
            int new_len = (int)old->length - 1;
            char* nd = (char*)slop_arena_alloc(local_arena, new_len + 1);
            memcpy(nd, old->data, cx - 1);
            memcpy(nd + cx - 1, old->data + cx, old->length - cx);
            nd[new_len] = '\0';
            old->data = nd;
            old->length = new_len;
            cx--;
            dirty = 1;
        } else if (cx == 0 && cy > 0) {
            SlopString* prev = (SlopString*)slop_array_get(lines, cy - 1);
            SlopString* cur = (SlopString*)slop_array_get(lines, cy);
            int prev_len = (int)prev->length;
            int new_len = prev_len + (int)cur->length;
            char* nd = (char*)slop_arena_alloc(local_arena, new_len + 1);
            memcpy(nd, prev->data, prev_len);
            memcpy(nd + prev_len, cur->data, cur->length);
            nd[new_len] = '\0';
            prev->data = nd;
            prev->length = new_len;
            for (int i = cy; i < (int)lines.length - 1; i++) lines.data[i] = lines.data[i + 1];
            lines.length--;
            cy--;
            cx = prev_len;
            dirty = 1;
        }
    } else if (c == '\r' || c == '\n') {
        if (cy < (int)lines.length) {
            SlopString* old = (SlopString*)slop_array_get(lines, cy);
            int new_len = (int)old->length - cx;
            char* nd = (char*)slop_arena_alloc(local_arena, new_len + 1);
            memcpy(nd, old->data + cx, new_len);
            nd[new_len] = '\0';
            SlopString* new_line = (SlopString*)slop_arena_alloc(local_arena, sizeof(SlopString));
            new_line->data = nd;
            new_line->length = new_len;
            old->length = cx;

            if (lines.length >= lines.capacity) {
                size_t new_cap = lines.capacity ? lines.capacity * 2 : 8;
                void** new_data = (void**)slop_arena_alloc(local_arena, new_cap * sizeof(void*));
                memcpy(new_data, lines.data, lines.length * sizeof(void*));
                lines.data = new_data;
                lines.capacity = new_cap;
            }
            for (int i = (int)lines.length; i > cy + 1; i--) lines.data[i] = lines.data[i - 1];
            lines.data[cy + 1] = new_line;
            lines.length++;
            cy++;
            cx = 0;
            dirty = 1;
        }
    } else if (c >= 32 && c < 127) {
        if (cy < (int)lines.length) {
            SlopString* old = (SlopString*)slop_array_get(lines, cy);
            int new_len = (int)old->length + 1;
            char* nd = (char*)slop_arena_alloc(local_arena, new_len + 1);
            memcpy(nd, old->data, cx);
            nd[cx] = c;
            memcpy(nd + cx + 1, old->data + cx, old->length - cx);
            nd[new_len] = '\0';
            old->data = nd;
            old->length = new_len;
            cx++;
            dirty = 1;
        }
    }

    if (cy < 0) cy = 0;
    if (cy >= (int)lines.length) cy = (int)lines.length - 1;
    if (cx < 0) cx = 0;
    if (cy >= 0 && cy < (int)lines.length) {
        SlopString* ln = (SlopString*)slop_array_get(lines, cy);
        if (cx > (int)ln->length) cx = (int)ln->length;
    } else {
        cx = 0;
    }

    if (cy < rowoff) rowoff = cy;
    if (cy >= rowoff + screenrows - 1) rowoff = cy - screenrows + 2;
    if (cx < coloff) coloff = cx;
    if (cx >= coloff + screencols) coloff = cx - screencols + 1;
}

tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
write(STDOUT_FILENO, "\x1b[2J", 4);
write(STDOUT_FILENO, "\x1b[H", 3);
'''

def main():
    lines = C_CODE.strip().split('\n')
    out = []
    out.append('# Slim - A high-performance, lightweight terminal text editor')
    out.append('# Written in Slop (https://github.com/gugu8intel-i9/Slop)')
    out.append('#')
    out.append('# Usage: slop slim.slop <file>')
    out.append('# Controls:')
    out.append('#   h j k l / arrows       Move cursor (normal mode)')
    out.append('#   Home/End               Line start/end')
    out.append('#   PgUp/PgDn              Scroll page')
    out.append('#   i                        Enter insert mode (normal mode)')
    out.append('#   Esc                      Return to normal mode')
    out.append('#   Printable / Enter / BS   Insert text (insert mode)')
    out.append('#   Ctrl+S                 Save')
    out.append('#   Ctrl+Q                 Quit')
    out.append('')
    out.append('fn main(args: array[string]) {')
    out.append('    let filename = ""')
    out.append('    if length(args) > 1 {')
    out.append('        filename = args[1]')
    out.append('    }')
    out.append('')
    out.append('    let content = read_file(filename)')
    out.append('    let lines = split(content, "\\n")')
    out.append('    let dirty = false')
    out.append('')
    # Wrap each C line in a raw block.
    for cl in lines:
        escaped = cl.replace('\\', '\\\\').replace('"', '\\"')
        out.append(f'    raw {{ "{escaped}" }}')
    out.append('')
    out.append('    # File is saved via Ctrl+S inside the editor loop.')
    out.append('}')
    out.append('')
    print('\n'.join(out))

if __name__ == '__main__':
    main()
