#include "conditions.h"
#include "../../types.h"
#include "../bytecode.h"
#include "../common.h"

void nilintrinsic_if(Nil* self) {
    NilCompileInfo* nci = nilcstack_push(self, NCI_COND);
    nci->cond.jump = nilop_emit_jnt(self);
    nci->cond.end_jump = 0;
}

void nilintrinsic_unless(Nil* self) {
    nilop_emit(self, NILOP_NOT);
    nilintrinsic_if(self);
}

void nilintrinsic_else(Nil* self) {
    if(!nilcstack_size(self)) nilcompiler_error(self, "unexpected ELSE");

    NilCompileInfo* nci = nilcstack_top(self);

    if(nci->type != NCI_COND)
        nilcompiler_error(self, "ELSE without matching block");

    // jump to skip ELSE
    NilCell rskip = nilop_emit_jmp(self);
    // patch previous conditional
    nilop_patch_jump(self, nci->cond.jump, self->codeoff);
    // now jump points to end of ELSE
    nci->cond.jump = rskip;
}
