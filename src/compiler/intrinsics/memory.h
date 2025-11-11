#pragma once

#include "../../types.h"

typedef struct Nil Nil;

void nilintrinsic_create(Nil* self);
void nilintrinsic_var(Nil* self);
void nilintrinsic_pair(Nil* self);
void nilintrinsic_fetch(Nil* self);
void nilintrinsic_store(Nil* self);
void nilintrinsic_cfetch(Nil* self);
void nilintrinsic_cstore(Nil* self);
void nilintrinsic_here(Nil* self);
void nilintrinsic_cells(Nil* self);
void nilintrinsic_unused(Nil* self);
void nilintrinsic_bytes(Nil* self);
