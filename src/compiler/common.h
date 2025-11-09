#pragma once

#include "../lexer/token.h"
#include "../types.h"

typedef struct NilToken NilToken;
typedef struct NilEntry NilEntry;

NIL_NORETURN void nilcompiler_errorat(const NilToken* token, const char* msg);
NIL_NORETURN void nilcompiler_error(Nil* self, const char* msg);
NIL_NORETURN void nilcompiler_internalerror(const char* msg);
void nilcompiler_warningat(const NilToken* token, const char* msg);
void nilcompiler_warning(Nil* self, const char* msg);
NilEntry* nilcompiler_findentry(Nil* self, const char* name, NilCell n);
bool nilcompiler_check(const Nil* self, NilTokenType type);
void nilcompiler_checkoverride(Nil* self, const char* name, NilCell n);
void nilcompiler_advance(Nil* self);
void nilcompiler_definelocal(Nil* self, NilCompileInfo* nci, NilCell ncells,
                             NilCell counteridx, NilCell listidx);
