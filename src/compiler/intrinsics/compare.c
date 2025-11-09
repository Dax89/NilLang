#include "compare.h"
#include "../bytecode.h"

void nilintrinsic_eq(Nil* self) { nilop_emit(self, NILOP_EQ); }
void nilintrinsic_lt(Nil* self) { nilop_emit(self, NILOP_LT); }

void nilintrinsic_ne(Nil* self) {
    nilop_emit(self, NILOP_EQ);
    nilop_emit(self, NILOP_NOT);
}

void nilintrinsic_le(Nil* self) {
    nilop_emit(self, NILOP_SWAP);
    nilop_emit(self, NILOP_LT);
    nilop_emit(self, NILOP_NOT);
}

void nilintrinsic_gt(Nil* self) {
    nilop_emit(self, NILOP_SWAP);
    nilop_emit(self, NILOP_LT);
}

void nilintrinsic_ge(Nil* self) {
    nilop_emit(self, NILOP_LT);
    nilop_emit(self, NILOP_NOT);
}
