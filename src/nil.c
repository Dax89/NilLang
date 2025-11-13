#include "compiler/compiler.h"
#include "compiler/intrinsics/intrinsics.h"
#include "io.h"
#include "stack.h"
#include "stringtable.h"
#include "types.h"
#include "utils.h"
#include "vm/disasm.h"
#include "vm/memory.h"
#include "vm/runtime.h"
#include "vm/vm.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void* _nil_allocator(void* ctx, void* ptr, int osize, int nsize) {
    (void)ctx;

    if(nsize == 0) {
        free(ptr);
        return NULL;
    }

    void* p = malloc(nsize);

    if(!p) {
        abort();
        return NULL;
    }

    if(ptr) {
        memcpy(p, ptr, osize);
        free(ptr);
    }

    return p;
}

static bool _nil_compilestring(Nil* self, const char* source, bool ret) {
    if(!source) return false;
    return nilcompiler_compile(self, source, ret);
}

static bool _nil_compilefile(Nil* self, const char* filepath, bool ret) {
    if(!filepath) return false;

    NilBuffer* source = nilio_slurpfile(self, filepath);
    if(!source) return false;
    bool ok = _nil_compilestring(self, source->data, ret);
    nilbuffer_destroy(source, self);
    return ok;
}

Nil* nil_create(void) { return nil_create_ex(NULL, NULL); }

Nil* nil_create_ex(NilAllocator alloc, void* ctx) {
    if(!alloc) alloc = _nil_allocator;

    Nil* self = alloc(ctx, NULL, 0, sizeof(Nil) + NIL_MEMORY_SIZE);
    memset(self, 0, sizeof(Nil) + NIL_MEMORY_SIZE);
    self->alloc = alloc;
    self->ctx = ctx;
    self->latest = 0;
    self->codeoff = 0;
    self->dataoff = NIL_CODE_SIZE;
    self->vm.ip = 0;
    self->vm.fp = 0;
    self->vm.dsp = 0;
    self->vm.wsp = 0;
    self->vm.dstack = nil_alloc(self, sizeof(NilCell) * NIL_DSTACK_CELLS);
    self->vm.wstack = nil_alloc(self, sizeof(NilCell) * NIL_WSTACK_CELLS);
    self->c.sp = 0;
    self->c.stack = nil_alloc(self, sizeof(NilCompileInfo) * NIL_CSTACK_CELLS);
    self->vm.heap = nilheap_create(NIL_HEAP_SIZE, self);

    memset(self->vm.dstack, 0, sizeof(NilCell) * NIL_DSTACK_CELLS);
    memset(self->vm.wstack, 0, sizeof(NilCell) * NIL_WSTACK_CELLS);
    memset(self->c.stack, 0, sizeof(NilCompileInfo) * NIL_CSTACK_CELLS);

    nilstringtable_init(self);
    nilintrinsics_init();
    nilruntime_register(self);
    return self;
}

void nil_destroy(Nil* self) {
    if(!self) return;

    self->vm.dsp = 0;
    self->vm.wsp = 0;
    self->c.sp = 0;

    nil_free(self, self->vm.dstack, sizeof(NilCell) * NIL_DSTACK_CELLS);
    nil_free(self, self->vm.wstack, sizeof(NilCell) * NIL_WSTACK_CELLS);
    nil_free(self, self->c.stack, sizeof(NilCompileInfo) * NIL_CSTACK_CELLS);
    nilheap_destroy(self->vm.heap, self);
    nil_free(self, self, sizeof(Nil));
}

NilAllocator nil_getallocator(const Nil* self, void** ctx) {
    if(!self) return NULL;
    if(ctx) *ctx = self->ctx;
    return self->alloc;
}

void* nil_alloc(const Nil* self, int size) {
    if(self && size) {
        NilCell aligned = NIL_ALIGNUP(size, sizeof(NilCell));
        return self->alloc(self->ctx, NULL, 0, aligned);
    }

    return NULL;
}

void* nil_realloc(const Nil* self, void* ptr, int osize, int nsize) {
    if(self) {
        NilCell aligned = NIL_ALIGNUP(nsize, sizeof(NilCell));
        return self->alloc(self->ctx, ptr, osize, aligned);
    }

    return NULL;
}

void nil_free(const Nil* self, void* ptr, int size) {
    if(self && ptr) {
        NilCell aligned = NIL_ALIGNUP(size, sizeof(NilCell));
        self->alloc(self->ctx, ptr, aligned, 0);
    }
}

void nil_savecore(const Nil* self, const char* filepath) {
    NilFile fp = nilio_openfile(filepath, NIO_W);

    NilCore core = {
        .config =
            {
                .cellsize = sizeof(NilCell),
                .jmpsize = sizeof(NilJump),
                .codesize = NIL_CODE_SIZE,
                .datasize = NIL_DATA_SIZE,
                .strsize = NIL_STRINGS_SIZE,
            },
        .latest = self->latest,
        .codeoff = self->codeoff,
        .dataoff = self->dataoff,
        .stroff = self->stroff,
    };

    nilio_writefile(fp, (const char*)&core, sizeof(NilCore));
    nilio_writefile(fp, self->memory, NIL_MEMORY_SIZE);
    nilio_closefile(fp);
}

void nil_loadcore(Nil* self, const char* filepath) {
    NilFile fp = nilio_openfile(filepath, NIO_R);

    NilCore core;
    nilio_readfile(fp, (char*)&core, sizeof(NilCore));

    if(core.config.cellsize != sizeof(NilCell)) {
        fprintf(stderr, "incompatible CELL size\n");
        exit(-3);
    }

    if(core.config.jmpsize != sizeof(NilJump)) {
        fprintf(stderr, "incompatible JMP size\n");
        exit(-3);
    }

    if(core.config.codesize != NIL_CODE_SIZE) {
        fprintf(stderr, "incompatible CODE size\n");
        exit(-3);
    }

    if(core.config.datasize != NIL_DATA_SIZE) {
        fprintf(stderr, "incompatible DATA size\n");
        exit(-3);
    }

    if(core.config.strsize != NIL_STRINGS_SIZE) {
        fprintf(stderr, "incompatible STRINGS size\n");
        exit(-3);
    }

    nilio_readfile(fp, self->memory, NIL_MEMORY_SIZE);
    nilio_closefile(fp);
}

bool nil_include(Nil* self, const char* filepath) {
    return _nil_compilefile(self, filepath, false);
}

bool nil_loadfile(Nil* self, const char* filepath) {
    return _nil_compilefile(self, filepath, true);
}

bool nil_loadstring(Nil* self, const char* source) {
    return _nil_compilestring(self, source, true);
}

bool nil_disasm(Nil* self) { return nilvm_disasm(self); }
bool nil_run(Nil* self) { return nilvm_run(self); }

bool nil_register(Nil* self, const char* name, NilFunction f) {
    if(!self || !name || !f) return false;

    NilEntry* e = nilmemory_allocentry(self, 1, name, strlen(name));
    e->cfa = NCFA_FUNCTION;
    e->pfa[0] = (NilCell)f;
    return true;
}

NilCell nil_pop(Nil* self) { return nildstack_pop(self); }
NilCell nil_top(const Nil* self) { return nildstack_top(self); }
NilCell nil_size(const Nil* self) { return nildstack_size(self); }
void nil_push(Nil* self, NilCell v) { nildstack_push(self, v); }
void nil_swap(Nil* self) { nildstack_swap(self); }
void nil_dup(Nil* self) { nildstack_dup(self); }
void nil_over(Nil* self) { nildstack_over(self); }

void nil_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "RUNTIME ERROR: ");
    vfprintf(stderr, fmt, args); // forward all arguments
    fprintf(stderr, "\n");

    va_end(args);
    exit(1);
}
