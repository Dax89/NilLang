#pragma once

#include "../types.h"
#include <nil/nil.h>

void nilruntime_register(Nil* nil);
NilFunction nilruntime_getep(NilCell idx);
