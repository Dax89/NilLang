#include "math.h"
#include "../bytecode.h"

void nilintrinsic_add(Nil* self) { nilop_emit(self, NILOP_ADD); }
void nilintrinsic_sub(Nil* self) { nilop_emit(self, NILOP_SUB); }
void nilintrinsic_mul(Nil* self) { nilop_emit(self, NILOP_MUL); }
void nilintrinsic_div(Nil* self) { nilop_emit(self, NILOP_DIV); }
void nilintrinsic_and(Nil* self) { nilop_emit(self, NILOP_AND); }
void nilintrinsic_or(Nil* self) { nilop_emit(self, NILOP_OR); }
void nilintrinsic_xor(Nil* self) { nilop_emit(self, NILOP_XOR); }
void nilintrinsic_not(Nil* self) { nilop_emit(self, NILOP_NOT); }
