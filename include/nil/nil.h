#pragma once

#include <nil/config.h>
#include <stdbool.h>

typedef NIL_CELLTYPE NilCell;
typedef NIL_JUMPTYPE NilJump;

typedef struct Nil Nil;

typedef void* (*NilAllocator)(void* ctx, void* ptr, int osize, int nsize);
typedef void (*NilFunction)(Nil* nil);

// clang-format off
NIL_EXPORT Nil* nil_create(void);
NIL_EXPORT Nil* nil_create_ex(NilAllocator alloc, void* ctx);
NIL_EXPORT void nil_destroy(Nil* self);
NIL_EXPORT NilAllocator nil_getallocator(const Nil* self, void** ctx);
NIL_EXPORT void* nil_alloc(const Nil* self, int size);
NIL_EXPORT void* nil_realloc(const Nil* self, void* ptr, int osize, int nsize);
NIL_EXPORT void nil_free(const Nil* self, void* ptr, int size);
NIL_EXPORT bool nil_include(Nil* self, const char* filepath);
NIL_EXPORT bool nil_loadfile(Nil* self, const char* filepath);
NIL_EXPORT bool nil_loadstring(Nil* self, const char* source);
NIL_EXPORT bool nil_disasm(Nil* self);
NIL_EXPORT bool nil_run(Nil* self);
NIL_EXPORT bool nil_register(Nil* self, const char* name, NilFunction f);
NIL_EXPORT NilCell nil_pop(Nil* self);
NIL_EXPORT NilCell nil_top(const Nil* self);
NIL_EXPORT NilCell nil_size(const Nil* self);
NIL_EXPORT void nil_push(Nil* self, NilCell v);
NIL_EXPORT void nil_swap(Nil* self);
NIL_EXPORT void nil_dup(Nil* self);
NIL_EXPORT void nil_over(Nil* self);
NIL_EXPORT NIL_NORETURN void nil_error(const char* fmt, ...);
// clang-format on

#define nil_runstring(self, filepath)                                          \
    (nil_loadstring(self, filepath) && nil_run(self))

#define nil_runfile(self, filepath)                                            \
    (nil_loadfile(self, filepath) && nil_run(self))
