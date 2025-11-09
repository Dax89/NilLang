#include "stringtable.h"
#include "utils.h"
#include "vm/memory.h"
#include <string.h>

#define NIL_STRINGTABLE_BUCKETS (1 << 8) // 256

typedef struct NilInternStr {
    NilCell link;
    NilCell data;
    NilCell length;
} NilInternStr;

// Use djb2 algorithm
static NilCell _nilstringtable_hash(const char* s, NilCell n) {
    NilCell h = 5381; // djb2 initial value

    while(n-- > 0)
        h = ((h << 5) + h) + (unsigned char)*s++; // h * 33 + current byte

    return h;
}

static void* _nilstringtable_alloc(Nil* self, NilCell n) {
    n = NIL_ALIGNUP(n, sizeof(NilCell));
    char* p = self->memory + NIL_DATA_END + self->stroff;

    if(p + n >= self->memory + NIL_STRINGS_END)
        nil_error("strings-memory exausted");

    self->stroff += n;
    return p;
}

static NilCell _nilstringtable_copy(Nil* self, const char* s, NilCell n) {
    char* res = _nilstringtable_alloc(self, n + 1);
    memcpy(res, s, n);
    res[n] = 0;
    return nilmemory_tocell(self, res);
}

void nilstringtable_init(Nil* self) {
    const NilCell BUCKETS_SIZE = sizeof(NilCell) * NIL_STRINGTABLE_BUCKETS;

    if(BUCKETS_SIZE >= NIL_STRINGS_SIZE) {
        nil_error("needs at least %" NIL_CELLFMT " bytes for string-table",
                  BUCKETS_SIZE);
    }

    char* pbuckets = _nilstringtable_alloc(self, BUCKETS_SIZE);
    memset(pbuckets, 0, BUCKETS_SIZE);
}

NilCell nilstringtable_intern(Nil* self, const char* s1, NilCell n) {
    NilCell* buckets = (NilCell*)(self->memory + NIL_DATA_END);
    NilCell h = _nilstringtable_hash(s1, n);
    NilCell idx = h & (NIL_STRINGTABLE_BUCKETS - 1);
    NilCell node = buckets[idx];

    while(node) {
        NilInternStr* entry = nilmemory_fromcell(self, node);

        if(entry->length == n) {
            const char* s2 = nilmemory_fromcell(self, entry->data);
            if(!memcmp(s1, s2, n)) return entry->data;
        }

        node = entry->link;
    }

    NilInternStr* nstr = _nilstringtable_alloc(self, sizeof(NilInternStr));
    nstr->link = node;
    nstr->data = _nilstringtable_copy(self, s1, n);
    nstr->length = n;

    buckets[idx] = nilmemory_tocell(self, nstr);
    return nstr->data;
}
