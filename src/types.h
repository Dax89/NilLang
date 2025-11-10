#pragma once

#include "heap.h"
#include "lexer/lexer.h"
#include "stack.h"
#include <nil/nil.h>

#define NIL_TRUE 1
#define NIL_FALSE 0
#define NIL_MEMORY_SIZE (NIL_CODE_SIZE + NIL_DATA_SIZE + NIL_STRINGS_SIZE)
#define NIL_CODE_END NIL_CODE_SIZE
#define NIL_DATA_END (NIL_CODE_SIZE + NIL_DATA_SIZE)
#define NIL_STRINGS_END NIL_MEMORY_SIZE
#define NIL_FIRSTARG 3 // First argument index in word frame

typedef enum NilCfaType {
    NCFA_MEMORY = 0,
    NCFA_INTERPRET,
    NCFA_FUNCTION,
} NilCfaType;

typedef enum NilPfaType {
    NPFA_ENTRY = 0,
    NPFA_NARGS,
    NPFA_NLOCALS,
    NPFA_ARGS,
    NPFA_LOCALS,
} NilPfaType;

typedef struct NilEntry {
    NilCell link;
    NilCell name;
    NilCell cfa;   // Code Field Address
    NilCell pfa[]; // Parameter Field Address
} NilEntry;

typedef struct NilLocalEntry {
    NilCell link;
    NilCell name;
} NilLocalEntry;

typedef struct Nil {
    NilAllocator alloc; // Allocator entry
    void* ctx;          // Allocator context

    struct {
        NilCell ip;  // Instruction pointer (pc)
        NilCell dsp; // Data Stack (top)
        NilCell wsp; // Word Stack (top)
        NilCell fp;  // Frame pointer (word stack bottom)
        NilHeap* heap;
        NilCell dstack[NIL_DSTACK_CELLS]; // Data Stack
        NilCell wstack[NIL_WSTACK_CELLS]; // Word Stack
        const NilEntry* word;             // Current Word (NULL = global scope)
    } vm;

    struct {
        int depth;                              // Scope depth
        NilLexer lexer;                         // Lexer
        NilToken previous;                      // Previous Token
        NilToken current;                       // Current Token
        NilCell sp;                             // Compiler Stack (top)
        NilCompileInfo stack[NIL_CSTACK_CELLS]; // Compiler stack
    } c;

    NilCell latest;  // Pointer to last entry
    NilCell codeoff; // Code offset (grows =>)
    NilCell dataoff; // Data offset (grows =>)
    NilCell stroff;  // Strings offset (grows =>)
    char memory[];   // VM Memory (Program Image)
} Nil;
