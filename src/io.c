#include "io.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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

NilResult nilio_fileopen(const char* filepath, NilIOMode mode) {
    NilFile fp = fopen(filepath, _nilio_modestr(mode));
    return (NilResult){.file = fp, .err = errno};
}

NilResult nilio_filesize(NilFile fp) {
    long oldp = ftell(fp);
    if(oldp == -1L) return (NilResult){.err = errno};

    fseek(fp, 0, SEEK_END);

    long sz = ftell(fp);
    if(sz == -1L) return (NilResult){.err = errno};

    fseek(fp, oldp, SEEK_SET);
    return (NilResult){.value = sz, .err = NIL_SUCCESS};
}

NilResult nilio_filepos(NilFile fp) {
    long pos = ftell(fp);
    return (NilResult){.value = pos, .err = pos == -1L ? errno : NIL_SUCCESS};
}

NilResult nilio_filewrite(NilFile fp, const char* data, NilCell n) {
    NilCell nw = fwrite(data, 1, n, fp);
    return (NilResult){.value = nw, .err = nw == n ? NIL_SUCCESS : NIL_FAIL};
}

NilResult nilio_filewriteln(NilFile fp, const char* data, NilCell n) {
    NilResult res = nilio_filewrite(fp, data, n);
    if(nilresult_ok(&res)) return nilio_filewrite(fp, "\n", sizeof("\n") - 1);
    return res;
}

NilResult nilio_fileread(NilFile fp, char* data, NilCell n) {
    NilCell nr = fread(data, sizeof(char), n, fp);
    return (NilResult){.value = n, .err = nr == n ? NIL_SUCCESS : NIL_FAIL};
}

NilResult nilio_filereadln(NilFile fp, char* data, NilCell n) {
    char* res = fgets(data, n, fp);
    if(res) return (NilResult){.value = strlen(data), .err = NIL_SUCCESS};
    if(feof(fp)) return (NilResult){.value = 0, .err = NIL_SUCCESS};
    return (NilResult){.value = 0, .err = ferror(fp)};
}

void nilio_fileclose(NilFile fp) { fclose(fp); }

NilResult nilio_fileslurp(const Nil* nil, const char* filepath) {
    NilResult fp = nilio_fileopen(filepath, NIO_R);
    if(!nilresult_ok(&fp)) return fp;

    NilResult size = nilio_filesize(fp.file);
    if(!nilresult_ok(&size)) return size;

    NilBuffer* buffer = nilbuffer_create(size.value, nil);
    nilio_fileread(fp.file, buffer->data, buffer->size);
    nilio_fileclose(fp.file);
    return (NilResult){.buffer = buffer, .err = NIL_SUCCESS};
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
