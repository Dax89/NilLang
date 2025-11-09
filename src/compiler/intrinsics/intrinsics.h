#pragma once

#include <nil/nil.h>

typedef struct NilToken NilToken;

void nilintrinsics_init(void);
NilFunction nilintrinsic_find(const char* name, NilCell n);
