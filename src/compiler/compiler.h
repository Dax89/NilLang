#pragma once

#include <nil/nil.h>
#include <stdbool.h>

typedef struct Nil Nil;

void nilcompiler_init(void);
bool nilcompiler_compile(Nil* self, const char* source);
