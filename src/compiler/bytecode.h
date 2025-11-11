#pragma once

#include <nil/nil.h>

// clang-format off
typedef enum NilOpCode {
    NILOP_NOP = 0, // Unused
    NILOP_CALL, NILOP_ENTER, NILOP_RET,
    NILOP_LT, NILOP_EQ,
    NILOP_JNT, NILOP_JMP,
    NILOP_ADD, NILOP_SUB, NILOP_MUL, NILOP_DIV,
    NILOP_AND, NILOP_OR, NILOP_XOR, NILOP_NOT,
    NILOP_BAND, NILOP_BOR, NILOP_BXOR, NILOP_BNOT,
    NILOP_LLOAD, NILOP_LOAD,
    NILOP_PUSH, NILOP_POP, 
    NILOP_DUP, NILOP_SWAP, NILOP_OVER, NILOP_ROT,
} NilOpCode;
// clang-format on

typedef struct Nil Nil;

void nilop_patch_jump(Nil* nil, NilCell r, NilJump addr);

void nilop_emit(Nil* self, NilOpCode op);
void nilop_emit_push(Nil* self, NilCell v);
void nilop_emit_load(Nil* self, NilCell addr);
void nilop_emit_lload(Nil* self, NilCell stackidx);
void nilop_emit_call(Nil* self, NilCell entry);
NilCell nilop_emit_jnt(Nil* self);
NilCell nilop_emit_jmp(Nil* self);
