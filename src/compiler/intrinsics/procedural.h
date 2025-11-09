#pragma once

typedef struct Nil Nil;

void nilintrinsic_begin(Nil* self);
void nilintrinsic_end(Nil* self);
void nilintrinsic_word(Nil* self);
void nilintrinsic_beginargs(Nil* self);
void nilintrinsic_endargs(Nil* self);
