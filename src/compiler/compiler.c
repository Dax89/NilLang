#include "compiler.h"
#include "../stringtable.h"
#include "../utils.h"
#include "../vm/memory.h"
#include "bytecode.h"
#include "common.h"
#include "intrinsics/intrinsics.h"

static void _nilcompiler_int(Nil* self) {
    NilCell v = str_toint(self->c.current.value, self->c.current.length);
    nilop_emit_push(self, v);
}

static void _nilcompiler_str(Nil* self) {
    // String leading and trailing \"
    NilCell addr = nilstringtable_intern(self, self->c.current.value + 1,
                                         self->c.current.length - 2);
    nilop_emit_load(self, addr);
    nilop_emit_push(self, self->c.current.length - 2);
}

static bool _nilcompiler_argvar(Nil* self, NilCompileInfo* nci) {
    if(self->c.current.type == NILT_ROUNDCLOSE) return false;
    nilcompiler_definelocal(self, nci, 1, NPFA_NARGS, NPFA_ARGS);
    nilcompiler_advance(self);
    return true;
}

static bool _nilcompiler_localload(Nil* self, const NilCompileInfo* nci) {
    bool isarg = false;
    NilCell frameidx = nilcompileinfo_frameindex(
        nci, self, self->c.current.value, self->c.current.length, &isarg);

    if(frameidx != NIL_WSTACK_CELLS) {
        nilop_emit_lload(self, frameidx);
        if(isarg) nilop_emit(self, NILOP_FETCH); // Dereference arguments
        nilcompiler_advance(self);
        return true;
    }

    return false;
}

static bool _nilcompiler_identifier(Nil* self) {
    NilCompileInfo* nci = nilcstack_closest(self, NCI_WORD);

    if(nci) {
        if(nci->word.inarglist) return _nilcompiler_argvar(self, nci);
        if(_nilcompiler_localload(self, nci)) return true;
    }

    // ...look for word entry...
    const NilEntry* e = nilcompiler_findentry(self, self->c.current.value,
                                              self->c.current.length);

    if(e) {
        nilcompiler_advance(self);

        if(e->cfa)
            nilop_emit_call(self, nilmemory_tocell(self, e));
        else
            nilop_emit_load(self, nilmemory_tocell(self, &e->pfa));

        return true;
    }

    return false;
}

bool nilcompiler_compile(Nil* self, const char* source, bool ret) {
    self->c.depth = 0;
    nillexer_init(&self->c.lexer, source);
    nilcompiler_advance(self);

    while(!nilcompiler_check(self, NILT_EOF)) {
        if(_nilcompiler_identifier(self)) continue;

        switch(self->c.current.type) {
            case NILT_INT: _nilcompiler_int(self); break;
            case NILT_STR: _nilcompiler_str(self); break;

            default: {
                NilFunction f = nilintrinsic_find(self->c.current.value,
                                                  self->c.current.length);

                if(f)
                    f(self);
                else
                    nilcompiler_error(self, "undefined WORD");

                break;
            }
        }

        nilcompiler_advance(self);
    }

    if(ret) nilop_emit(self, NILOP_RET);

    if(nilcstack_size(self)) {
        const NilCompileInfo* nci = nilcstack_top(self);

        if(nci->type == NCI_WORD)
            nilcompiler_errorat(&nci->token, "unterminated WORD");
        else if(nci->type == NCI_BEGIN)
            nilcompiler_errorat(&nci->token, "unterminated BEGIN");
        else if(nci->type == NCI_COND)
            nilcompiler_errorat(&nci->token, "unterminated IF/UNLESS");
        else
            nilcompiler_internalerror("compiler-stack is not empty");
    }

    if(self->c.depth) nilcompiler_internalerror("invalid top-level");
    return true;
}
