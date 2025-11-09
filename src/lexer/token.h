#pragma once

// clang-format off
typedef enum NilTokenType {
    NILT_INVALID = 0,
    NILT_WORD,
    NILT_STR, NILT_INT, NILT_REAL,
    NILT_ROUNDOPEN, NILT_ROUNDCLOSE,
    NILT_ERROR, NILT_EOF,
    NILT_COUNT,
} NilTokenType;
// clang-format on

typedef struct NilToken {
    int line;
    int col;

    NilTokenType type;
    const char* value;
    int length;
} NilToken;
