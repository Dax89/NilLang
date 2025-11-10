#include "runtime.h"
#include "../heap.h"
#include "../utils.h"
#include "memory.h"
#include <assert.h>
#include <stdio.h>

#define NIL_RUNTIME_ENTRY(n, ep)                                               \
    {.name = #n, .length = sizeof(#n) - 1, .entry = ep}

#define NIL_DUMPLINE 16

static void _nilruntime_checkaddress(const Nil* nil, const void* ptr) {
    const char* p = (const char*)ptr;

    if(p >= nil->memory && p < nil->memory + NIL_MEMORY_SIZE) return;

    if(p >= nil->vm.heap->data && p < nil->vm.heap->data + NIL_HEAP_SIZE)
        return;

    const char* pstack = (const char*)nil->vm.wstack;
    if(p >= pstack && p < pstack + (NIL_WSTACK_CELLS * sizeof(NilCell))) return;

    nil_error("unknown MEMORY address %" PRIxPTR, ptr);
}

static void nilruntime_popprint(Nil* nil) {
    printf("%" NIL_CELLFMT, nil_pop(nil));
}

static void nilruntime_printstack(Nil* nil) {
    printf("<%" NIL_CELLFMT "> (", nil->vm.dsp);

    for(NilCell i = 0; i < nil->vm.dsp; i++) {
        if(i) putchar(' ');
        printf("%" NIL_CELLFMT, nil->vm.dstack[i]);
    }

    printf(")");
}

static void nilruntime_dump(Nil* nil) {
    NilCell len = nil_pop(nil);
    unsigned char* p = (unsigned char*)(uintptr_t)nil_pop(nil);
    _nilruntime_checkaddress(nil, p);

    for(NilCell i = 0; i < len; i += NIL_DUMPLINE) {
        printf("%" PRIxPTR "  ", (uintptr_t)(p + i));

        // Hex part .
        for(NilCell j = 0; j < NIL_DUMPLINE; j++) {
            if(i + j < len)
                printf("%02x ", p[i + j]);
            else
                printf("   ");
        }

        printf(" ");

        // ASCII part
        for(NilCell j = 0; j < NIL_DUMPLINE && i + j < len; j++) {
            char c = p[i + j];
            printf("%c", (c >= 32 && c < 127) ? c : '.');
        }

        printf("\n");
    }
}

static void nilruntime_allot(Nil* nil) {
    if(nil->vm.fp) nil_error("Cannot allot inside WORDs");
    nilmemory_alloc(nil, nil_pop(nil));
}

static void nilruntime_allocate(Nil* nil) {
    NilCell n = nil_pop(nil);
    void* p = nilheap_alloc0(nil->vm.heap, n);
    if(!p) nil_error("cannot allocate %d bytes", n);
    nil_push(nil, (NilCell)p);
}

static void nilruntime_reallocate(Nil* nil) {
    NilCell n = nil_pop(nil);
    void* oldp = (void*)nil_pop(nil);
    void* p = nilheap_realloc0(nil->vm.heap, oldp, n);
    if(!p) nil_error("cannot reallocate %d bytes", n);
    nil_push(nil, (NilCell)p);
}

static void nilruntime_free(Nil* nil) {
    NilCell p = nil_pop(nil);
    if(p) nilheap_free(nil->vm.heap, (void*)p);
}

static void nilruntime_say(Nil* nil) {
    NilCell n = nil_pop(nil);
    const char* s = (const char*)nil_pop(nil);

    for(NilCell i = 0; i < n; i++)
        putchar(s[i]);
}

static void nilruntime_nl(Nil* nil) {
    NIL_UNUSED(nil);
    putchar('\n');
}

static void nilruntime_printcell(Nil* nil) {
    NilCell* v = (NilCell*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);

    printf("%" NIL_CELLFMT, *v);
}

static void nilruntime_loadcell1(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, *v);
}

static void nilruntime_storecell1(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    *v = nil_pop(nil);
}

static void nilruntime_loadcell2(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, v[0]);
    nil_push(nil, v[1]);
    nil_swap(nil); // Swap (for strings)
}

static void nilruntime_storecell2(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    v[0] = nil_pop(nil);
    v[1] = nil_pop(nil);
}

static void nilruntime_loadcelln(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, v[nil_pop(nil)]);
}

static void nilruntime_storecelln(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    v[nil_pop(nil)] = nil_pop(nil);
}

static void nilruntime_printchar(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    printf("%d", *v);
}

static void nilruntime_loadchar1(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, *v);
}

static void nilruntime_storechar1(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    *v = (char)nil_pop(nil);
}

static void nilruntime_loadchar2(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, (NilCell)v[0]);
    nil_push(nil, (NilCell)v[1]);
}

static void nilruntime_storechar2(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    v[0] = (char)nil_pop(nil);
    v[1] = (char)nil_pop(nil);
}

static void nilruntime_loadcharn(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    nil_push(nil, (NilCell)v[nil_pop(nil)]);
}

static void nilruntime_storecharn(Nil* nil) {
    unsigned char* v = (unsigned char*)nil_top(nil);
    _nilruntime_checkaddress(nil, v);
    v[nil_pop(nil)] = (char)nil_pop(nil);
}

static void nilruntime_incvar(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    *v += nil_pop(nil);
}

static void nilruntime_decvar(Nil* nil) {
    NilCell* v = (NilCell*)nil_pop(nil);
    _nilruntime_checkaddress(nil, v);
    *v -= nil_pop(nil);
}

static void nilruntime_cells(Nil* nil) {
    nil_push(nil, nil_pop(nil) * sizeof(NilCell));
}

typedef struct NilRuntimeEntry {
    const char* name;
    NilCell length;
    NilFunction entry;
} NilRuntimeEntry;

// clang-format off
static const NilRuntimeEntry NIL_RUNTIME[] = {
    {NULL}, // Reserved (Memory)
    {NULL}, // Reserved (Interpret)
    {NULL}, // Reserved (Function)
    NIL_RUNTIME_ENTRY(., nilruntime_popprint),
    NIL_RUNTIME_ENTRY(.s, nilruntime_printstack),
    NIL_RUNTIME_ENTRY(@, nilruntime_loadcell1),
    NIL_RUNTIME_ENTRY(!, nilruntime_storecell1),
    NIL_RUNTIME_ENTRY(2@, nilruntime_loadcell2),
    NIL_RUNTIME_ENTRY(2!, nilruntime_storecell2),
    NIL_RUNTIME_ENTRY(n@, nilruntime_loadcelln),
    NIL_RUNTIME_ENTRY(n!, nilruntime_storecelln),
    NIL_RUNTIME_ENTRY(?, nilruntime_printcell),
    NIL_RUNTIME_ENTRY(c@, nilruntime_loadchar1),
    NIL_RUNTIME_ENTRY(c!, nilruntime_storechar1),
    NIL_RUNTIME_ENTRY(2c@, nilruntime_loadchar2),
    NIL_RUNTIME_ENTRY(2c!, nilruntime_storechar2),
    NIL_RUNTIME_ENTRY(nc@, nilruntime_loadcharn),
    NIL_RUNTIME_ENTRY(nc!, nilruntime_storecharn),
    NIL_RUNTIME_ENTRY(c?, nilruntime_printchar),
    NIL_RUNTIME_ENTRY(+!, nilruntime_incvar),
    NIL_RUNTIME_ENTRY(-!, nilruntime_decvar),
    NIL_RUNTIME_ENTRY(dump, nilruntime_dump),
    NIL_RUNTIME_ENTRY(cells, nilruntime_cells),
    NIL_RUNTIME_ENTRY(allot, nilruntime_allot),
    NIL_RUNTIME_ENTRY(allocate, nilruntime_allocate),
    NIL_RUNTIME_ENTRY(reallocate, nilruntime_reallocate),
    NIL_RUNTIME_ENTRY(free, nilruntime_free),
    NIL_RUNTIME_ENTRY(say, nilruntime_say),
    NIL_RUNTIME_ENTRY(nl, nilruntime_nl),
};
// clang-format on

void nilruntime_register(Nil* nil) {
    const NilCell N = sizeof(NIL_RUNTIME) / sizeof(*NIL_RUNTIME);

    for(NilCell i = 0; i < N; i++) {
        const NilRuntimeEntry* re = &NIL_RUNTIME[i];
        if(!re->name) continue;

        NilEntry* e = nilmemory_allocentry(nil, 1, re->name, re->length);
        e->cfa = i;
    }
}

NilFunction nilruntime_getep(NilCell idx) {
    const NilCell N = sizeof(NIL_RUNTIME) / sizeof(*NIL_RUNTIME);
    if(idx >= N) nil_error("runtime index %d out of range", idx);
    const NilRuntimeEntry* re = &NIL_RUNTIME[idx];
    return re->entry;
}
