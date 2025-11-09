#pragma once

typedef struct Nil Nil;

void nilintrinsic_add(Nil* self);
void nilintrinsic_sub(Nil* self);
void nilintrinsic_mul(Nil* self);
void nilintrinsic_div(Nil* self);
void nilintrinsic_and(Nil* self);
void nilintrinsic_or(Nil* self);
void nilintrinsic_xor(Nil* self);
void nilintrinsic_not(Nil* self);
void nilintrinsic_band(Nil* self);
void nilintrinsic_bor(Nil* self);
void nilintrinsic_bxor(Nil* self);
void nilintrinsic_bnot(Nil* self);
