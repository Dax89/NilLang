#pragma once

#include "types.h"
#include <nil/nil.h>
#include <stdio.h>

typedef FILE* NilFile;

#define NIL_SUCCESS NIL_FALSE
#define NIL_FAIL NIL_TRUE

typedef enum NilIOMode {
    NIO_NONE = 0,
    NIO_R = 1,
    NIO_W = 2,
    NIO_RW = NIO_R | NIO_W,
} NilIOMode;

typedef struct NilFileBuffer {
    NilCell size;
    char data[];
} NilBuffer;

typedef struct NilResult {
    union {
        NilBuffer* buffer;
        NilFile file;
        NilCell value;
    };

    NilCell err;
} NilResult;

static inline bool nilresult_ok(const NilResult* self) {
    return self->err == NIL_SUCCESS;
}

NilResult nilio_fileopen(const char* filepath, NilIOMode mode);
NilResult nilio_filesize(NilFile fp);
NilResult nilio_filepos(NilFile fp);
NilResult nilio_filewrite(NilFile fp, const char* data, NilCell n);
NilResult nilio_filewriteln(NilFile fp, const char* data, NilCell n);
NilResult nilio_fileputchar(NilFile fp, char ch);
NilResult nilio_fileread(NilFile fp, char* data, NilCell n);
NilResult nilio_filereadln(NilFile fp, char* data, NilCell n);
NilResult nilio_filegetchar(NilFile fp);
void nilio_fileclose(NilFile fp);
NilResult nilio_fileslurp(const Nil* nil, const char* filepath);

NilBuffer* nilbuffer_create(NilCell size, const Nil* nil);
void nilbuffer_destroy(NilBuffer* self, Nil* nil);
