#include "intrinsics.h"
#include "../../utils.h"
#include "../../vm/memory.h"
#include "../bytecode.h"
#include "compare.h"
#include "conditions.h"
#include "controlflow.h"
#include "math.h"
#include "memory.h"
#include "procedural.h"
#include "stack.h"
#include <stdlib.h>

static void nilintrinsic_base(Nil* nil) {
    nilop_emit_load(nil, nilmemory_tocell(nil, &nil->vm.base));
}

static void nilintrinsic_emit(Nil* nil) { nilop_emit(nil, NILOP_EMIT); }

typedef struct NilIntrinsic {
    const char* name;
    int length;
    NilFunction entry;
} NilIntrinsic;

#define NIL_INTRINSIC(x, f) {.name = (x), .length = sizeof(x) - 1, .entry = (f)}

// clang-format off
static NilIntrinsic nil_intrinsics[] = {
    // base
    NIL_INTRINSIC("base", nilintrinsic_base), NIL_INTRINSIC("emit", nilintrinsic_emit),

    // procedural
    NIL_INTRINSIC("begin", nilintrinsic_begin), NIL_INTRINSIC("end", nilintrinsic_end),
    NIL_INTRINSIC("word", nilintrinsic_word),
    NIL_INTRINSIC("[", nilintrinsic_beginargs), NIL_INTRINSIC("]", nilintrinsic_endargs),

    // memory
    NIL_INTRINSIC("@", nilintrinsic_fetch), NIL_INTRINSIC("!", nilintrinsic_store),
    NIL_INTRINSIC("c@", nilintrinsic_cfetch), NIL_INTRINSIC("c!", nilintrinsic_cstore),
    NIL_INTRINSIC("here", nilintrinsic_here), NIL_INTRINSIC("unused", nilintrinsic_unused), 
    NIL_INTRINSIC("create", nilintrinsic_create), NIL_INTRINSIC("var", nilintrinsic_var), 
    NIL_INTRINSIC("pair", nilintrinsic_pair), 
    NIL_INTRINSIC("bytes", nilintrinsic_bytes), NIL_INTRINSIC("cell", nilintrinsic_cell), 
    NIL_INTRINSIC("cells", nilintrinsic_cells),

    // stack
    NIL_INTRINSIC("drop", nilintrinsic_pop), NIL_INTRINSIC("dup", nilintrinsic_dup),
    NIL_INTRINSIC("over", nilintrinsic_over), NIL_INTRINSIC("swap", nilintrinsic_swap),
    NIL_INTRINSIC("rot", nilintrinsic_rot),
    NIL_INTRINSIC("nip", nilintrinsic_nip), NIL_INTRINSIC("tuck", nilintrinsic_tuck),

    // math
    NIL_INTRINSIC("+", nilintrinsic_add), NIL_INTRINSIC("-", nilintrinsic_sub), 
    NIL_INTRINSIC("*", nilintrinsic_mul), NIL_INTRINSIC("/", nilintrinsic_div), 
    NIL_INTRINSIC("and", nilintrinsic_and), NIL_INTRINSIC("or", nilintrinsic_or), 
    NIL_INTRINSIC("xor", nilintrinsic_xor), NIL_INTRINSIC("not", nilintrinsic_not), 

    // compare
    NIL_INTRINSIC("==", nilintrinsic_eq), NIL_INTRINSIC("<", nilintrinsic_lt), 
    NIL_INTRINSIC("<=", nilintrinsic_le), NIL_INTRINSIC("!=", nilintrinsic_ne),
    NIL_INTRINSIC(">", nilintrinsic_gt), NIL_INTRINSIC(">=", nilintrinsic_ge),

    // conditions
    NIL_INTRINSIC("if", nilintrinsic_if), NIL_INTRINSIC("unless", nilintrinsic_unless),
    NIL_INTRINSIC("else", nilintrinsic_else),

    // control flow
    NIL_INTRINSIC("repeat", nilintrinsic_repeat), NIL_INTRINSIC("return", nilintrinsic_return),
    NIL_INTRINSIC("break", nilintrinsic_break), NIL_INTRINSIC("continue", nilintrinsic_continue),
};
// clang-format on

const int NIL_NINTRINSICS = sizeof(nil_intrinsics) / sizeof(*nil_intrinsics);

static int _nilintrinsics_icmp(const void* a, const void* b) {
    const NilIntrinsic* itr1 = a;
    const NilIntrinsic* itr2 = b;

    // First compare lengths - saves time for most mismatches
    if(itr1->length != itr2->length)
        return (itr1->length < itr2->length) ? -1 : 1;

    // Compare case-insensitively for the same length
    for(int i = 0; i < itr1->length; i++) {
        char ch1 = str_toupper(itr1->name[i]);
        char ch2 = str_toupper(itr2->name[i]);
        if(ch1 != ch2) return ch1 - ch2;
    }

    return 0;
}

void nilintrinsics_init(void) {
    qsort(nil_intrinsics, NIL_NINTRINSICS, sizeof(*nil_intrinsics),
          _nilintrinsics_icmp);
}

NilFunction nilintrinsic_find(const char* name, NilCell n) {
    NilIntrinsic key = {.name = name, .length = n};

    const NilIntrinsic* itr =
        bsearch(&key, nil_intrinsics, NIL_NINTRINSICS, sizeof(*nil_intrinsics),
                _nilintrinsics_icmp);

    return itr ? itr->entry : NULL;
}
