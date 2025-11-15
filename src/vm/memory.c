#include "memory.h"
#include "../stringtable.h"
#include "../utils.h"
#include <string.h>

static void _nilmemory_checkaddress(const Nil* self, const void* p) {
    if((const char*)p < (const char*)self ||
       (const char*)p >= (const char*)self + sizeof(Nil) + NIL_MEMORY_SIZE)
        nil_error("unknown CELL address %" PRIxPTR, p);
}

void* nilmemory_alloc(Nil* self, NilCell nbytes) {
    nbytes = NIL_ALIGNUP(nbytes, sizeof(NilCell));

    if(self->dataoff + nbytes >= NIL_DATA_END)
        nil_error("data-memory exausted");

    void* ptr = self->memory + self->dataoff;
    self->dataoff += nbytes;
    memset(ptr, 0, nbytes); // Cleanup memory
    return ptr;
}

NilEntry* nilmemory_allocentry(Nil* self, NilCell ncells, const char* name,
                               NilCell n) {
    NilEntry* entry =
        nilmemory_alloc(self, sizeof(NilEntry) + (ncells * sizeof(NilCell)));
    entry->link = self->latest;
    entry->name = nilstringtable_intern(self, name, n);
    self->latest = nilmemory_tocell(self, entry);
    return entry;
}

NilCell nilmemory_tocell(const Nil* self, const void* p) {
    if(!p) return 0;

    _nilmemory_checkaddress(self, p);
    return (NilCell)((const char*)p - (const char*)self);
}

void* nilmemory_fromcell(Nil* self, NilCell cell) {
    if(!cell) return 0;

    char* p = ((char*)self + cell);
    _nilmemory_checkaddress(self, p);
    return p;
}
