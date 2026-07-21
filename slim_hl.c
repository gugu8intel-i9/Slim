#include "slim_hl.h"
#include <string.h>
#include <unistd.h>

#define HL_RESET   "\x1b[0m"
#define HL_KEYWORD "\x1b[35m"
#define HL_TYPE    "\x1b[34m"
#define HL_STRING  "\x1b[32m"
#define HL_COMMENT "\x1b[90m"
#define HL_NUMBER  "\x1b[33m"
#define HL_PREPROC "\x1b[36m"

static int slop_hl_word_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static int slop_hl_is_kw(const char* w, int len, int lang) {
    if (lang == 3) {
        const char* kws[] = {"and","as","assert","break","class","continue","def","del","elif","else","except","False","finally","for","from","global","if","import","in","is","lambda","None","nonlocal","not","or","pass","raise","return","True","try","while","with","yield","print",NULL};
        for (int i = 0; kws[i]; i++) if ((int)strlen(kws[i]) == len && strncmp(w, kws[i], len) == 0) return 1;
        return 0;
    }
    if (lang == 4) {
        const char* kws[] = {"if","then","else","elif","fi","case","esac","for","do","done","while","until","function","return","exit","echo","read","local","export","source","trap","shift","set","unset","in",NULL};
        for (int i = 0; kws[i]; i++) if ((int)strlen(kws[i]) == len && strncmp(w, kws[i], len) == 0) return 1;
        return 0;
    }
    const char* kws[] = {"auto","break","case","const","continue","default","do","else","enum","extern","false","for","goto","if","inline","include","return","signed","sizeof","static","struct","switch","typedef","union","unsigned","volatile","while","define","ifdef","ifndef","endif","elif","pragma","error","warning","fn","let","raw","import","from","as","in","and","or","not","is","true","false","NULL",NULL};
    for (int i = 0; kws[i]; i++) if ((int)strlen(kws[i]) == len && strncmp(w, kws[i], len) == 0) return 1;
    return 0;
}

static int slop_hl_is_type(const char* w, int len, int lang) {
    if (lang == 3 || lang == 4) return 0;
    const char* types[] = {"bool","char","double","float","int","long","short","signed","size_t","ssize_t","uint8_t","uint16_t","uint32_t","uint64_t","int8_t","int16_t","int32_t","int64_t","void","FILE","SlopString","SlopArray","SlopArena","SlopTensor",NULL};
    for (int i = 0; types[i]; i++) if ((int)strlen(types[i]) == len && strncmp(w, types[i], len) == 0) return 1;
    return 0;
}

int slop_hl_detect_lang(SlopString filename) {
    if (filename.length < 3) return 0;
    const char* dot = strrchr(filename.data, '.');
    if (!dot) return 0;
    if (strcmp(dot, ".slop") == 0) return 1;
    if (strcmp(dot, ".c") == 0 || strcmp(dot, ".h") == 0) return 2;
    if (strcmp(dot, ".cpp") == 0 || strcmp(dot, ".cc") == 0 || strcmp(dot, ".hpp") == 0 || strcmp(dot, ".cxx") == 0) return 5;
    if (strcmp(dot, ".rs") == 0 || strcmp(dot, ".go") == 0 || strcmp(dot, ".js") == 0 || strcmp(dot, ".ts") == 0 || strcmp(dot, ".java") == 0) return 1;
    if (strcmp(dot, ".py") == 0) return 3;
    if (strcmp(dot, ".sh") == 0 || strcmp(dot, ".bash") == 0 || strcmp(dot, ".zsh") == 0) return 4;
    return 0;
}

void slop_hl_print(SlopString* line, int start, int len, int lang) {
    if (lang == 0 || len <= 0) {
        write(STDOUT_FILENO, line->data + start, len);
        return;
    }
    int end = start + len;
    if (end > (int)line->length) end = (int)line->length;
    int i = start;
    while (i < end) {
        if (lang == 3 || lang == 4) {
            if (line->data[i] == '#') {
                write(STDOUT_FILENO, HL_COMMENT, strlen(HL_COMMENT));
                write(STDOUT_FILENO, line->data + i, end - i);
                write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
                return;
            }
        } else {
            if (i < end - 1 && line->data[i] == '/' && line->data[i+1] == '/') {
                write(STDOUT_FILENO, HL_COMMENT, strlen(HL_COMMENT));
                write(STDOUT_FILENO, line->data + i, end - i);
                write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
                return;
            }
        }
        if (line->data[i] == '"' || line->data[i] == '\'') {
            char q = line->data[i];
            int j = i + 1;
            while (j < end && line->data[j] != q) {
                if (line->data[j] == '\\' && j + 1 < end) j += 2;
                else j++;
            }
            if (j < end) j++;
            write(STDOUT_FILENO, HL_STRING, strlen(HL_STRING));
            write(STDOUT_FILENO, line->data + i, j - i);
            write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
            i = j;
            continue;
        }
        if (i == start && lang != 3 && lang != 4 && line->data[i] == '#') {
            int j = i;
            while (j < end && line->data[j] != ' ' && line->data[j] != '\t') j++;
            write(STDOUT_FILENO, HL_PREPROC, strlen(HL_PREPROC));
            write(STDOUT_FILENO, line->data + i, j - i);
            write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
            i = j;
            continue;
        }
        if ((line->data[i] >= '0' && line->data[i] <= '9') ||
            (line->data[i] == '.' && i + 1 < end && line->data[i+1] >= '0' && line->data[i+1] <= '9')) {
            int j = i;
            while (j < end && ((line->data[j] >= '0' && line->data[j] <= '9') || line->data[j] == '.' ||
                   line->data[j] == 'x' || line->data[j] == 'X' ||
                   (line->data[j] >= 'a' && line->data[j] <= 'f') ||
                   (line->data[j] >= 'A' && line->data[j] <= 'F'))) j++;
            write(STDOUT_FILENO, HL_NUMBER, strlen(HL_NUMBER));
            write(STDOUT_FILENO, line->data + i, j - i);
            write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
            i = j;
            continue;
        }
        if (slop_hl_word_char(line->data[i])) {
            int j = i;
            while (j < end && slop_hl_word_char(line->data[j])) j++;
            int wlen = j - i;
            if (slop_hl_is_kw(line->data + i, wlen, lang)) {
                write(STDOUT_FILENO, HL_KEYWORD, strlen(HL_KEYWORD));
                write(STDOUT_FILENO, line->data + i, wlen);
                write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
            } else if (slop_hl_is_type(line->data + i, wlen, lang)) {
                write(STDOUT_FILENO, HL_TYPE, strlen(HL_TYPE));
                write(STDOUT_FILENO, line->data + i, wlen);
                write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
            } else {
                write(STDOUT_FILENO, line->data + i, wlen);
            }
            i = j;
            continue;
        }
        write(STDOUT_FILENO, line->data + i, 1);
        i++;
    }
}
