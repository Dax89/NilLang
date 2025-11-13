#pragma once

#include <nil/nil.h>
#include <stdio.h>

typedef FILE* NilFile;

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

NilFile nilio_openfile(const char* filepath, NilIOMode mode);
NilCell nilio_filesize(NilFile fp);
void nilio_writefile(NilFile fp, const char* data, NilCell len);
void nilio_readfile(NilFile fp, char* data, NilCell n);
void nilio_closefile(NilFile fp);
NilBuffer* nilio_slurpfile(const Nil* nil, const char* filepath);

NilBuffer* nilbuffer_create(NilCell siznile, const Nil* nil);
void nilbuffer_destroy(NilBuffer* self, Nil* nil);
