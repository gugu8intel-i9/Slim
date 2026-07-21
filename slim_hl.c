#include "slim_hl.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>

// VS Code Dark+ inspired truecolor palette.
#define HL_DEFAULT   "\x1b[38;2;212;212;212m"
#define HL_KEYWORD   "\x1b[38;2;86;156;214m"
#define HL_TYPE      "\x1b[38;2;78;201;176m"
#define HL_FUNCTION  "\x1b[38;2;220;220;170m"
#define HL_STRING    "\x1b[38;2;206;145;120m"
#define HL_COMMENT   "\x1b[38;2;106;153;85m"
#define HL_NUMBER    "\x1b[38;2;181;206;168m"
#define HL_OPERATOR  "\x1b[38;2;212;212;212m"
#define HL_PREPROC   "\x1b[38;2;197;134;192m"
#define HL_ATTRIBUTE "\x1b[38;2;255;215;0m"
#define HL_REGEX     "\x1b[38;2;209;105;105m"
#define HL_ESCAPE    "\x1b[38;2;215;186;125m"
#define HL_VARIABLE  "\x1b[38;2;156;220;254m"
#define HL_RESET     "\x1b[0m"

typedef enum {
    HL_STYLE_DEFAULT = 0,
    HL_STYLE_KEYWORD,
    HL_STYLE_TYPE,
    HL_STYLE_FUNCTION,
    HL_STYLE_STRING,
    HL_STYLE_COMMENT,
    HL_STYLE_NUMBER,
    HL_STYLE_OPERATOR,
    HL_STYLE_PREPROC,
    HL_STYLE_ATTRIBUTE,
    HL_STYLE_REGEX,
    HL_STYLE_ESCAPE,
    HL_STYLE_VARIABLE,
    HL_STYLE_COUNT
} SlopHlStyle;

static const char* hl_styles[HL_STYLE_COUNT] = {
    HL_DEFAULT, HL_KEYWORD, HL_TYPE, HL_FUNCTION, HL_STRING,
    HL_COMMENT, HL_NUMBER, HL_OPERATOR, HL_PREPROC, HL_ATTRIBUTE,
    HL_REGEX, HL_ESCAPE, HL_VARIABLE
};

typedef struct {
    const char* word;
    int len;
    SlopHlStyle style;
} SlopKwEntry;

typedef struct {
    const char* single;
    const char* multi_start;
    const char* multi_end;
    const char* quotes;
    const char* operators;
    const char* string_prefixes;
    int has_preproc;
    int has_function;
    int has_regex;
    int has_attr;
    int has_shebang;
    int capitals_types;
    SlopKwEntry* keywords;
    int kw_count;
    SlopKwEntry* types;
    int type_count;
} SlopLangProfile;

typedef enum {
    LANG_NONE = 0,
    LANG_C,
    LANG_CPP,
    LANG_RUST,
    LANG_GO,
    LANG_JAVASCRIPT,
    LANG_TYPESCRIPT,
    LANG_JAVA,
    LANG_PYTHON,
    LANG_SHELL,
    LANG_RUBY,
    LANG_PHP,
    LANG_LUA,
    LANG_PERL,
    LANG_SQL,
    LANG_SWIFT,
    LANG_KOTLIN,
    LANG_CSHARP,
    LANG_HTML,
    LANG_XML,
    LANG_CSS,
    LANG_JSON,
    LANG_YAML,
    LANG_TOML,
    LANG_INI,
    LANG_DOCKERFILE,
    LANG_MAKEFILE,
    LANG_LISP,
    LANG_HASKELL,
    LANG_ERLANG,
    LANG_MARKDOWN,
    LANG_ASSEMBLY,
    LANG_COUNT
} SlopLangId;

static SlopKwEntry c_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_TYPE },
    { "auto", 4, HL_STYLE_KEYWORD },
    { "bool", 4, HL_STYLE_KEYWORD },
    { "bool", 4, HL_STYLE_TYPE },
    { "case", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_TYPE },
    { "elif", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "FILE", 4, HL_STYLE_TYPE },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "line", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_TYPE },
    { "NULL", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_TYPE },
    { "_Bool", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "endif", 5, HL_STYLE_KEYWORD },
    { "error", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_TYPE },
    { "ifdef", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_TYPE },
    { "undef", 5, HL_STYLE_KEYWORD },
    { "union", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "atomic", 6, HL_STYLE_KEYWORD },
    { "define", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_TYPE },
    { "extern", 6, HL_STYLE_KEYWORD },
    { "ifndef", 6, HL_STYLE_KEYWORD },
    { "inline", 6, HL_STYLE_KEYWORD },
    { "int8_t", 6, HL_STYLE_TYPE },
    { "pragma", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "signed", 6, HL_STYLE_KEYWORD },
    { "signed", 6, HL_STYLE_TYPE },
    { "size_t", 6, HL_STYLE_TYPE },
    { "sizeof", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "_Atomic", 7, HL_STYLE_KEYWORD },
    { "alignas", 7, HL_STYLE_KEYWORD },
    { "alignof", 7, HL_STYLE_KEYWORD },
    { "complex", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "defined", 7, HL_STYLE_KEYWORD },
    { "generic", 7, HL_STYLE_KEYWORD },
    { "include", 7, HL_STYLE_KEYWORD },
    { "int16_t", 7, HL_STYLE_TYPE },
    { "int32_t", 7, HL_STYLE_TYPE },
    { "int64_t", 7, HL_STYLE_TYPE },
    { "ssize_t", 7, HL_STYLE_TYPE },
    { "typedef", 7, HL_STYLE_KEYWORD },
    { "uint8_t", 7, HL_STYLE_TYPE },
    { "warning", 7, HL_STYLE_KEYWORD },
    { "_Alignas", 8, HL_STYLE_KEYWORD },
    { "_Alignof", 8, HL_STYLE_KEYWORD },
    { "_Complex", 8, HL_STYLE_KEYWORD },
    { "_Generic", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "noreturn", 8, HL_STYLE_KEYWORD },
    { "register", 8, HL_STYLE_KEYWORD },
    { "restrict", 8, HL_STYLE_KEYWORD },
    { "uint16_t", 8, HL_STYLE_TYPE },
    { "uint32_t", 8, HL_STYLE_TYPE },
    { "uint64_t", 8, HL_STYLE_TYPE },
    { "unsigned", 8, HL_STYLE_KEYWORD },
    { "volatile", 8, HL_STYLE_KEYWORD },
    { "_Noreturn", 9, HL_STYLE_KEYWORD },
    { "imaginary", 9, HL_STYLE_KEYWORD },
    { "SlopArena", 9, HL_STYLE_TYPE },
    { "SlopArray", 9, HL_STYLE_TYPE },
    { "_Imaginary", 10, HL_STYLE_KEYWORD },
    { "SlopString", 10, HL_STYLE_TYPE },
    { "SlopTensor", 10, HL_STYLE_TYPE },
    { "thread_local", 12, HL_STYLE_KEYWORD },
    { "_Thread_local", 13, HL_STYLE_KEYWORD },
    { "static_assert", 13, HL_STYLE_KEYWORD },
    { "_Static_assert", 14, HL_STYLE_KEYWORD },
};
static int c_kw_count = 93;

static SlopKwEntry cpp_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "asm", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_TYPE },
    { "map", 3, HL_STYLE_TYPE },
    { "new", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_TYPE },
    { "try", 3, HL_STYLE_KEYWORD },
    { "auto", 4, HL_STYLE_KEYWORD },
    { "bool", 4, HL_STYLE_TYPE },
    { "case", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_TYPE },
    { "elif", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "FILE", 4, HL_STYLE_TYPE },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "line", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_TYPE },
    { "NULL", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_TYPE },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "endif", 5, HL_STYLE_KEYWORD },
    { "error", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_TYPE },
    { "ifdef", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_TYPE },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "undef", 5, HL_STYLE_KEYWORD },
    { "union", 5, HL_STYLE_KEYWORD },
    { "using", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "define", 6, HL_STYLE_KEYWORD },
    { "delete", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_TYPE },
    { "export", 6, HL_STYLE_KEYWORD },
    { "extern", 6, HL_STYLE_KEYWORD },
    { "friend", 6, HL_STYLE_KEYWORD },
    { "ifndef", 6, HL_STYLE_KEYWORD },
    { "inline", 6, HL_STYLE_KEYWORD },
    { "int8_t", 6, HL_STYLE_TYPE },
    { "pragma", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "signed", 6, HL_STYLE_KEYWORD },
    { "signed", 6, HL_STYLE_TYPE },
    { "size_t", 6, HL_STYLE_TYPE },
    { "sizeof", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "string", 6, HL_STYLE_TYPE },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "typeid", 6, HL_STYLE_KEYWORD },
    { "vector", 6, HL_STYLE_TYPE },
    { "concept", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "defined", 7, HL_STYLE_KEYWORD },
    { "include", 7, HL_STYLE_KEYWORD },
    { "int16_t", 7, HL_STYLE_TYPE },
    { "int32_t", 7, HL_STYLE_TYPE },
    { "int64_t", 7, HL_STYLE_TYPE },
    { "mutable", 7, HL_STYLE_KEYWORD },
    { "nullptr", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "ssize_t", 7, HL_STYLE_TYPE },
    { "typedef", 7, HL_STYLE_KEYWORD },
    { "uint8_t", 7, HL_STYLE_TYPE },
    { "virtual", 7, HL_STYLE_KEYWORD },
    { "warning", 7, HL_STYLE_KEYWORD },
    { "wchar_t", 7, HL_STYLE_KEYWORD },
    { "wchar_t", 7, HL_STYLE_TYPE },
    { "co_await", 8, HL_STYLE_KEYWORD },
    { "co_yield", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "decltype", 8, HL_STYLE_KEYWORD },
    { "explicit", 8, HL_STYLE_KEYWORD },
    { "noexcept", 8, HL_STYLE_KEYWORD },
    { "operator", 8, HL_STYLE_KEYWORD },
    { "register", 8, HL_STYLE_KEYWORD },
    { "requires", 8, HL_STYLE_KEYWORD },
    { "template", 8, HL_STYLE_KEYWORD },
    { "typename", 8, HL_STYLE_KEYWORD },
    { "uint16_t", 8, HL_STYLE_TYPE },
    { "uint32_t", 8, HL_STYLE_TYPE },
    { "uint64_t", 8, HL_STYLE_TYPE },
    { "unsigned", 8, HL_STYLE_KEYWORD },
    { "volatile", 8, HL_STYLE_KEYWORD },
    { "co_return", 9, HL_STYLE_KEYWORD },
    { "consteval", 9, HL_STYLE_KEYWORD },
    { "constexpr", 9, HL_STYLE_KEYWORD },
    { "constinit", 9, HL_STYLE_KEYWORD },
    { "namespace", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "const_cast", 10, HL_STYLE_KEYWORD },
    { "static_cast", 11, HL_STYLE_KEYWORD },
    { "dynamic_cast", 12, HL_STYLE_KEYWORD },
    { "unordered_map", 13, HL_STYLE_TYPE },
    { "unordered_set", 13, HL_STYLE_TYPE },
    { "reinterpret_cast", 16, HL_STYLE_KEYWORD },
};
static int cpp_kw_count = 113;

static SlopKwEntry rust_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "fn", 2, HL_STYLE_KEYWORD },
    { "i8", 2, HL_STYLE_TYPE },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "Rc", 2, HL_STYLE_TYPE },
    { "u8", 2, HL_STYLE_TYPE },
    { "Arc", 3, HL_STYLE_TYPE },
    { "Box", 3, HL_STYLE_TYPE },
    { "dyn", 3, HL_STYLE_KEYWORD },
    { "f32", 3, HL_STYLE_TYPE },
    { "f64", 3, HL_STYLE_TYPE },
    { "for", 3, HL_STYLE_KEYWORD },
    { "i16", 3, HL_STYLE_TYPE },
    { "i32", 3, HL_STYLE_TYPE },
    { "i64", 3, HL_STYLE_TYPE },
    { "let", 3, HL_STYLE_KEYWORD },
    { "mod", 3, HL_STYLE_KEYWORD },
    { "mut", 3, HL_STYLE_KEYWORD },
    { "pub", 3, HL_STYLE_KEYWORD },
    { "ref", 3, HL_STYLE_KEYWORD },
    { "str", 3, HL_STYLE_TYPE },
    { "u16", 3, HL_STYLE_TYPE },
    { "u32", 3, HL_STYLE_TYPE },
    { "u64", 3, HL_STYLE_TYPE },
    { "use", 3, HL_STYLE_KEYWORD },
    { "Vec", 3, HL_STYLE_TYPE },
    { "bool", 4, HL_STYLE_TYPE },
    { "char", 4, HL_STYLE_TYPE },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "i128", 4, HL_STYLE_TYPE },
    { "impl", 4, HL_STYLE_KEYWORD },
    { "loop", 4, HL_STYLE_KEYWORD },
    { "move", 4, HL_STYLE_KEYWORD },
    { "self", 4, HL_STYLE_KEYWORD },
    { "Self", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "type", 4, HL_STYLE_KEYWORD },
    { "u128", 4, HL_STYLE_TYPE },
    { "async", 5, HL_STYLE_KEYWORD },
    { "await", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "crate", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "isize", 5, HL_STYLE_TYPE },
    { "match", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "trait", 5, HL_STYLE_KEYWORD },
    { "union", 5, HL_STYLE_KEYWORD },
    { "usize", 5, HL_STYLE_TYPE },
    { "where", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "extern", 6, HL_STYLE_KEYWORD },
    { "Option", 6, HL_STYLE_TYPE },
    { "Result", 6, HL_STYLE_TYPE },
    { "return", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "String", 6, HL_STYLE_TYPE },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "unsafe", 6, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "macro_rules", 11, HL_STYLE_KEYWORD },
};
static int rust_kw_count = 64;

static SlopKwEntry go_keywords[] = {
    { "go", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_TYPE },
    { "map", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "bool", 4, HL_STYLE_TYPE },
    { "byte", 4, HL_STYLE_TYPE },
    { "case", 4, HL_STYLE_KEYWORD },
    { "chan", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "func", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "int8", 4, HL_STYLE_TYPE },
    { "rune", 4, HL_STYLE_TYPE },
    { "type", 4, HL_STYLE_KEYWORD },
    { "uint", 4, HL_STYLE_TYPE },
    { "break", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "defer", 5, HL_STYLE_KEYWORD },
    { "error", 5, HL_STYLE_TYPE },
    { "int16", 5, HL_STYLE_TYPE },
    { "int32", 5, HL_STYLE_TYPE },
    { "int64", 5, HL_STYLE_TYPE },
    { "range", 5, HL_STYLE_KEYWORD },
    { "uint8", 5, HL_STYLE_TYPE },
    { "import", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "select", 6, HL_STYLE_KEYWORD },
    { "string", 6, HL_STYLE_TYPE },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "uint16", 6, HL_STYLE_TYPE },
    { "uint32", 6, HL_STYLE_TYPE },
    { "uint64", 6, HL_STYLE_TYPE },
    { "default", 7, HL_STYLE_KEYWORD },
    { "float32", 7, HL_STYLE_TYPE },
    { "float64", 7, HL_STYLE_TYPE },
    { "package", 7, HL_STYLE_KEYWORD },
    { "uintptr", 7, HL_STYLE_TYPE },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "complex64", 9, HL_STYLE_TYPE },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "complex128", 10, HL_STYLE_TYPE },
    { "fallthrough", 11, HL_STYLE_KEYWORD },
};
static int go_kw_count = 45;

static SlopKwEntry javascript_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "NaN", 3, HL_STYLE_KEYWORD },
    { "new", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "with", 4, HL_STYLE_KEYWORD },
    { "async", 5, HL_STYLE_KEYWORD },
    { "await", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "delete", 6, HL_STYLE_KEYWORD },
    { "export", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "typeof", 6, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "extends", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "debugger", 8, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
    { "Infinity", 8, HL_STYLE_KEYWORD },
    { "undefined", 9, HL_STYLE_KEYWORD },
    { "instanceof", 10, HL_STYLE_KEYWORD },
};
static int javascript_kw_count = 42;

static SlopKwEntry typescript_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "any", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "get", 3, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "NaN", 3, HL_STYLE_KEYWORD },
    { "new", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "type", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "with", 4, HL_STYLE_KEYWORD },
    { "async", 5, HL_STYLE_KEYWORD },
    { "await", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "keyof", 5, HL_STYLE_KEYWORD },
    { "never", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "delete", 6, HL_STYLE_KEYWORD },
    { "export", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "module", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "typeof", 6, HL_STYLE_KEYWORD },
    { "declare", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "extends", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "unknown", 7, HL_STYLE_KEYWORD },
    { "abstract", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "debugger", 8, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
    { "Infinity", 8, HL_STYLE_KEYWORD },
    { "readonly", 8, HL_STYLE_KEYWORD },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "namespace", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "satisfies", 9, HL_STYLE_KEYWORD },
    { "undefined", 9, HL_STYLE_KEYWORD },
    { "implements", 10, HL_STYLE_KEYWORD },
    { "instanceof", 10, HL_STYLE_KEYWORD },
};
static int typescript_kw_count = 63;

static SlopKwEntry java_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_KEYWORD },
    { "new", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "byte", 4, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "final", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "assert", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "native", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "throws", 6, HL_STYLE_KEYWORD },
    { "boolean", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "extends", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "package", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "abstract", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "strictfp", 8, HL_STYLE_KEYWORD },
    { "volatile", 8, HL_STYLE_KEYWORD },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "transient", 9, HL_STYLE_KEYWORD },
    { "implements", 10, HL_STYLE_KEYWORD },
    { "instanceof", 10, HL_STYLE_KEYWORD },
    { "synchronized", 12, HL_STYLE_KEYWORD },
};
static int java_kw_count = 53;

static SlopKwEntry python_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "is", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "cls", 3, HL_STYLE_KEYWORD },
    { "def", 3, HL_STYLE_KEYWORD },
    { "del", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "elif", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "from", 4, HL_STYLE_KEYWORD },
    { "None", 4, HL_STYLE_KEYWORD },
    { "pass", 4, HL_STYLE_KEYWORD },
    { "self", 4, HL_STYLE_KEYWORD },
    { "True", 4, HL_STYLE_KEYWORD },
    { "with", 4, HL_STYLE_KEYWORD },
    { "async", 5, HL_STYLE_KEYWORD },
    { "await", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "False", 5, HL_STYLE_KEYWORD },
    { "print", 5, HL_STYLE_KEYWORD },
    { "raise", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "assert", 6, HL_STYLE_KEYWORD },
    { "except", 6, HL_STYLE_KEYWORD },
    { "global", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "lambda", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "nonlocal", 8, HL_STYLE_KEYWORD },
};
static int python_kw_count = 38;

static SlopKwEntry shell_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "fi", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "done", 4, HL_STYLE_KEYWORD },
    { "echo", 4, HL_STYLE_KEYWORD },
    { "elif", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "esac", 4, HL_STYLE_KEYWORD },
    { "eval", 4, HL_STYLE_KEYWORD },
    { "exec", 4, HL_STYLE_KEYWORD },
    { "exit", 4, HL_STYLE_KEYWORD },
    { "read", 4, HL_STYLE_KEYWORD },
    { "then", 4, HL_STYLE_KEYWORD },
    { "trap", 4, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "local", 5, HL_STYLE_KEYWORD },
    { "shift", 5, HL_STYLE_KEYWORD },
    { "unset", 5, HL_STYLE_KEYWORD },
    { "until", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "export", 6, HL_STYLE_KEYWORD },
    { "printf", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "source", 6, HL_STYLE_KEYWORD },
    { "declare", 7, HL_STYLE_KEYWORD },
    { "typeset", 7, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
    { "readonly", 8, HL_STYLE_KEYWORD },
};
static int shell_kw_count = 33;

static SlopKwEntry ruby_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "def", 3, HL_STYLE_KEYWORD },
    { "end", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "nil", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "next", 4, HL_STYLE_KEYWORD },
    { "redo", 4, HL_STYLE_KEYWORD },
    { "self", 4, HL_STYLE_KEYWORD },
    { "then", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "when", 4, HL_STYLE_KEYWORD },
    { "alias", 5, HL_STYLE_KEYWORD },
    { "begin", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "elsif", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "retry", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "undef", 5, HL_STYLE_KEYWORD },
    { "until", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "ensure", 6, HL_STYLE_KEYWORD },
    { "module", 6, HL_STYLE_KEYWORD },
    { "rescue", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "unless", 6, HL_STYLE_KEYWORD },
    { "defined?", 8, HL_STYLE_KEYWORD },
};
static int ruby_kw_count = 36;

static SlopKwEntry php_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "fn", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "die", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "new", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "use", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "xor", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "echo", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "eval", 4, HL_STYLE_KEYWORD },
    { "exit", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "list", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "array", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "clone", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "empty", 5, HL_STYLE_KEYWORD },
    { "endif", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "final", 5, HL_STYLE_KEYWORD },
    { "isset", 5, HL_STYLE_KEYWORD },
    { "match", 5, HL_STYLE_KEYWORD },
    { "print", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "trait", 5, HL_STYLE_KEYWORD },
    { "unset", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "elseif", 6, HL_STYLE_KEYWORD },
    { "endfor", 6, HL_STYLE_KEYWORD },
    { "global", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "declare", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "extends", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "foreach", 7, HL_STYLE_KEYWORD },
    { "include", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "require", 7, HL_STYLE_KEYWORD },
    { "abstract", 8, HL_STYLE_KEYWORD },
    { "callable", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "endwhile", 8, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
    { "readonly", 8, HL_STYLE_KEYWORD },
    { "endswitch", 9, HL_STYLE_KEYWORD },
    { "insteadof", 9, HL_STYLE_KEYWORD },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "namespace", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "enddeclare", 10, HL_STYLE_KEYWORD },
    { "endforeach", 10, HL_STYLE_KEYWORD },
    { "implements", 10, HL_STYLE_KEYWORD },
    { "instanceof", 10, HL_STYLE_KEYWORD },
    { "include_once", 12, HL_STYLE_KEYWORD },
    { "require_once", 12, HL_STYLE_KEYWORD },
    { "__halt_compiler", 15, HL_STYLE_KEYWORD },
};
static int php_kw_count = 73;

static SlopKwEntry lua_keywords[] = {
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "end", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "nil", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "then", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "local", 5, HL_STYLE_KEYWORD },
    { "until", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "elseif", 6, HL_STYLE_KEYWORD },
    { "repeat", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
};
static int lua_kw_count = 22;

static SlopKwEntry perl_keywords[] = {
    { "q", 1, HL_STYLE_KEYWORD },
    { "y", 1, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "fc", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "lc", 2, HL_STYLE_KEYWORD },
    { "my", 2, HL_STYLE_KEYWORD },
    { "no", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "qq", 2, HL_STYLE_KEYWORD },
    { "qr", 2, HL_STYLE_KEYWORD },
    { "qw", 2, HL_STYLE_KEYWORD },
    { "qx", 2, HL_STYLE_KEYWORD },
    { "uc", 2, HL_STYLE_KEYWORD },
    { "abs", 3, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "chr", 3, HL_STYLE_KEYWORD },
    { "cmp", 3, HL_STYLE_KEYWORD },
    { "cos", 3, HL_STYLE_KEYWORD },
    { "die", 3, HL_STYLE_KEYWORD },
    { "eof", 3, HL_STYLE_KEYWORD },
    { "exp", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "hex", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_KEYWORD },
    { "log", 3, HL_STYLE_KEYWORD },
    { "map", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "oct", 3, HL_STYLE_KEYWORD },
    { "ord", 3, HL_STYLE_KEYWORD },
    { "our", 3, HL_STYLE_KEYWORD },
    { "pop", 3, HL_STYLE_KEYWORD },
    { "pos", 3, HL_STYLE_KEYWORD },
    { "ref", 3, HL_STYLE_KEYWORD },
    { "say", 3, HL_STYLE_KEYWORD },
    { "sin", 3, HL_STYLE_KEYWORD },
    { "sub", 3, HL_STYLE_KEYWORD },
    { "tie", 3, HL_STYLE_KEYWORD },
    { "use", 3, HL_STYLE_KEYWORD },
    { "vec", 3, HL_STYLE_KEYWORD },
    { "xor", 3, HL_STYLE_KEYWORD },
    { "bind", 4, HL_STYLE_KEYWORD },
    { "chop", 4, HL_STYLE_KEYWORD },
    { "dump", 4, HL_STYLE_KEYWORD },
    { "each", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "eval", 4, HL_STYLE_KEYWORD },
    { "exec", 4, HL_STYLE_KEYWORD },
    { "exit", 4, HL_STYLE_KEYWORD },
    { "fork", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "grep", 4, HL_STYLE_KEYWORD },
    { "join", 4, HL_STYLE_KEYWORD },
    { "keys", 4, HL_STYLE_KEYWORD },
    { "kill", 4, HL_STYLE_KEYWORD },
    { "last", 4, HL_STYLE_KEYWORD },
    { "link", 4, HL_STYLE_KEYWORD },
    { "next", 4, HL_STYLE_KEYWORD },
    { "open", 4, HL_STYLE_KEYWORD },
    { "pack", 4, HL_STYLE_KEYWORD },
    { "pipe", 4, HL_STYLE_KEYWORD },
    { "push", 4, HL_STYLE_KEYWORD },
    { "rand", 4, HL_STYLE_KEYWORD },
    { "read", 4, HL_STYLE_KEYWORD },
    { "recv", 4, HL_STYLE_KEYWORD },
    { "redo", 4, HL_STYLE_KEYWORD },
    { "seek", 4, HL_STYLE_KEYWORD },
    { "send", 4, HL_STYLE_KEYWORD },
    { "sort", 4, HL_STYLE_KEYWORD },
    { "sqrt", 4, HL_STYLE_KEYWORD },
    { "stat", 4, HL_STYLE_KEYWORD },
    { "tell", 4, HL_STYLE_KEYWORD },
    { "tied", 4, HL_STYLE_KEYWORD },
    { "time", 4, HL_STYLE_KEYWORD },
    { "wait", 4, HL_STYLE_KEYWORD },
    { "warn", 4, HL_STYLE_KEYWORD },
    { "atan2", 5, HL_STYLE_KEYWORD },
    { "bless", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "chdir", 5, HL_STYLE_KEYWORD },
    { "chmod", 5, HL_STYLE_KEYWORD },
    { "chomp", 5, HL_STYLE_KEYWORD },
    { "chown", 5, HL_STYLE_KEYWORD },
    { "close", 5, HL_STYLE_KEYWORD },
    { "crypt", 5, HL_STYLE_KEYWORD },
    { "elsif", 5, HL_STYLE_KEYWORD },
    { "fcntl", 5, HL_STYLE_KEYWORD },
    { "flock", 5, HL_STYLE_KEYWORD },
    { "index", 5, HL_STYLE_KEYWORD },
    { "ioctl", 5, HL_STYLE_KEYWORD },
    { "local", 5, HL_STYLE_KEYWORD },
    { "lstat", 5, HL_STYLE_KEYWORD },
    { "mkdir", 5, HL_STYLE_KEYWORD },
    { "print", 5, HL_STYLE_KEYWORD },
    { "reset", 5, HL_STYLE_KEYWORD },
    { "rmdir", 5, HL_STYLE_KEYWORD },
    { "semop", 5, HL_STYLE_KEYWORD },
    { "shift", 5, HL_STYLE_KEYWORD },
    { "sleep", 5, HL_STYLE_KEYWORD },
    { "split", 5, HL_STYLE_KEYWORD },
    { "srand", 5, HL_STYLE_KEYWORD },
    { "state", 5, HL_STYLE_KEYWORD },
    { "study", 5, HL_STYLE_KEYWORD },
    { "times", 5, HL_STYLE_KEYWORD },
    { "umask", 5, HL_STYLE_KEYWORD },
    { "undef", 5, HL_STYLE_KEYWORD },
    { "untie", 5, HL_STYLE_KEYWORD },
    { "until", 5, HL_STYLE_KEYWORD },
    { "utime", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "write", 5, HL_STYLE_KEYWORD },
    { "caller", 6, HL_STYLE_KEYWORD },
    { "chroot", 6, HL_STYLE_KEYWORD },
    { "delete", 6, HL_STYLE_KEYWORD },
    { "exists", 6, HL_STYLE_KEYWORD },
    { "fileno", 6, HL_STYLE_KEYWORD },
    { "format", 6, HL_STYLE_KEYWORD },
    { "length", 6, HL_STYLE_KEYWORD },
    { "listen", 6, HL_STYLE_KEYWORD },
    { "msgctl", 6, HL_STYLE_KEYWORD },
    { "msgget", 6, HL_STYLE_KEYWORD },
    { "msgrcv", 6, HL_STYLE_KEYWORD },
    { "msgsnd", 6, HL_STYLE_KEYWORD },
    { "printf", 6, HL_STYLE_KEYWORD },
    { "rename", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "rindex", 6, HL_STYLE_KEYWORD },
    { "scalar", 6, HL_STYLE_KEYWORD },
    { "select", 6, HL_STYLE_KEYWORD },
    { "semctl", 6, HL_STYLE_KEYWORD },
    { "semget", 6, HL_STYLE_KEYWORD },
    { "shmctl", 6, HL_STYLE_KEYWORD },
    { "shmget", 6, HL_STYLE_KEYWORD },
    { "socket", 6, HL_STYLE_KEYWORD },
    { "splice", 6, HL_STYLE_KEYWORD },
    { "substr", 6, HL_STYLE_KEYWORD },
    { "system", 6, HL_STYLE_KEYWORD },
    { "unless", 6, HL_STYLE_KEYWORD },
    { "values", 6, HL_STYLE_KEYWORD },
    { "binmode", 7, HL_STYLE_KEYWORD },
    { "connect", 7, HL_STYLE_KEYWORD },
    { "dbmopen", 7, HL_STYLE_KEYWORD },
    { "defined", 7, HL_STYLE_KEYWORD },
    { "foreach", 7, HL_STYLE_KEYWORD },
    { "lcfirst", 7, HL_STYLE_KEYWORD },
    { "opendir", 7, HL_STYLE_KEYWORD },
    { "package", 7, HL_STYLE_KEYWORD },
    { "readdir", 7, HL_STYLE_KEYWORD },
    { "require", 7, HL_STYLE_KEYWORD },
    { "reverse", 7, HL_STYLE_KEYWORD },
    { "seekdir", 7, HL_STYLE_KEYWORD },
    { "shmread", 7, HL_STYLE_KEYWORD },
    { "sprintf", 7, HL_STYLE_KEYWORD },
    { "symlink", 7, HL_STYLE_KEYWORD },
    { "sysopen", 7, HL_STYLE_KEYWORD },
    { "sysread", 7, HL_STYLE_KEYWORD },
    { "sysseek", 7, HL_STYLE_KEYWORD },
    { "telldir", 7, HL_STYLE_KEYWORD },
    { "ucfirst", 7, HL_STYLE_KEYWORD },
    { "unshift", 7, HL_STYLE_KEYWORD },
    { "waitpid", 7, HL_STYLE_KEYWORD },
    { "closedir", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "dbmclose", 8, HL_STYLE_KEYWORD },
    { "endgrent", 8, HL_STYLE_KEYWORD },
    { "endpwent", 8, HL_STYLE_KEYWORD },
    { "readline", 8, HL_STYLE_KEYWORD },
    { "readlink", 8, HL_STYLE_KEYWORD },
    { "readpipe", 8, HL_STYLE_KEYWORD },
    { "setgrent", 8, HL_STYLE_KEYWORD },
    { "setpwent", 8, HL_STYLE_KEYWORD },
    { "shmwrite", 8, HL_STYLE_KEYWORD },
    { "shutdown", 8, HL_STYLE_KEYWORD },
    { "syswrite", 8, HL_STYLE_KEYWORD },
    { "truncate", 8, HL_STYLE_KEYWORD },
    { "endnetent", 9, HL_STYLE_KEYWORD },
    { "localtime", 9, HL_STYLE_KEYWORD },
    { "prototype", 9, HL_STYLE_KEYWORD },
    { "quotemeta", 9, HL_STYLE_KEYWORD },
    { "rewinddir", 9, HL_STYLE_KEYWORD },
    { "setnetent", 9, HL_STYLE_KEYWORD },
    { "wantarray", 9, HL_STYLE_KEYWORD },
    { "endhostent", 10, HL_STYLE_KEYWORD },
    { "endservent", 10, HL_STYLE_KEYWORD },
    { "sethostent", 10, HL_STYLE_KEYWORD },
    { "setservent", 10, HL_STYLE_KEYWORD },
    { "setsockopt", 10, HL_STYLE_KEYWORD },
    { "socketpair", 10, HL_STYLE_KEYWORD },
    { "endprotoent", 11, HL_STYLE_KEYWORD },
    { "setprotoent", 11, HL_STYLE_KEYWORD },
};
static int perl_kw_count = 190;

static SlopKwEntry sql_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "by", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "is", 2, HL_STYLE_KEYWORD },
    { "of", 2, HL_STYLE_KEYWORD },
    { "on", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "to", 2, HL_STYLE_KEYWORD },
    { "add", 3, HL_STYLE_KEYWORD },
    { "all", 3, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "any", 3, HL_STYLE_KEYWORD },
    { "asc", 3, HL_STYLE_KEYWORD },
    { "end", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "key", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "off", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_KEYWORD },
    { "top", 3, HL_STYLE_KEYWORD },
    { "use", 3, HL_STYLE_KEYWORD },
    { "bulk", 4, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "dbcc", 4, HL_STYLE_KEYWORD },
    { "deny", 4, HL_STYLE_KEYWORD },
    { "desc", 4, HL_STYLE_KEYWORD },
    { "disk", 4, HL_STYLE_KEYWORD },
    { "drop", 4, HL_STYLE_KEYWORD },
    { "dump", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "exec", 4, HL_STYLE_KEYWORD },
    { "exit", 4, HL_STYLE_KEYWORD },
    { "file", 4, HL_STYLE_KEYWORD },
    { "from", 4, HL_STYLE_KEYWORD },
    { "full", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "into", 4, HL_STYLE_KEYWORD },
    { "join", 4, HL_STYLE_KEYWORD },
    { "kill", 4, HL_STYLE_KEYWORD },
    { "left", 4, HL_STYLE_KEYWORD },
    { "like", 4, HL_STYLE_KEYWORD },
    { "load", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "open", 4, HL_STYLE_KEYWORD },
    { "over", 4, HL_STYLE_KEYWORD },
    { "plan", 4, HL_STYLE_KEYWORD },
    { "proc", 4, HL_STYLE_KEYWORD },
    { "read", 4, HL_STYLE_KEYWORD },
    { "rule", 4, HL_STYLE_KEYWORD },
    { "save", 4, HL_STYLE_KEYWORD },
    { "some", 4, HL_STYLE_KEYWORD },
    { "then", 4, HL_STYLE_KEYWORD },
    { "tran", 4, HL_STYLE_KEYWORD },
    { "user", 4, HL_STYLE_KEYWORD },
    { "view", 4, HL_STYLE_KEYWORD },
    { "when", 4, HL_STYLE_KEYWORD },
    { "with", 4, HL_STYLE_KEYWORD },
    { "alter", 5, HL_STYLE_KEYWORD },
    { "begin", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "check", 5, HL_STYLE_KEYWORD },
    { "close", 5, HL_STYLE_KEYWORD },
    { "cross", 5, HL_STYLE_KEYWORD },
    { "fetch", 5, HL_STYLE_KEYWORD },
    { "grant", 5, HL_STYLE_KEYWORD },
    { "group", 5, HL_STYLE_KEYWORD },
    { "index", 5, HL_STYLE_KEYWORD },
    { "inner", 5, HL_STYLE_KEYWORD },
    { "merge", 5, HL_STYLE_KEYWORD },
    { "order", 5, HL_STYLE_KEYWORD },
    { "outer", 5, HL_STYLE_KEYWORD },
    { "pivot", 5, HL_STYLE_KEYWORD },
    { "print", 5, HL_STYLE_KEYWORD },
    { "right", 5, HL_STYLE_KEYWORD },
    { "table", 5, HL_STYLE_KEYWORD },
    { "union", 5, HL_STYLE_KEYWORD },
    { "where", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "backup", 6, HL_STYLE_KEYWORD },
    { "browse", 6, HL_STYLE_KEYWORD },
    { "column", 6, HL_STYLE_KEYWORD },
    { "commit", 6, HL_STYLE_KEYWORD },
    { "create", 6, HL_STYLE_KEYWORD },
    { "cursor", 6, HL_STYLE_KEYWORD },
    { "delete", 6, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_KEYWORD },
    { "errlvl", 6, HL_STYLE_KEYWORD },
    { "escape", 6, HL_STYLE_KEYWORD },
    { "except", 6, HL_STYLE_KEYWORD },
    { "exists", 6, HL_STYLE_KEYWORD },
    { "having", 6, HL_STYLE_KEYWORD },
    { "insert", 6, HL_STYLE_KEYWORD },
    { "lineno", 6, HL_STYLE_KEYWORD },
    { "nullif", 6, HL_STYLE_KEYWORD },
    { "option", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "revert", 6, HL_STYLE_KEYWORD },
    { "revoke", 6, HL_STYLE_KEYWORD },
    { "schema", 6, HL_STYLE_KEYWORD },
    { "select", 6, HL_STYLE_KEYWORD },
    { "unique", 6, HL_STYLE_KEYWORD },
    { "update", 6, HL_STYLE_KEYWORD },
    { "values", 6, HL_STYLE_KEYWORD },
    { "between", 7, HL_STYLE_KEYWORD },
    { "cascade", 7, HL_STYLE_KEYWORD },
    { "collate", 7, HL_STYLE_KEYWORD },
    { "compute", 7, HL_STYLE_KEYWORD },
    { "connect", 7, HL_STYLE_KEYWORD },
    { "convert", 7, HL_STYLE_KEYWORD },
    { "current", 7, HL_STYLE_KEYWORD },
    { "declare", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "execute", 7, HL_STYLE_KEYWORD },
    { "foreign", 7, HL_STYLE_KEYWORD },
    { "nocheck", 7, HL_STYLE_KEYWORD },
    { "offsets", 7, HL_STYLE_KEYWORD },
    { "openxml", 7, HL_STYLE_KEYWORD },
    { "percent", 7, HL_STYLE_KEYWORD },
    { "primary", 7, HL_STYLE_KEYWORD },
    { "restore", 7, HL_STYLE_KEYWORD },
    { "setuser", 7, HL_STYLE_KEYWORD },
    { "trigger", 7, HL_STYLE_KEYWORD },
    { "tsequal", 7, HL_STYLE_KEYWORD },
    { "unpivot", 7, HL_STYLE_KEYWORD },
    { "varying", 7, HL_STYLE_KEYWORD },
    { "waitfor", 7, HL_STYLE_KEYWORD },
    { "coalesce", 8, HL_STYLE_KEYWORD },
    { "contains", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "database", 8, HL_STYLE_KEYWORD },
    { "distinct", 8, HL_STYLE_KEYWORD },
    { "external", 8, HL_STYLE_KEYWORD },
    { "freetext", 8, HL_STYLE_KEYWORD },
    { "function", 8, HL_STYLE_KEYWORD },
    { "holdlock", 8, HL_STYLE_KEYWORD },
    { "identity", 8, HL_STYLE_KEYWORD },
    { "national", 8, HL_STYLE_KEYWORD },
    { "readtext", 8, HL_STYLE_KEYWORD },
    { "restrict", 8, HL_STYLE_KEYWORD },
    { "rollback", 8, HL_STYLE_KEYWORD },
    { "rowcount", 8, HL_STYLE_KEYWORD },
    { "shutdown", 8, HL_STYLE_KEYWORD },
    { "textsize", 8, HL_STYLE_KEYWORD },
    { "truncate", 8, HL_STYLE_KEYWORD },
    { "clustered", 9, HL_STYLE_KEYWORD },
    { "intersect", 9, HL_STYLE_KEYWORD },
    { "openquery", 9, HL_STYLE_KEYWORD },
    { "precision", 9, HL_STYLE_KEYWORD },
    { "procedure", 9, HL_STYLE_KEYWORD },
    { "raiserror", 9, HL_STYLE_KEYWORD },
    { "writetext", 9, HL_STYLE_KEYWORD },
    { "checkpoint", 10, HL_STYLE_KEYWORD },
    { "constraint", 10, HL_STYLE_KEYWORD },
    { "deallocate", 10, HL_STYLE_KEYWORD },
    { "fillfactor", 10, HL_STYLE_KEYWORD },
    { "openrowset", 10, HL_STYLE_KEYWORD },
    { "references", 10, HL_STYLE_KEYWORD },
    { "rowguidcol", 10, HL_STYLE_KEYWORD },
    { "statistics", 10, HL_STYLE_KEYWORD },
    { "updatetext", 10, HL_STYLE_KEYWORD },
    { "distributed", 11, HL_STYLE_KEYWORD },
    { "identitycol", 11, HL_STYLE_KEYWORD },
    { "reconfigure", 11, HL_STYLE_KEYWORD },
    { "replication", 11, HL_STYLE_KEYWORD },
    { "system_user", 11, HL_STYLE_KEYWORD },
    { "tablesample", 11, HL_STYLE_KEYWORD },
    { "transaction", 11, HL_STYLE_KEYWORD },
    { "try_convert", 11, HL_STYLE_KEYWORD },
    { "current_date", 12, HL_STYLE_KEYWORD },
    { "current_time", 12, HL_STYLE_KEYWORD },
    { "current_user", 12, HL_STYLE_KEYWORD },
    { "nonclustered", 12, HL_STYLE_KEYWORD },
    { "session_user", 12, HL_STYLE_KEYWORD },
    { "authorization", 13, HL_STYLE_KEYWORD },
    { "containstable", 13, HL_STYLE_KEYWORD },
    { "freetexttable", 13, HL_STYLE_KEYWORD },
    { "securityaudit", 13, HL_STYLE_KEYWORD },
    { "opendatasource", 14, HL_STYLE_KEYWORD },
    { "identity_insert", 15, HL_STYLE_KEYWORD },
    { "current_timestamp", 17, HL_STYLE_KEYWORD },
    { "semantickeyphrasetable", 22, HL_STYLE_KEYWORD },
    { "semanticsimilaritytable", 23, HL_STYLE_KEYWORD },
    { "semanticsimilaritydetailstable", 30, HL_STYLE_KEYWORD },
};
static int sql_kw_count = 185;

static SlopKwEntry swift_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "is", 2, HL_STYLE_KEYWORD },
    { "Any", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "nil", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "func", 4, HL_STYLE_KEYWORD },
    { "init", 4, HL_STYLE_KEYWORD },
    { "open", 4, HL_STYLE_KEYWORD },
    { "self", 4, HL_STYLE_KEYWORD },
    { "Self", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "defer", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "guard", 5, HL_STYLE_KEYWORD },
    { "inout", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "where", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "deinit", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "repeat", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "throws", 6, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "escaping", 8, HL_STYLE_KEYWORD },
    { "internal", 8, HL_STYLE_KEYWORD },
    { "operator", 8, HL_STYLE_KEYWORD },
    { "protocol", 8, HL_STYLE_KEYWORD },
    { "rethrows", 8, HL_STYLE_KEYWORD },
    { "available", 9, HL_STYLE_KEYWORD },
    { "extension", 9, HL_STYLE_KEYWORD },
    { "subscript", 9, HL_STYLE_KEYWORD },
    { "typealias", 9, HL_STYLE_KEYWORD },
    { "autoclosur", 10, HL_STYLE_KEYWORD },
    { "fallthrough", 11, HL_STYLE_KEYWORD },
    { "fileprivate", 11, HL_STYLE_KEYWORD },
    { "associatedtype", 14, HL_STYLE_KEYWORD },
    { "precedencegroup", 15, HL_STYLE_KEYWORD },
    { "discardableResult", 17, HL_STYLE_KEYWORD },
};
static int swift_kw_count = 58;

static SlopKwEntry kotlin_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "by", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "is", 2, HL_STYLE_KEYWORD },
    { "it", 2, HL_STYLE_KEYWORD },
    { "!in", 3, HL_STYLE_KEYWORD },
    { "!is", 3, HL_STYLE_KEYWORD },
    { "as?", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "fun", 3, HL_STYLE_KEYWORD },
    { "get", 3, HL_STYLE_KEYWORD },
    { "out", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "val", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "data", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "file", 4, HL_STYLE_KEYWORD },
    { "init", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "open", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "when", 4, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "field", 5, HL_STYLE_KEYWORD },
    { "final", 5, HL_STYLE_KEYWORD },
    { "infix", 5, HL_STYLE_KEYWORD },
    { "inner", 5, HL_STYLE_KEYWORD },
    { "param", 5, HL_STYLE_KEYWORD },
    { "super", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "value", 5, HL_STYLE_KEYWORD },
    { "where", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "actual", 6, HL_STYLE_KEYWORD },
    { "expect", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "inline", 6, HL_STYLE_KEYWORD },
    { "object", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "sealed", 6, HL_STYLE_KEYWORD },
    { "typeof", 6, HL_STYLE_KEYWORD },
    { "vararg", 6, HL_STYLE_KEYWORD },
    { "dynamic", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "package", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "reified", 7, HL_STYLE_KEYWORD },
    { "suspend", 7, HL_STYLE_KEYWORD },
    { "tailrec", 7, HL_STYLE_KEYWORD },
    { "abstract", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "delegate", 8, HL_STYLE_KEYWORD },
    { "external", 8, HL_STYLE_KEYWORD },
    { "internal", 8, HL_STYLE_KEYWORD },
    { "lateinit", 8, HL_STYLE_KEYWORD },
    { "noinline", 8, HL_STYLE_KEYWORD },
    { "operator", 8, HL_STYLE_KEYWORD },
    { "override", 8, HL_STYLE_KEYWORD },
    { "property", 8, HL_STYLE_KEYWORD },
    { "receiver", 8, HL_STYLE_KEYWORD },
    { "setparam", 8, HL_STYLE_KEYWORD },
    { "companion", 9, HL_STYLE_KEYWORD },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "typealias", 9, HL_STYLE_KEYWORD },
    { "annotation", 10, HL_STYLE_KEYWORD },
    { "constructor", 11, HL_STYLE_KEYWORD },
    { "crossinline", 11, HL_STYLE_KEYWORD },
};
static int kotlin_kw_count = 79;

static SlopKwEntry csharp_keywords[] = {
    { "as", 2, HL_STYLE_KEYWORD },
    { "by", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "is", 2, HL_STYLE_KEYWORD },
    { "on", 2, HL_STYLE_KEYWORD },
    { "add", 3, HL_STYLE_KEYWORD },
    { "for", 3, HL_STYLE_KEYWORD },
    { "get", 3, HL_STYLE_KEYWORD },
    { "int", 3, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "new", 3, HL_STYLE_KEYWORD },
    { "out", 3, HL_STYLE_KEYWORD },
    { "ref", 3, HL_STYLE_KEYWORD },
    { "set", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "var", 3, HL_STYLE_KEYWORD },
    { "base", 4, HL_STYLE_KEYWORD },
    { "bool", 4, HL_STYLE_KEYWORD },
    { "byte", 4, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "char", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "enum", 4, HL_STYLE_KEYWORD },
    { "from", 4, HL_STYLE_KEYWORD },
    { "goto", 4, HL_STYLE_KEYWORD },
    { "into", 4, HL_STYLE_KEYWORD },
    { "join", 4, HL_STYLE_KEYWORD },
    { "lock", 4, HL_STYLE_KEYWORD },
    { "long", 4, HL_STYLE_KEYWORD },
    { "null", 4, HL_STYLE_KEYWORD },
    { "this", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "uint", 4, HL_STYLE_KEYWORD },
    { "void", 4, HL_STYLE_KEYWORD },
    { "when", 4, HL_STYLE_KEYWORD },
    { "alias", 5, HL_STYLE_KEYWORD },
    { "async", 5, HL_STYLE_KEYWORD },
    { "await", 5, HL_STYLE_KEYWORD },
    { "break", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "const", 5, HL_STYLE_KEYWORD },
    { "event", 5, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
    { "fixed", 5, HL_STYLE_KEYWORD },
    { "float", 5, HL_STYLE_KEYWORD },
    { "group", 5, HL_STYLE_KEYWORD },
    { "sbyte", 5, HL_STYLE_KEYWORD },
    { "short", 5, HL_STYLE_KEYWORD },
    { "throw", 5, HL_STYLE_KEYWORD },
    { "ulong", 5, HL_STYLE_KEYWORD },
    { "using", 5, HL_STYLE_KEYWORD },
    { "value", 5, HL_STYLE_KEYWORD },
    { "where", 5, HL_STYLE_KEYWORD },
    { "while", 5, HL_STYLE_KEYWORD },
    { "yield", 5, HL_STYLE_KEYWORD },
    { "double", 6, HL_STYLE_KEYWORD },
    { "equals", 6, HL_STYLE_KEYWORD },
    { "extern", 6, HL_STYLE_KEYWORD },
    { "global", 6, HL_STYLE_KEYWORD },
    { "nameof", 6, HL_STYLE_KEYWORD },
    { "object", 6, HL_STYLE_KEYWORD },
    { "params", 6, HL_STYLE_KEYWORD },
    { "public", 6, HL_STYLE_KEYWORD },
    { "remove", 6, HL_STYLE_KEYWORD },
    { "return", 6, HL_STYLE_KEYWORD },
    { "sealed", 6, HL_STYLE_KEYWORD },
    { "select", 6, HL_STYLE_KEYWORD },
    { "sizeof", 6, HL_STYLE_KEYWORD },
    { "static", 6, HL_STYLE_KEYWORD },
    { "string", 6, HL_STYLE_KEYWORD },
    { "struct", 6, HL_STYLE_KEYWORD },
    { "switch", 6, HL_STYLE_KEYWORD },
    { "typeof", 6, HL_STYLE_KEYWORD },
    { "unsafe", 6, HL_STYLE_KEYWORD },
    { "ushort", 6, HL_STYLE_KEYWORD },
    { "checked", 7, HL_STYLE_KEYWORD },
    { "decimal", 7, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "dynamic", 7, HL_STYLE_KEYWORD },
    { "finally", 7, HL_STYLE_KEYWORD },
    { "foreach", 7, HL_STYLE_KEYWORD },
    { "orderby", 7, HL_STYLE_KEYWORD },
    { "partial", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { "virtual", 7, HL_STYLE_KEYWORD },
    { "abstract", 8, HL_STYLE_KEYWORD },
    { "continue", 8, HL_STYLE_KEYWORD },
    { "delegate", 8, HL_STYLE_KEYWORD },
    { "explicit", 8, HL_STYLE_KEYWORD },
    { "implicit", 8, HL_STYLE_KEYWORD },
    { "internal", 8, HL_STYLE_KEYWORD },
    { "operator", 8, HL_STYLE_KEYWORD },
    { "override", 8, HL_STYLE_KEYWORD },
    { "readonly", 8, HL_STYLE_KEYWORD },
    { "volatile", 8, HL_STYLE_KEYWORD },
    { "ascending", 9, HL_STYLE_KEYWORD },
    { "interface", 9, HL_STYLE_KEYWORD },
    { "namespace", 9, HL_STYLE_KEYWORD },
    { "protected", 9, HL_STYLE_KEYWORD },
    { "unchecked", 9, HL_STYLE_KEYWORD },
    { "descending", 10, HL_STYLE_KEYWORD },
    { "stackalloc", 10, HL_STYLE_KEYWORD },
};
static int csharp_kw_count = 105;



static SlopKwEntry css_keywords[] = {
    { "-o-", 3, HL_STYLE_KEYWORD },
    { "-ms-", 4, HL_STYLE_KEYWORD },
    { "-moz-", 5, HL_STYLE_KEYWORD },
    { "@page", 5, HL_STYLE_KEYWORD },
    { "@media", 6, HL_STYLE_KEYWORD },
    { "@import", 7, HL_STYLE_KEYWORD },
    { "-webkit-", 8, HL_STYLE_KEYWORD },
    { "@charset", 8, HL_STYLE_KEYWORD },
    { "property", 8, HL_STYLE_KEYWORD },
    { "@document", 9, HL_STYLE_KEYWORD },
    { "@supports", 9, HL_STYLE_KEYWORD },
    { "@viewport", 9, HL_STYLE_KEYWORD },
    { "@font-face", 10, HL_STYLE_KEYWORD },
    { "@keyframes", 10, HL_STYLE_KEYWORD },
    { "@namespace", 10, HL_STYLE_KEYWORD },
    { "color-adjust", 12, HL_STYLE_KEYWORD },
    { "@counter-style", 14, HL_STYLE_KEYWORD },
};
static int css_kw_count = 17;

static SlopKwEntry json_keywords[] = {
    { "null", 4, HL_STYLE_KEYWORD },
    { "true", 4, HL_STYLE_KEYWORD },
    { "false", 5, HL_STYLE_KEYWORD },
};
static int json_kw_count = 3;




static SlopKwEntry dockerfile_keywords[] = {
    { "AS", 2, HL_STYLE_KEYWORD },
    { "ADD", 3, HL_STYLE_KEYWORD },
    { "ARG", 3, HL_STYLE_KEYWORD },
    { "CMD", 3, HL_STYLE_KEYWORD },
    { "ENV", 3, HL_STYLE_KEYWORD },
    { "RUN", 3, HL_STYLE_KEYWORD },
    { "COPY", 4, HL_STYLE_KEYWORD },
    { "FROM", 4, HL_STYLE_KEYWORD },
    { "USER", 4, HL_STYLE_KEYWORD },
    { "LABEL", 5, HL_STYLE_KEYWORD },
    { "SHELL", 5, HL_STYLE_KEYWORD },
    { "EXPOSE", 6, HL_STYLE_KEYWORD },
    { "VOLUME", 6, HL_STYLE_KEYWORD },
    { "ONBUILD", 7, HL_STYLE_KEYWORD },
    { "WORKDIR", 7, HL_STYLE_KEYWORD },
    { "ENTRYPOINT", 10, HL_STYLE_KEYWORD },
    { "MAINTAINER", 10, HL_STYLE_KEYWORD },
    { "STOPSIGNAL", 10, HL_STYLE_KEYWORD },
    { "HEALTHCHECK", 11, HL_STYLE_KEYWORD },
};
static int dockerfile_kw_count = 19;

static SlopKwEntry makefile_keywords[] = {
    { "else", 4, HL_STYLE_KEYWORD },
    { "ifeq", 4, HL_STYLE_KEYWORD },
    { "endef", 5, HL_STYLE_KEYWORD },
    { "endif", 5, HL_STYLE_KEYWORD },
    { "ifneq", 5, HL_STYLE_KEYWORD },
    { "vpath", 5, HL_STYLE_KEYWORD },
    { ".PHONY", 6, HL_STYLE_KEYWORD },
    { ".POSIX", 6, HL_STYLE_KEYWORD },
    { "define", 6, HL_STYLE_KEYWORD },
    { "export", 6, HL_STYLE_KEYWORD },
    { ".IGNORE", 7, HL_STYLE_KEYWORD },
    { ".SILENT", 7, HL_STYLE_KEYWORD },
    { "include", 7, HL_STYLE_KEYWORD },
    { "private", 7, HL_STYLE_KEYWORD },
    { ".DEFAULT", 8, HL_STYLE_KEYWORD },
    { "override", 8, HL_STYLE_KEYWORD },
    { ".ONESHELL", 9, HL_STYLE_KEYWORD },
    { ".PRECIOUS", 9, HL_STYLE_KEYWORD },
    { ".SUFFIXES", 9, HL_STYLE_KEYWORD },
    { ".SECONDARY", 10, HL_STYLE_KEYWORD },
    { ".NOTPARALLEL", 12, HL_STYLE_KEYWORD },
    { ".INTERMEDIATE", 13, HL_STYLE_KEYWORD },
    { ".DELETE_ON_ERROR", 16, HL_STYLE_KEYWORD },
    { ".SECONDEXPANSION", 16, HL_STYLE_KEYWORD },
    { ".LOW_RESOLUTION_TIME", 20, HL_STYLE_KEYWORD },
    { ".EXPORT_ALL_VARIABLES", 21, HL_STYLE_KEYWORD },
};
static int makefile_kw_count = 26;


static SlopKwEntry haskell_keywords[] = {
    { "_", 1, HL_STYLE_KEYWORD },
    { "as", 2, HL_STYLE_KEYWORD },
    { "do", 2, HL_STYLE_KEYWORD },
    { "if", 2, HL_STYLE_KEYWORD },
    { "in", 2, HL_STYLE_KEYWORD },
    { "of", 2, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "mdo", 3, HL_STYLE_KEYWORD },
    { "rec", 3, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "data", 4, HL_STYLE_KEYWORD },
    { "else", 4, HL_STYLE_KEYWORD },
    { "proc", 4, HL_STYLE_KEYWORD },
    { "then", 4, HL_STYLE_KEYWORD },
    { "type", 4, HL_STYLE_KEYWORD },
    { "class", 5, HL_STYLE_KEYWORD },
    { "infix", 5, HL_STYLE_KEYWORD },
    { "where", 5, HL_STYLE_KEYWORD },
    { "forall", 6, HL_STYLE_KEYWORD },
    { "hiding", 6, HL_STYLE_KEYWORD },
    { "import", 6, HL_STYLE_KEYWORD },
    { "infixl", 6, HL_STYLE_KEYWORD },
    { "infixr", 6, HL_STYLE_KEYWORD },
    { "module", 6, HL_STYLE_KEYWORD },
    { "default", 7, HL_STYLE_KEYWORD },
    { "foreign", 7, HL_STYLE_KEYWORD },
    { "newtype", 7, HL_STYLE_KEYWORD },
    { "deriving", 8, HL_STYLE_KEYWORD },
    { "instance", 8, HL_STYLE_KEYWORD },
    { "qualified", 9, HL_STYLE_KEYWORD },
};
static int haskell_kw_count = 30;

static SlopKwEntry erlang_keywords[] = {
    { "if", 2, HL_STYLE_KEYWORD },
    { "of", 2, HL_STYLE_KEYWORD },
    { "or", 2, HL_STYLE_KEYWORD },
    { "and", 3, HL_STYLE_KEYWORD },
    { "bor", 3, HL_STYLE_KEYWORD },
    { "bsl", 3, HL_STYLE_KEYWORD },
    { "bsr", 3, HL_STYLE_KEYWORD },
    { "div", 3, HL_STYLE_KEYWORD },
    { "end", 3, HL_STYLE_KEYWORD },
    { "fun", 3, HL_STYLE_KEYWORD },
    { "let", 3, HL_STYLE_KEYWORD },
    { "not", 3, HL_STYLE_KEYWORD },
    { "rem", 3, HL_STYLE_KEYWORD },
    { "try", 3, HL_STYLE_KEYWORD },
    { "xor", 3, HL_STYLE_KEYWORD },
    { "band", 4, HL_STYLE_KEYWORD },
    { "bnot", 4, HL_STYLE_KEYWORD },
    { "bxor", 4, HL_STYLE_KEYWORD },
    { "case", 4, HL_STYLE_KEYWORD },
    { "cond", 4, HL_STYLE_KEYWORD },
    { "when", 4, HL_STYLE_KEYWORD },
    { "after", 5, HL_STYLE_KEYWORD },
    { "begin", 5, HL_STYLE_KEYWORD },
    { "catch", 5, HL_STYLE_KEYWORD },
    { "orelse", 6, HL_STYLE_KEYWORD },
    { "andalso", 7, HL_STYLE_KEYWORD },
    { "receive", 7, HL_STYLE_KEYWORD },
};
static int erlang_kw_count = 27;



static SlopLangProfile slop_profiles[LANG_COUNT] = {
    [LANG_NONE] = { NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_C] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, c_keywords, 93, NULL, 0 },
    [LANG_CPP] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, cpp_keywords, 113, NULL, 0 },
    [LANG_RUST] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, rust_keywords, 64, NULL, 0 },
    [LANG_GO] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, go_keywords, 45, NULL, 0 },
    [LANG_JAVASCRIPT] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, javascript_keywords, 42, NULL, 0 },
    [LANG_TYPESCRIPT] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, typescript_keywords, 63, NULL, 0 },
    [LANG_JAVA] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, java_keywords, 53, NULL, 0 },
    [LANG_PYTHON] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", "fFrRuUbB", 0, 1, 0, 1, 1, 0, python_keywords, 38, NULL, 0 },
    [LANG_SHELL] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.$", NULL, 0, 0, 0, 0, 1, 0, shell_keywords, 33, NULL, 0 },
    [LANG_RUBY] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 1, 1, 0, ruby_keywords, 36, NULL, 0 },
    [LANG_PHP] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, php_keywords, 73, NULL, 0 },
    [LANG_LUA] = { ";", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 0, 0, 0, lua_keywords, 22, NULL, 0 },
    [LANG_PERL] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 1, 1, 0, perl_keywords, 190, NULL, 0 },
    [LANG_SQL] = { "--", "/*", "*/", "'\"", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 0, 0, 0, sql_keywords, 185, NULL, 0 },
    [LANG_SWIFT] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, swift_keywords, 58, NULL, 0 },
    [LANG_KOTLIN] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, kotlin_keywords, 79, NULL, 0 },
    [LANG_CSHARP] = { "//", "/*", "*/", "\"'", "+-*/%=<>!&|^~?:.", NULL, 1, 1, 0, 0, 0, 0, csharp_keywords, 105, NULL, 0 },
    [LANG_HTML] = { NULL, "<!--", "-->", "\"'", NULL, NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_XML] = { NULL, "<!--", "-->", "\"'", NULL, NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_CSS] = { NULL, "/*", "*/", "\"'", ":;{},", NULL, 0, 1, 0, 0, 0, 0, css_keywords, 17, NULL, 0 },
    [LANG_JSON] = { NULL, NULL, NULL, "\"", ":,{}[]", NULL, 0, 0, 0, 0, 0, 0, json_keywords, 3, NULL, 0 },
    [LANG_YAML] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 1, 1, 0, NULL, 0, NULL, 0 },
    [LANG_TOML] = { "#", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 1, 1, 0, NULL, 0, NULL, 0 },
    [LANG_INI] = { "#;", NULL, NULL, "\"'", "=", NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_DOCKERFILE] = { "#", NULL, NULL, "\"'", NULL, NULL, 0, 0, 0, 0, 0, 0, dockerfile_keywords, 19, NULL, 0 },
    [LANG_MAKEFILE] = { "#", NULL, NULL, "\"'", ":=?$()", NULL, 0, 0, 0, 0, 0, 0, makefile_keywords, 26, NULL, 0 },
    [LANG_LISP] = { ";", "#|", "|#", "\"", NULL, NULL, 0, 1, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_HASKELL] = { "--", "{-", "-}", "\"'", "+-*/%=<>!&|^~?:.\\$", NULL, 0, 1, 0, 0, 0, 0, haskell_keywords, 30, NULL, 0 },
    [LANG_ERLANG] = { "%", NULL, NULL, "\"'", "+-*/%=<>!&|^~?:.", NULL, 0, 1, 0, 0, 0, 0, erlang_keywords, 27, NULL, 0 },
    [LANG_MARKDOWN] = { NULL, NULL, NULL, "\"'`", "`*#-_|>![]()", NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
    [LANG_ASSEMBLY] = { ";", NULL, NULL, "'\"", "+-*/%=<>!&|^~?:.", NULL, 0, 0, 0, 0, 0, 0, NULL, 0, NULL, 0 },
};

static struct { const char* ext; int lang; } slop_ext_map[] = {
    { ".properties", LANG_INI },
    { ".dockerfile", LANG_DOCKERFILE },
    { "GNUmakefile", LANG_MAKEFILE },
    { "Dockerfile", LANG_DOCKERFILE },
    { ".markdown", LANG_MARKDOWN },
    { ".gemspec", LANG_RUBY },
    { "Makefile", LANG_MAKEFILE },
    { "makefile", LANG_MAKEFILE },
    { ".phtml", LANG_PHP },
    { ".swift", LANG_SWIFT },
    { ".xhtml", LANG_HTML },
    { ".mdown", LANG_MARKDOWN },
    { ".slop", LANG_C },
    { ".java", LANG_JAVA },
    { ".bash", LANG_SHELL },
    { ".fish", LANG_SHELL },
    { ".rake", LANG_RUBY },
    { ".html", LANG_HTML },
    { ".scss", LANG_CSS },
    { ".sass", LANG_CSS },
    { ".less", LANG_CSS },
    { ".json", LANG_JSON },
    { ".yaml", LANG_YAML },
    { ".toml", LANG_TOML },
    { ".conf", LANG_INI },
    { ".lisp", LANG_LISP },
    { ".cljs", LANG_LISP },
    { ".cljc", LANG_LISP },
    { ".nasm", LANG_ASSEMBLY },
    { ".cpp", LANG_CPP },
    { ".cxx", LANG_CPP },
    { ".hpp", LANG_CPP },
    { ".hxx", LANG_CPP },
    { ".mjs", LANG_JAVASCRIPT },
    { ".cjs", LANG_JAVASCRIPT },
    { ".tsx", LANG_TYPESCRIPT },
    { ".pyw", LANG_PYTHON },
    { ".pyi", LANG_PYTHON },
    { ".zsh", LANG_SHELL },
    { ".ksh", LANG_SHELL },
    { ".php", LANG_PHP },
    { ".lua", LANG_LUA },
    { ".sql", LANG_SQL },
    { ".kts", LANG_KOTLIN },
    { ".htm", LANG_HTML },
    { ".xml", LANG_XML },
    { ".svg", LANG_XML },
    { ".xsd", LANG_XML },
    { ".xsl", LANG_XML },
    { ".css", LANG_CSS },
    { ".yml", LANG_YAML },
    { ".ini", LANG_INI },
    { ".cfg", LANG_INI },
    { ".clj", LANG_LISP },
    { ".scm", LANG_LISP },
    { ".rkt", LANG_LISP },
    { ".lhs", LANG_HASKELL },
    { ".erl", LANG_ERLANG },
    { ".hrl", LANG_ERLANG },
    { ".asm", LANG_ASSEMBLY },
    { ".cc", LANG_CPP },
    { ".rs", LANG_RUST },
    { ".go", LANG_GO },
    { ".js", LANG_JAVASCRIPT },
    { ".ts", LANG_TYPESCRIPT },
    { ".py", LANG_PYTHON },
    { ".sh", LANG_SHELL },
    { ".rb", LANG_RUBY },
    { ".pl", LANG_PERL },
    { ".pm", LANG_PERL },
    { ".kt", LANG_KOTLIN },
    { ".cs", LANG_CSHARP },
    { ".mk", LANG_MAKEFILE },
    { ".ss", LANG_LISP },
    { ".hs", LANG_HASKELL },
    { ".md", LANG_MARKDOWN },
    { ".c", LANG_C },
    { ".h", LANG_C },
    { ".s", LANG_ASSEMBLY },
    { NULL, LANG_NONE }
};

static int slop_hl_word_char(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_';
}

static int slop_hl_kw_cmp(const void* a, const void* b) {
    const SlopKwEntry* ka = (const SlopKwEntry*)a;
    const SlopKwEntry* kb = (const SlopKwEntry*)b;
    if (ka->len != kb->len) return ka->len - kb->len;
    return strcmp(ka->word, kb->word);
}

static const SlopKwEntry* slop_hl_find_kw(const SlopKwEntry* table, int count, const char* w, int len) {
    if (!table || count <= 0) return NULL;
    SlopKwEntry key = { w, len, HL_STYLE_DEFAULT };
    return (const SlopKwEntry*)bsearch(&key, table, count, sizeof(SlopKwEntry), slop_hl_kw_cmp);
}

static int slop_hl_startswith(const char* s, int slen, int pos, const char* prefix) {
    if (!prefix) return 0;
    int plen = (int)strlen(prefix);
    return pos + plen <= slen && memcmp(s + pos, prefix, plen) == 0;
}

static int slop_hl_find_char(const char* set, char c) {
    if (!set) return 0;
    for (int i = 0; set[i]; i++) if (set[i] == c) return 1;
    return 0;
}

static int slop_hl_is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void slop_hl_emit(SlopHlStyle style, const char* text, int len) {
    write(STDOUT_FILENO, hl_styles[style], strlen(hl_styles[style]));
    write(STDOUT_FILENO, text, len);
    write(STDOUT_FILENO, HL_RESET, strlen(HL_RESET));
}

int slop_hl_detect_lang(SlopString filename) {
    if (filename.length == 0) return LANG_NONE;
    for (int i = 0; slop_ext_map[i].ext; i++) {
        const char* ext = slop_ext_map[i].ext;
        int elen = (int)strlen(ext);
        if ((int)filename.length >= elen && memcmp(filename.data + filename.length - elen, ext, elen) == 0) {
            return slop_ext_map[i].lang;
        }
    }
    const char* slash = strrchr(filename.data, '/');
    const char* name = slash ? slash + 1 : filename.data;
    for (int i = 0; slop_ext_map[i].ext; i++) {
        const char* ext = slop_ext_map[i].ext;
        if (ext[0] != '.') {
            if (strncmp(name, ext, strlen(ext)) == 0) return slop_ext_map[i].lang;
        }
    }
    return LANG_NONE;
}

void slop_hl_print(SlopString* line, int start, int len, int lang) {
    if (lang <= 0 || lang >= LANG_COUNT || len <= 0) {
        write(STDOUT_FILENO, line->data + start, len);
        return;
    }
    const SlopLangProfile* p = &slop_profiles[lang];
    int end = start + len;
    if (end > (int)line->length) end = (int)line->length;
    int i = start;

    // Track current style so we only emit color codes on change.
    SlopHlStyle cur_style = HL_STYLE_DEFAULT;
    int run_start = i;

    static int in_ml = 0;
    if (start == 0) in_ml = 0; // per-line reset for visible region; diagnostics handles real state

    #define FLUSH_STYLE() do {                 if (i > run_start) {                     if (cur_style != HL_STYLE_DEFAULT) slop_hl_emit(cur_style, line->data + run_start, i - run_start);                     else write(STDOUT_FILENO, line->data + run_start, i - run_start);                 }                 run_start = i;             } while (0)

    #define SET_STYLE(s) do {                 if (cur_style != (s)) {                     FLUSH_STYLE();                     cur_style = (s);                 }             } while (0)

    while (i < end) {
        // Multi-line comments visible within line
        if (in_ml) {
            SET_STYLE(HL_STYLE_COMMENT);
            int j = i;
            while (j < end && !(p->multi_end && slop_hl_startswith(line->data, end, j, p->multi_end))) j++;
            if (j < end && p->multi_end) j += (int)strlen(p->multi_end);
            i = j;
            in_ml = (j >= end || !(p->multi_end && slop_hl_startswith(line->data, end, i - (int)strlen(p->multi_end), p->multi_end)));
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Shebang
        if (i == start && p->has_shebang && slop_hl_startswith(line->data, end, i, "#!")) {
            SET_STYLE(HL_STYLE_PREPROC);
            i = end;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Single-line comments
        if (p->single) {
            int in_single = 0;
            for (int k = 0; p->single[k]; k++) {
                if (line->data[i] == p->single[k]) { in_single = 1; break; }
            }
            if (in_single) {
                SET_STYLE(HL_STYLE_COMMENT);
                i = end;
                FLUSH_STYLE();
                cur_style = HL_STYLE_DEFAULT;
                continue;
            }
        }

        // Multi-line comment start
        if (p->multi_start && slop_hl_startswith(line->data, end, i, p->multi_start)) {
            SET_STYLE(HL_STYLE_COMMENT);
            int j = i + (int)strlen(p->multi_start);
            while (j < end && !(p->multi_end && slop_hl_startswith(line->data, end, j, p->multi_end))) j++;
            if (j < end && p->multi_end) j += (int)strlen(p->multi_end);
            else in_ml = 1;
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Preprocessor (C-like) at line start
        if (p->has_preproc && i == start && line->data[i] == '#') {
            SET_STYLE(HL_STYLE_PREPROC);
            int j = i;
            while (j < end && line->data[j] != ' ' && line->data[j] != '\t') j++;
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Attributes / decorators (e.g. @name, #[attr])
        if (p->has_attr && line->data[i] == '@') {
            SET_STYLE(HL_STYLE_ATTRIBUTE);
            int j = i + 1;
            while (j < end && slop_hl_word_char(line->data[j])) j++;
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Strings with prefix handling (e.g. f"", r"" in Python)
        if (p->quotes && slop_hl_find_char(p->quotes, line->data[i])) {
            char q = line->data[i];
            SET_STYLE(HL_STYLE_STRING);
            int j = i + 1;
            while (j < end && line->data[j] != q) {
                if (line->data[j] == '\\' && j + 1 < end) {
                    // highlight escape separately
                    if (j > run_start) { FLUSH_STYLE(); }
                    j += 2;
                    run_start = j;
                } else {
                    j++;
                }
            }
            if (j < end) j++;
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // String prefixes (e.g. Python f"", Rust r#""")
        if (p->string_prefixes && slop_hl_find_char(p->string_prefixes, line->data[i])) {
            int prefix_end = i + 1;
            while (prefix_end < end && slop_hl_find_char(p->string_prefixes, line->data[prefix_end])) prefix_end++;
            if (prefix_end < end && p->quotes && slop_hl_find_char(p->quotes, line->data[prefix_end])) {
                SET_STYLE(HL_STYLE_STRING);
                char q = line->data[prefix_end];
                int j = prefix_end + 1;
                while (j < end && line->data[j] != q) {
                    if (line->data[j] == '\\' && j + 1 < end) j += 2;
                    else j++;
                }
                if (j < end) j++;
                i = j;
                FLUSH_STYLE();
                cur_style = HL_STYLE_DEFAULT;
                continue;
            }
        }

        // Regex literals for JS/TS/Perl: /pattern/flags
        if (p->has_regex && line->data[i] == '/') {
            int j = i + 1;
            while (j < end && line->data[j] != '/' && line->data[j] != '\n') {
                if (line->data[j] == '\\' && j + 1 < end) j += 2;
                else j++;
            }
            if (j < end && line->data[j] == '/') {
                while (j + 1 < end && strchr("gimsuy", line->data[j+1])) j++;
                SET_STYLE(HL_STYLE_REGEX);
                i = j + 1;
                FLUSH_STYLE();
                cur_style = HL_STYLE_DEFAULT;
                continue;
            }
        }

        // Numbers
        if ((line->data[i] >= '0' && line->data[i] <= '9') ||
            (line->data[i] == '.' && i + 1 < end && line->data[i+1] >= '0' && line->data[i+1] <= '9')) {
            SET_STYLE(HL_STYLE_NUMBER);
            int j = i;
            if (j + 1 < end && line->data[j] == '0' && strchr("xXbBoO", line->data[j+1])) {
                j += 2;
                while (j < end && (isxdigit((unsigned char)line->data[j]) || line->data[j] == '_')) j++;
            } else {
                while (j < end && ((line->data[j] >= '0' && line->data[j] <= '9') || line->data[j] == '.' || line->data[j] == '_')) j++;
                if (j < end && strchr("eE", line->data[j])) {
                    j++;
                    if (j < end && (line->data[j] == '+' || line->data[j] == '-')) j++;
                    while (j < end && (line->data[j] >= '0' && line->data[j] <= '9')) j++;
                }
                while (j < end && strchr("uUlLfF", line->data[j])) j++;
            }
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Words
        if (slop_hl_word_char(line->data[i])) {
            int j = i;
            while (j < end && slop_hl_word_char(line->data[j])) j++;
            int wlen = j - i;
            const SlopKwEntry* kw = slop_hl_find_kw(p->keywords, p->kw_count, line->data + i, wlen);
            if (kw) {
                SET_STYLE(kw->style);
            } else if (p->capitals_types && wlen > 0 && line->data[i] >= 'A' && line->data[i] <= 'Z') {
                SET_STYLE(HL_STYLE_TYPE);
            } else if (p->has_function && j < end && line->data[j] == '(') {
                SET_STYLE(HL_STYLE_FUNCTION);
            } else {
                SET_STYLE(HL_STYLE_DEFAULT);
            }
            i = j;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Operators
        if (p->operators && slop_hl_find_char(p->operators, line->data[i])) {
            SET_STYLE(HL_STYLE_OPERATOR);
            i++;
            FLUSH_STYLE();
            cur_style = HL_STYLE_DEFAULT;
            continue;
        }

        // Default char
        SET_STYLE(HL_STYLE_DEFAULT);
        i++;
    }
    FLUSH_STYLE();

    #undef FLUSH_STYLE
    #undef SET_STYLE
}

SlopHlDiag slop_hl_diagnostics(SlopArray lines, int lang) {
    SlopHlDiag diag = {0, 0};
    if (lang <= 0 || lang >= LANG_COUNT) return diag;
    const SlopLangProfile* p = &slop_profiles[lang];
    int paren = 0, bracket = 0, brace = 0;
    int in_ml = 0;
    int in_string = 0;
    char string_quote = 0;

    for (int row = 0; row < (int)lines.length; row++) {
        SlopString* line = (SlopString*)slop_array_get(lines, row);
        int len = (int)line->length;
        int trailing_ws = 0;
        for (int i = len - 1; i >= 0; i--) {
            if (line->data[i] == ' ' || line->data[i] == '\t') trailing_ws++;
            else break;
        }
        if (trailing_ws > 0) diag.warnings++;

        for (int i = 0; i < len; ) {
            if (in_ml) {
                if (p->multi_end && slop_hl_startswith(line->data, len, i, p->multi_end)) {
                    i += (int)strlen(p->multi_end);
                    in_ml = 0;
                } else {
                    i++;
                }
                continue;
            }
            if (p->single) {
                int in_single = 0;
                for (int k = 0; p->single[k]; k++) {
                    if (line->data[i] == p->single[k]) { in_single = 1; break; }
                }
                if (in_single) break;
            }
            if (p->multi_start && slop_hl_startswith(line->data, len, i, p->multi_start)) {
                i += (int)strlen(p->multi_start);
                in_ml = 1;
                continue;
            }
            if (!in_string && p->quotes && slop_hl_find_char(p->quotes, line->data[i])) {
                in_string = 1;
                string_quote = line->data[i];
                i++;
                continue;
            }
            if (in_string) {
                if (line->data[i] == '\\' && i + 1 < len) { i += 2; continue; }
                if (line->data[i] == string_quote) { in_string = 0; }
                i++;
                continue;
            }
            if (line->data[i] == '(') paren++;
            else if (line->data[i] == ')') paren--;
            else if (line->data[i] == '[') bracket++;
            else if (line->data[i] == ']') bracket--;
            else if (line->data[i] == '{') brace++;
            else if (line->data[i] == '}') brace--;
            i++;
        }
    }

    diag.errors += (paren < 0 ? -paren : paren);
    diag.errors += (bracket < 0 ? -bracket : bracket);
    diag.errors += (brace < 0 ? -brace : brace);
    if (in_string) diag.errors++;
    if (in_ml) diag.errors++;
    return diag;
}
