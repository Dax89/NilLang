#include "bytecode.h"
#include "../types.h"

#define NIL_PLACEHOLDER ((NilJump)0)

static void _nilop_writecode(Nil* self, char b) {
    if(self->codeoff + 1 >= NIL_CODE_SIZE) nil_error("code-memory exausted");
    self->memory[self->codeoff++] = b;
}

static void _nilop_write_jump(Nil* self, NilJump addr) {
    for(unsigned int i = 0; i < sizeof(NilJump); i++)
        _nilop_writecode(self, (char)((addr >> (i * 8)) & 0xFF));
}

static void _nilop_write_uleb128(Nil* self, NilCell v) {
    do {
        unsigned char byte = v & 0x7F;
        v >>= 7;
        if(v != 0) byte |= 0x80;
        _nilop_writecode(self, byte);
    } while(v);
}

void nilop_patch_jump(Nil* self, NilCell r, NilJump addr) {
    if(r + sizeof(NilJump) >= NIL_CODE_SIZE)
        nil_error("invalid jmp location (patch)");

    for(unsigned int i = 0; i < sizeof(NilJump); i++)
        self->memory[r + i] = (char)((addr >> (i * 8)) & 0xFF);
}

void nilop_emit(Nil* self, NilOpCode op) { _nilop_writecode(self, op); }

void nilop_emit_push(Nil* self, NilCell v) {
    nilop_emit(self, NILOP_PUSH);
    _nilop_write_uleb128(self, v);
}

void nilop_emit_load(Nil* self, NilCell addr) {
    nilop_emit(self, NILOP_LOAD);
    _nilop_write_uleb128(self, addr);
}

void nilop_emit_aload(Nil* self, NilCell stackidx) {
    nilop_emit(self, NILOP_ALOAD);
    _nilop_write_uleb128(self, stackidx);
}

void nilop_emit_lload(Nil* self, NilCell stackidx) {
    nilop_emit(self, NILOP_LLOAD);
    _nilop_write_uleb128(self, stackidx);
}

void nilop_emit_call(Nil* self, NilCell entry) {
    nilop_emit(self, NILOP_CALL);
    _nilop_write_uleb128(self, entry);
}

NilCell nilop_emit_jnt(Nil* self) {
    nilop_emit(self, NILOP_JNT);
    NilCell r = self->codeoff;
    _nilop_write_jump(self, NIL_PLACEHOLDER);
    return r;
}

NilCell nilop_emit_jmp(Nil* self) {
    nilop_emit(self, NILOP_JMP);
    NilCell r = self->codeoff;
    _nilop_write_jump(self, NIL_PLACEHOLDER);
    return r;
}
