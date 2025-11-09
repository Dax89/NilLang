#pragma once

#include "../types.h"

// clang-format off
void* nilmemory_alloc(Nil* self, NilCell n);
NilEntry* nilmemory_allocentry(Nil* self, NilCell ncells, const char* name, NilCell n);
NilCell nilmemory_tocell(const Nil* self, const void* p);
void* nilmemory_fromcell(Nil* self, NilCell cell);
// clang-format on
