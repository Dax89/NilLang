#pragma once

#include "lexer/token.h"
#include <nil/nil.h>

#define NIL_DECLARE_STACK_OPS(id)                                              \
    void nil##id##_push(Nil* self, NilCell v);                                 \
    void nil##id##_dup(Nil* self);                                             \
    void nil##id##_drop(Nil* self, NilCell c);                                 \
    void nil##id##_swap(Nil* self);                                            \
    void nil##id##_over(Nil* self);                                            \
    void nil##id##_rot(Nil* self);                                             \
    NilCell nil##id##_size(const Nil* self);                                   \
    NilCell nil##id##_top(const Nil* self);                                    \
    NilCell nil##id##_pop(Nil* self);                                          \
    NilCell nil##id##_get(Nil* self, NilCell idx);                             \
    NilCell* nil##id##_reserve(Nil* self, NilCell c)

typedef struct NilStack {
    Nil* nil;
    const char* id;
    NilCell size;
    NilCell sp;
    NilCell data[];
} NilStack;

NIL_DECLARE_STACK_OPS(dstack);
NIL_DECLARE_STACK_OPS(wstack);

typedef enum NilCompileType {
    NCI_BEGIN = 0,
    NCI_COND,
    NCI_WORD,
} NilCompileType;

typedef struct NilEntry NilEntry;

typedef struct NilCompileInfo {
    NilCompileType type;
    NilToken token;

    union {
        struct {
            NilCell jump;
            NilCell end_jump;
        } cond;

        struct {
            NilCell begin;
            NilCell exits[NIL_LOOP_EXITS];
            NilCell n_exits;
        } loop;

        struct {
            bool inarglist;
            NilEntry* entry;
            NilCell jmp_reloc; // bypass-word jump reloc
        } word;
    };
} NilCompileInfo;

typedef struct NilFrameIndex {
    NilCell value;
    bool isarg;
} NilFrameIndex;

NilCompileInfo* nilcstack_push(Nil* self, NilCompileType t);
NilCompileInfo nilcstack_pop(Nil* self);
NilCompileInfo* nilcstack_top(Nil* self);
NilCompileInfo* nilcstack_closest(Nil* self, NilCompileType t);
NilCell nilcstack_size(Nil* self);

bool nilcompileinfo_addexit(NilCompileInfo* self, NilCell e);
NilFrameIndex nilcompileinfo_frameindex(const NilCompileInfo* self, Nil* nil,
                                        const char* name, NilCell n);
