#pragma once

#include "token.h"
#include <stdbool.h>

typedef struct NilLexer {
    const char* current;
    const char* start;
    int start_line;
    int start_col;
    int length;
    int line;
    int col;
} NilLexer;

void nillexer_init(NilLexer* self, const char* source);
NilToken nillexer_lex(NilLexer* self);
