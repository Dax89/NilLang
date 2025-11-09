#pragma once

#include <nil/nil.h>
#include <stdbool.h>

#define NIL_ALIGNUP(x, a) (((x) + ((a) - 1)) & ~((a) - 1))
#define NIL_UNUSED(x) ((void)(x))

NilCell uleb128_read(const char* p, NilCell n, NilCell* outsz);
char str_toupper(char s);
bool str_iequals(const char* s1, const char* s2);
bool str_iequals_n(const char* s1, int n, const char* s2);
NilCell str_toint(const char* s, int n);
double str_toreal(const char* s, int n);
char* nilp_readfile(const Nil* nil, const char* filepath, int* outsz);
