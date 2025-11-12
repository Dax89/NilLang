#include "vm.h"
#include "../compiler/bytecode.h"
#include "../compiler/common.h"
#include "../stack.h"
#include "memory.h"
#include "runtime.h"
#include <assert.h>

static void _nilvm_checkaddress(const Nil* nil, const void* ptr) {
    const char* p = (const char*)ptr;

    if(p >= nil->memory && p < nil->memory + NIL_MEMORY_SIZE) return;

    if(p >= nil->vm.heap->data && p < nil->vm.heap->data + NIL_HEAP_SIZE)
        return;

    const char* pstack = (const char*)nil->vm.wstack;
    if(p >= pstack && p < pstack + (NIL_WSTACK_CELLS * sizeof(NilCell))) return;

    nil_error("unknown MEMORY address %" PRIxPTR, ptr);
}

static void _nilvm_binary(Nil* self, NilOpCode op) {
    NilCell op2 = nildstack_pop(self);
    NilCell op1 = nildstack_pop(self);
    NilCell res;

    switch(op) {
        case NILOP_ADD: res = op1 + op2; break;
        case NILOP_SUB: res = op1 - op2; break;
        case NILOP_MUL: res = op1 * op2; break;

        case NILOP_DIV: {
            if(!op2) nil_error("division by zero");
            res = op1 / op2;
            break;
        }

        case NILOP_AND: res = op1 & op2; break;
        case NILOP_OR: res = op1 | op2; break;
        case NILOP_XOR: res = op1 ^ op2; break;
        case NILOP_LT: res = op1 < op2 ? NIL_TRUE : NIL_FALSE; break;
        case NILOP_EQ: res = op1 == op2 ? NIL_TRUE : NIL_FALSE; break;
        default: nilcompiler_internalerror("invalid binary-operation"); break;
    }

    nildstack_push(self, res);
}

static void _nilvm_enterframe(Nil* self) {
    assert(self->vm.word);
    nilwstack_push(self, self->vm.fp);
    self->vm.fp = self->vm.wsp;

    NilCell nargcells = self->vm.word->pfa[NPFA_NARGS];
    NilCell* base = nilwstack_reserve(self, nargcells);

    // Move args to word stacks in reversed order
    for(NilCell i = nargcells; i-- > 0;)
        base[i] = nildstack_pop(self);

    // Reserve for locals too
    NilCell nlocalcells = self->vm.word->pfa[NPFA_NLOCALS];
    base = nilwstack_reserve(self, nlocalcells);
}

static void _nilvm_exitframe(Nil* self) {
    assert(self->vm.word);
    NilCell nargs = self->vm.word->pfa[NPFA_NARGS];
    NilCell nlocals = self->vm.word->pfa[NPFA_NLOCALS];
    nilwstack_drop(self, nlocals);
    nilwstack_drop(self, nargs);

    self->vm.fp = nilwstack_pop(self);
    self->vm.word = (const NilEntry*)nilwstack_pop(self);
    self->vm.ip = nilwstack_pop(self);
}

static void _nilvm_call(Nil* self) {
    NilCell addr = nilvm_readuleb128(self);
    const NilEntry* e = nilmemory_fromcell(self, addr);

    switch(e->cfa) {
        case NCFA_MEMORY:
            nil_error("'%s' is not callable",
                      nilmemory_fromcell(self, e->name));
            break;

        case NCFA_INTERPRET:
            nilwstack_push(self, self->vm.ip);
            nilwstack_push(self, (NilCell)self->vm.word);
            self->vm.ip = e->pfa[NPFA_ENTRY];
            self->vm.word = e;
            break;

        case NCFA_FUNCTION: ((NilFunction)e->pfa[0])(self); break;
        default: nilruntime_getep(e->cfa)(self); break;
    }
}

NilCell nilvm_readuleb128(Nil* self) {
    NilCell result = 0, shift = 0;
    unsigned char byte;

    do {
        byte = nilvm_readbyte(self);
        result |= ((NilCell)(byte & 0x7F) << shift);
        shift += 7;
    } while(byte & 0x80);

    return result;
}

NilJump nilvm_readjump(Nil* self) {
    NilJump value = 0;
    for(unsigned int i = 0; i < sizeof(NilJump); i++)
        value |= ((NilJump)nilvm_readbyte(self)) << (i * 8);

    return value;
}

unsigned char nilvm_readbyte(Nil* self) { return self->memory[self->vm.ip++]; }

bool nilvm_run(Nil* self) {
    for(;;) {
        NilOpCode op = (NilOpCode)nilvm_readbyte(self);

        switch(op) {
            case NILOP_ADD:
            case NILOP_SUB:
            case NILOP_MUL:
            case NILOP_DIV:
            case NILOP_AND:
            case NILOP_OR:
            case NILOP_XOR:
            case NILOP_LT:
            case NILOP_EQ: _nilvm_binary(self, op); break;

            case NILOP_NOT: nildstack_push(self, ~nildstack_pop(self)); break;

            case NILOP_ALOAD: {
                NilCell idx = self->vm.fp + nilvm_readuleb128(self);
                nildstack_push(self, self->vm.wstack[idx]);
                break;
            }

            case NILOP_LLOAD: {
                NilCell idx = self->vm.fp + nilvm_readuleb128(self);
                nildstack_push(self, (NilCell)&self->vm.wstack[idx]);
                break;
            }

            case NILOP_LOAD: {
                const void* ptr =
                    nilmemory_fromcell(self, nilvm_readuleb128(self));
                nildstack_push(self, (NilCell)ptr);
                break;
            }

            case NILOP_FETCH: {
                const NilCell* ptr = (const NilCell*)nildstack_pop(self);
                _nilvm_checkaddress(self, ptr);
                nildstack_push(self, *ptr);
                break;
            }

            case NILOP_STORE: {
                NilCell* ptr = (NilCell*)nildstack_pop(self);
                _nilvm_checkaddress(self, ptr);
                *ptr = nil_pop(self);
                break;
            }

            case NILOP_CFETCH: {
                const unsigned char* ptr =
                    (const unsigned char*)nildstack_pop(self);
                _nilvm_checkaddress(self, ptr);
                nildstack_push(self, *ptr);
                break;
            }

            case NILOP_CSTORE: {
                unsigned char* ptr = (unsigned char*)nildstack_pop(self);
                _nilvm_checkaddress(self, ptr);
                *ptr = (unsigned char)nil_pop(self);
                break;
            }

            case NILOP_PUSH:
                nildstack_push(self, nilvm_readuleb128(self));
                break;

            case NILOP_POP: nildstack_pop(self); break;
            case NILOP_DUP: nildstack_dup(self); break;
            case NILOP_OVER: nildstack_over(self); break;
            case NILOP_SWAP: nildstack_swap(self); break;
            case NILOP_ROT: nildstack_rot(self); break;
            case NILOP_ENTER: _nilvm_enterframe(self); break;
            case NILOP_CALL: _nilvm_call(self); break;

            case NILOP_RET: {
                if(self->vm.word) {
                    _nilvm_exitframe(self);
                    break;
                }

                return true;
            }

            case NILOP_JNT: {
                NilCell cond = nildstack_pop(self);
                NilJump dest = nilvm_readjump(self);
                if(!cond) self->vm.ip = (NilCell)dest;
                break;
            }

            case NILOP_JMP: self->vm.ip = (NilCell)nilvm_readjump(self); break;
            default: nilcompiler_internalerror("unknown opcode"); break;
        }
    }

    return false;
}
