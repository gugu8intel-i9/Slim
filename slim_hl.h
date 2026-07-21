#ifndef SLIM_HL_H
#define SLIM_HL_H

#include "slop_rt.h"

int slop_hl_detect_lang(SlopString filename);
void slop_hl_print(SlopString* line, int start, int len, int lang);

#endif
