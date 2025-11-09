#pragma once

#include <nil/nil.h>

unsigned char nilvm_readbyte(Nil* self);
NilCell nilvm_readuleb128(Nil* self);
NilJump nilvm_readjump(Nil* self);
bool nilvm_run(Nil* self);
