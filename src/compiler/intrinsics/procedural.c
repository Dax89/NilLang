#include "procedural.h"
#include "../../stack.h"
#include "../../types.h"
#include "../../vm/memory.h"
#include "../bytecode.h"
#include "../common.h"

void nilintrinsic_begin(Nil* self) {
    NilCompileInfo* nci = nilcstack_push(self, NCI_BEGIN);
    nci->loop.begin = self->codeoff;
}

void nilintrinsic_end(Nil* self) {
    if(!nilcstack_size(self)) nilcompiler_error(self, "unexpected END");

    NilCompileInfo nci = nilcstack_pop(self);

    if(nci.type == NCI_WORD) {
        nilop_emit_ret(self, false);
        nilop_patch_jump(self, nci.word.jmp_reloc, self->codeoff);
        self->c.depth--;
    }
    else if(nci.type == NCI_COND)
        nilop_patch_jump(self, nci.cond.jump, self->codeoff);
    else
        nilcompiler_internalerror("invalid END");
}

void nilintrinsic_word(Nil* self) {
    if(self->c.depth) nilcompiler_error(self, "FUNC must be used at top-level");

    nilcompiler_advance(self);
    nilcompiler_checkoverride(self, self->c.current.value,
                              self->c.current.length);

    NilCompileInfo* nci = nilcstack_push(self, NCI_WORD);
    self->c.depth++;
    nci->word.inarglist = false;
    nci->word.jmp_reloc = nilop_emit_jmp(self);

    nci->word.entry = nilmemory_allocentry(self, 5, self->c.current.value,
                                           self->c.current.length);

    // pfa format = entry, nargs, nlocals, argnames..., localsnames...
    nci->word.entry->cfa = NCFA_INTERPRET;
    nci->word.entry->pfa[NPFA_ENTRY] = self->codeoff;
    nci->word.entry->pfa[NPFA_NARGS] = 0;
    nci->word.entry->pfa[NPFA_NLOCALS] = 0;
    nci->word.entry->pfa[NPFA_ARGS] = 0;
    nci->word.entry->pfa[NPFA_LOCALS] = 0;
    nilop_emit(self, NILOP_ENTER);
}

void nilintrinsic_beginargs(Nil* self) {
    if(!nilcstack_size(self))
        nilcompiler_error(self, "cannot open an arg-list at top level");

    NilCompileInfo* nci = nilcstack_top(self);

    if(nci->type != NCI_WORD) {
        nilcompiler_error(self,
                          "cannot open arg-list outside a function definition");
    }

    nci->word.inarglist = true;
}

void nilintrinsic_endargs(Nil* self) {
    if(!nilcstack_size(self))
        nilcompiler_error(self, "cannot close an arg-list at top level");

    NilCompileInfo* nci = nilcstack_top(self);

    if(nci->type != NCI_WORD) {
        nilcompiler_error(
            self, "cannot close arg-list outside a function definition");
    }

    nci->word.inarglist = false;
}
