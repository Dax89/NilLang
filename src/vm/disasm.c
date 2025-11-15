#include "disasm.h"
#include "../compiler/bytecode.h"
#include "../types.h"
#include "memory.h"
#include "vm.h"
#include <stdarg.h>
#include <stdio.h>

static void _nilvm_print(NilCell addr, NilCell depth, const char* fmt, ...) {
    printf("%08" NIL_CELLXFMT " ", addr);

    for(NilCell i = 0; i < depth; i++) // Indent
        printf("  ");

    va_list args;
    va_start(args, fmt);
    vfprintf(stdout, fmt, args); // forward all arguments
    va_end(args);
}

const char* _nilvm_entryname(Nil* self) {
    NilEntry* e = nilmemory_fromcell(self, self->latest);
    NilCell ep = self->vm.ip - 1;

    while(e) {
        if(e->cfa == NCFA_INTERPRET && e->pfa[NPFA_ENTRY] == ep)
            return nilmemory_fromcell(self, e->name);

        e = nilmemory_fromcell(self, e->link);
    }

    return "???";
}

bool nilvm_disasm(Nil* self) {
    NilCell depth = 0;
    NilCell oldip = self->vm.ip;
    self->vm.ip = 0;

    while(self->vm.ip < self->codeoff) {
        NilCell addr = self->vm.ip;
        NilOpCode op = (NilOpCode)nilvm_readbyte(self);

        switch(op) {
            case NILOP_ADD: _nilvm_print(addr, depth, "add\n"); break;
            case NILOP_SUB: _nilvm_print(addr, depth, "sub\n"); break;
            case NILOP_MUL: _nilvm_print(addr, depth, "mul\n"); break;
            case NILOP_DIV: _nilvm_print(addr, depth, "div\n"); break;
            case NILOP_AND: _nilvm_print(addr, depth, "and\n"); break;
            case NILOP_OR: _nilvm_print(addr, depth, "or\n"); break;
            case NILOP_XOR: _nilvm_print(addr, depth, "xor\n"); break;
            case NILOP_NOT: _nilvm_print(addr, depth, "not\n"); break;
            case NILOP_SWAP: _nilvm_print(addr, depth, "swap\n"); break;
            case NILOP_OVER: _nilvm_print(addr, depth, "over\n"); break;
            case NILOP_ROT: _nilvm_print(addr, depth, "rot\n"); break;
            case NILOP_DUP: _nilvm_print(addr, depth, "dup\n"); break;
            case NILOP_LT: _nilvm_print(addr, depth, "lt\n"); break;
            case NILOP_EQ: _nilvm_print(addr, depth, "eq\n"); break;
            case NILOP_NOP: _nilvm_print(addr, depth, "nop\n"); break;
            case NILOP_EMIT: _nilvm_print(addr, depth, "emit\n"); break;
            case NILOP_FETCH: _nilvm_print(addr, depth, "fetch\n"); break;
            case NILOP_STORE: _nilvm_print(addr, depth, "store\n"); break;
            case NILOP_CFETCH: _nilvm_print(addr, depth, "c.fetch\n"); break;
            case NILOP_CSTORE: _nilvm_print(addr, depth, "c.store\n"); break;

            case NILOP_ALOAD:
                _nilvm_print(addr, depth, "a.load #%" NIL_CELLFMT "\n",
                             nilvm_readuleb128(self));
                break;

            case NILOP_LLOAD:
                _nilvm_print(addr, depth, "l.load #%" NIL_CELLFMT "\n",
                             nilvm_readuleb128(self));
                break;

            case NILOP_LOAD:
                _nilvm_print(addr, depth, "load 0x%" NIL_CELLXFMT "\n",
                             nilvm_readuleb128(self));
                break;

            case NILOP_PUSH:
                _nilvm_print(addr, depth, "push 0x%" NIL_CELLXFMT "\n",
                             nilvm_readuleb128(self));
                break;

            case NILOP_POP: _nilvm_print(addr, depth, "pop\n"); break;

            case NILOP_JMP:
                _nilvm_print(addr, depth, "jmp #%" NIL_JUMPXFMT "\n",
                             nilvm_readjump(self));
                break;

            case NILOP_JNT:
                _nilvm_print(addr, depth, "jnt #%" NIL_JUMPXFMT "\n",
                             nilvm_readjump(self));
                break;

            case NILOP_ENTER:
                _nilvm_print(addr, ++depth, "enter (%s)\n",
                             _nilvm_entryname(self));
                break;

            case NILOP_CALL: {
                const NilEntry* e =
                    nilmemory_fromcell(self, nilvm_readuleb128(self));

                if(e) {
                    const char* name = nilmemory_fromcell(self, e->name);
                    _nilvm_print(addr, depth, "call <%s>\n", name);
                }
                else
                    _nilvm_print(addr, depth, "call ???\n");

                break;
            }

            case NILOP_RET: {
                _nilvm_print(addr, depth, "ret\n");
                if(depth > 0) depth--;
                break;
            }

            default:
                _nilvm_print(addr, depth, "??? %02X\n", op);
                self->vm.ip = oldip;
                return false;
        }
    }

    self->vm.ip = oldip;
    return true;
}
