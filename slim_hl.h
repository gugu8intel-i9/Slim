#ifndef SLIM_HL_H
#define SLIM_HL_H

#include "slop_rt.h"

typedef struct {
    int errors;
    int warnings;
} SlopHlDiag;

int slop_hl_detect_lang(SlopString filename);
void slop_hl_print(SlopString* line, int start, int len, int lang);
SlopHlDiag slop_hl_diagnostics(SlopArray lines, int lang);

#endif
