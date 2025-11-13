#include "io.h"
#include <stdlib.h>

static const char* _nilio_modestr(NilIOMode mode) {
    switch(mode) {
        case NIO_R: return "rb";
        case NIO_W: return "wb";
        case NIO_RW: return "rwb";
        default: break;
    }

    fprintf(stderr, "NilIO: invalid mode");
    exit(-2);
}

NilFile nilio_openfile(const char* filepath, NilIOMode mode) {
    NilFile fp = fopen(filepath, _nilio_modestr(mode));
    if(fp) return fp;

    fprintf(stderr, "NilIO: could not open file \"%s\".\n", filepath);
    exit(-1);
}

NilCell nilio_filesize(NilFile fp) {
    int oldp = ftell(fp);
    fseek(fp, 0, SEEK_END);
    NilCell sz = ftell(fp);
    fseek(fp, oldp, SEEK_SET);
    return sz;
}

void nilio_writefile(NilFile fp, const char* data, NilCell len) {
    if(fwrite(data, 1, len, fp) == len) return;

    fprintf(stderr, "NilIO: writing failed");
    exit(-1);
}

void nilio_readfile(NilFile fp, char* data, NilCell n) {
    if(fread(data, sizeof(char), n, fp) != n) {
        fprintf(stderr, "NilIO: reading failed");
        exit(-1);
    }
}

void nilio_closefile(NilFile fp) { fclose(fp); }

NilBuffer* nilio_slurpfile(const Nil* nil, const char* filepath) {
    NilFile fp = nilio_openfile(filepath, NIO_R);
    NilBuffer* buffer = nilbuffer_create(nilio_filesize(fp), nil);
    nilio_readfile(fp, buffer->data, buffer->size);
    nilio_closefile(fp);
    return buffer;
}

NilBuffer* nilbuffer_create(NilCell size, const Nil* nil) {
    NilBuffer* fb = nil_alloc(nil, sizeof(NilBuffer) + size + 1);
    fb->size = size;
    fb->data[size] = 0; // Always add null terminator
    return fb;
}

void nilbuffer_destroy(NilBuffer* self, Nil* nil) {
    nil_free(nil, self, sizeof(NilBuffer) + self->size + 1);
}
