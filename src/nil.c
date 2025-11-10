#include "compiler/compiler.h"
#include "compiler/intrinsics/intrinsics.h"
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
    self->c.sp = 0;
    self->vm.heap = nilheap_create(NIL_HEAP_SIZE, self);

    nilstringtable_init(self);
    nilintrinsics_init();
    nilruntime_register(self);
    return self;
}

void nil_destroy(Nil* self) {
    if(!self) return;
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

bool nil_loadfile(Nil* self, const char* filepath) {
    if(!filepath) return false;

    int sz = 0;
    char* source = nilp_readfile(self, filepath, &sz);
    if(!source) return false;

    bool res = nil_loadstring(self, source);
    nil_free(self, source, sz);
    return res;
}

bool nil_loadstring(Nil* self, const char* source) {
    if(!source) return false;
    return nilcompiler_compile(self, source);
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
