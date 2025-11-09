#pragma once

#include <nil/nil.h>

void nilstringtable_init(Nil* self);
NilCell nilstringtable_intern(Nil* self, const char* s1, NilCell n);
