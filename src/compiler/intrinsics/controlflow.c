#include "controlflow.h"
#include "../../stack.h"
#include "../../types.h"
#include "../bytecode.h"
#include "../common.h"

static void _nilintrinsic_patchexits(Nil* self, const NilCompileInfo* nci) {
    for(NilCell i = 0; i < nci->loop.n_exits; i++)
        nilop_patch_jump(self, nci->loop.exits[i], self->codeoff);
}

void nilintrinsic_repeat(Nil* self) {
    if(!nilcstack_size(self)) nilcompiler_error(self, "unexpected REPEAT");

    NilCompileInfo nci = nilcstack_pop(self);

    if(nci.type == NCI_BEGIN) {
        NilCell r = nilop_emit_jmp(self);
        nilop_patch_jump(self, r, nci.loop.begin);
        _nilintrinsic_patchexits(self, &nci);
        return;
    }

    nilcompiler_error(self, "REPEAT without BEGIN");
}

void nilintrinsic_break(Nil* self) {
    NilCompileInfo* nci = nilcstack_closest(self, NCI_BEGIN);
    if(!nci) nilcompiler_error(self, "BREAK without BEGIN");

    NilCell r = nilop_emit_jmp(self);

    if(!nilcompileinfo_addexit(nci, r))
        nilcompiler_error(self, "too many BREAKs");
}

void nilintrinsic_continue(Nil* self) {
    NilCompileInfo* nci = nilcstack_closest(self, NCI_BEGIN);
    if(!nilcstack_size(self)) nilcompiler_error(self, "CONTINUE without BEGIN");

    NilCell r = nilop_emit_jmp(self);
    nilop_patch_jump(self, r, nci->loop.begin);
}

void nilintrinsic_return(Nil* self) { nilop_emit(self, NILOP_RET); }
