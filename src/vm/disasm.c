#include "disasm.h"
#include "../compiler/bytecode.h"
#include "../types.h"
#include "memory.h"
#include "vm.h"
#include <stdio.h>

static void _nilvm_printaddr(Nil* self, NilCell depth) {
    printf("%08" NIL_CELLXFMT " ", self->vm.ip);

    for(NilCell i = 0; i < depth; i++) // Indent
        printf("  ");
}

bool nilvm_disasm(Nil* self) {
    NilCell depth = 0;
    NilCell oldip = self->vm.ip;
    self->vm.ip = 0;

    while(self->vm.ip < self->codeoff) {
        _nilvm_printaddr(self, depth);
        NilOpCode op = (NilOpCode)nilvm_readbyte(self);

        switch(op) {
            case NILOP_ADD: printf("add\n"); break;
            case NILOP_SUB: printf("sub\n"); break;
            case NILOP_MUL: printf("mul\n"); break;
            case NILOP_DIV: printf("div\n"); break;
            case NILOP_AND: printf("and\n"); break;
            case NILOP_OR: printf("or\n"); break;
            case NILOP_XOR: printf("xor\n"); break;
            case NILOP_NOT: printf("not\n"); break;
            case NILOP_SWAP: printf("swap\n"); break;
            case NILOP_OVER: printf("over\n"); break;
            case NILOP_ROT: printf("rot\n"); break;
            case NILOP_DUP: printf("dup\n"); break;
            case NILOP_LT: printf("lt\n"); break;
            case NILOP_EQ: printf("eq\n"); break;
            case NILOP_NOP: printf("nop\n"); break;
            case NILOP_EMIT: printf("emit\n"); break;
            case NILOP_FETCH: printf("fetch\n"); break;
            case NILOP_STORE: printf("store\n"); break;
            case NILOP_CFETCH: printf("c.fetch\n"); break;
            case NILOP_CSTORE: printf("c.store\n"); break;

            case NILOP_ALOAD:
                printf("a.load #%" NIL_CELLFMT "\n", nilvm_readuleb128(self));
                break;

            case NILOP_LLOAD:
                printf("l.load #%" NIL_CELLFMT "\n", nilvm_readuleb128(self));
                break;

            case NILOP_LOAD:
                printf("load 0x%" NIL_CELLXFMT "\n", nilvm_readuleb128(self));
                break;

            case NILOP_PUSH:
                printf("push 0x%" NIL_CELLXFMT "\n", nilvm_readuleb128(self));
                break;

            case NILOP_POP:
                printf("pop %" NIL_CELLXFMT "\n", nilvm_readuleb128(self));
                break;

            case NILOP_JMP:
                printf("jmp #%" NIL_JUMPXFMT "\n", nilvm_readjump(self));
                break;

            case NILOP_JNT:
                printf("jnt #%" NIL_JUMPXFMT "\n", nilvm_readjump(self));
                break;

            case NILOP_ENTER:
                depth++;
                printf("enter\n");
                break;

            case NILOP_CALL: {
                const NilEntry* e =
                    nilmemory_fromcell(self, nilvm_readuleb128(self));

                if(e) {
                    const char* name = nilmemory_fromcell(self, e->name);
                    printf("call <%s>\n", name);
                }
                else
                    printf("call ???\n");

                break;
            }

            case NILOP_RET: {
                printf("ret\n");
                if(depth > 0) depth--;
                break;
            }

            default:
                printf("??? %02X\n", op);
                self->vm.ip = oldip;
                return false;
        }
    }

    self->vm.ip = oldip;
    return true;
}
