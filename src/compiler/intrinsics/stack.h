#pragma once

typedef struct Nil Nil;

void nilintrinsic_pop(Nil* self);
void nilintrinsic_dup(Nil* self);
void nilintrinsic_over(Nil* self);
void nilintrinsic_swap(Nil* self);
void nilintrinsic_rot(Nil* self);
void nilintrinsic_nip(Nil* self);
void nilintrinsic_tuck(Nil* self);
