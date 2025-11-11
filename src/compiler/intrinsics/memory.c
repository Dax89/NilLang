#include "memory.h"
#include "../../types.h"
#include "../../utils.h"
#include "../../vm/memory.h"
#include "../bytecode.h"
#include "../common.h"

static void _nilintrinsic_alloclocal(Nil* self, NilCell ncells) {
    NilCompileInfo* nci = nilcstack_closest(self, NCI_WORD);

    if(!nci) {
        nilcompiler_internalerror(
            "cannot allocate local variables at top level");
    }

    if(nci->type != NCI_WORD) {
        nilcompiler_internalerror(
            "cannot allocate local variables outside WORDs");
    }

    if(nci->word.inarglist) {
        nilcompiler_error(self,
                          "cannot allocate local variables inside arg-list");
    }

    nci->word.entry->pfa[NPFA_NLOCALS] += ncells;
}

static void _nilintrinsic_createlocal(Nil* self, NilCell ncells) {
    NilCompileInfo* nci = nilcstack_closest(self, NCI_WORD);

    if(!nci) {
        nilcompiler_internalerror(
            "cannot declare local variables at top level");
    }

    if(nci->type != NCI_WORD) {
        nilcompiler_internalerror(
            "cannot declare local variables outside WORDs");
    }

    if(nci->word.inarglist) {
        nilcompiler_error(self,
                          "cannot declare local variables inside arg-list");
    }

    nilcompiler_definelocal(self, nci, ncells, NPFA_NLOCALS, NPFA_LOCALS);
}

static void _nilintrinsic_create(Nil* self, NilCell ncells) {
    nilcompiler_advance(self);

    if(self->c.depth) {
        _nilintrinsic_createlocal(self, ncells);
        return;
    }

    nilcompiler_checkoverride(self, self->c.current.value,
                              self->c.current.length);

    nilmemory_allocentry(self, ncells, self->c.current.value,
                         self->c.current.length);
}

void nilintrinsic_create(Nil* self) { _nilintrinsic_create(self, 0); }
void nilintrinsic_var(Nil* self) { _nilintrinsic_create(self, 1); }
void nilintrinsic_pair(Nil* self) { _nilintrinsic_create(self, 2); }
void nilintrinsic_here(Nil* self) { nilop_emit_load(self, self->dataoff); }
void nilintrinsic_fetch(Nil* self) { nilop_emit(self, NILOP_FETCH); }
void nilintrinsic_store(Nil* self) { nilop_emit(self, NILOP_STORE); }
void nilintrinsic_cfetch(Nil* self) { nilop_emit(self, NILOP_CFETCH); }
void nilintrinsic_cstore(Nil* self) { nilop_emit(self, NILOP_CSTORE); }

void nilintrinsic_cells(Nil* self) {
    nilcompiler_advance(self);

    NilCell ncells = 0;

    if(nilcompiler_check(self, NILT_INT))
        ncells = str_toint(self->c.current.value, self->c.current.length);
    else
        nilcompiler_error(self, "expected INT cells count");

    if(self->c.depth)
        _nilintrinsic_alloclocal(self, ncells);
    else
        nilmemory_alloc(self, ncells);
}

void nilintrinsic_bytes(Nil* self) {
    nilcompiler_advance(self);

    NilCell nbytes = 0;

    if(nilcompiler_check(self, NILT_INT))
        nbytes = str_toint(self->c.current.value, self->c.current.length);
    else
        nilcompiler_error(self, "expected INT bytes count");

    if(self->c.depth) {
        _nilintrinsic_alloclocal(self, NIL_ALIGNUP(nbytes, sizeof(NilCell)) /
                                           sizeof(NilCell));
    }
    else
        nilmemory_alloc(self, nbytes);
}

void nilintrinsic_unused(Nil* self) {
    nilop_emit_load(self, NIL_DATA_END - self->dataoff);
}
