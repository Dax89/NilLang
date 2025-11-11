#include "stack.h"
#include "../bytecode.h"

void nilintrinsic_pop(Nil* self) { nilop_emit(self, NILOP_POP); }
void nilintrinsic_dup(Nil* self) { nilop_emit(self, NILOP_DUP); }
void nilintrinsic_over(Nil* self) { nilop_emit(self, NILOP_OVER); }
void nilintrinsic_swap(Nil* self) { nilop_emit(self, NILOP_SWAP); }
void nilintrinsic_rot(Nil* self) { nilop_emit(self, NILOP_ROT); }

void nilintrinsic_nip(Nil* self) {
    nilop_emit(self, NILOP_SWAP);
    nilop_emit(self, NILOP_POP);
}

void nilintrinsic_tuck(Nil* self) {
    nilop_emit(self, NILOP_SWAP);
    nilop_emit(self, NILOP_OVER);
}
