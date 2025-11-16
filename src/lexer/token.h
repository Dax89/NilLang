#pragma once

// clang-format off
typedef enum NilTokenType {
    NILT_EOF = 0,
    NILT_IDENTIFIER,
    NILT_CHAR, NILT_STR, 
    NILT_INT, NILT_REAL,
    NILT_ERROR,
} NilTokenType;
// clang-format on

typedef struct NilToken {
    int line;
    int col;

    NilTokenType type;
    const char* value;
    int length;
} NilToken;
