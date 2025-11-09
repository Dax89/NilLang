#pragma once

#include <inttypes.h>
#include <stdint.h>

#ifdef __cplusplus
#define NIL_EXTERN_C extern "C"
#else
#define NIL_EXTERN_C
#endif

#if defined _MSC_VER // Defined by Visual Studio
#define NIL_IMPORT NIL_EXTERN_C __declspec(dllimport)
#define NIL_EXPORT NIL_EXTERN_C __declspec(dllexport)
#else
#if __GNUC__ >= 4 // Defined by GNU C Compiler. Also for C++
#define NIL_IMPORT NIL_EXTERN_C __attribute__((visibility("default")))
#define NIL_EXPORT NIL_EXTERN_C __attribute__((visibility("default")))
#else
#define NIL_IMPORT NIL_EXTERN_C
#define NIL_EXPORT NIL_EXTERN_C
#endif
#endif

#if !defined(NIL_NORETURN)
#if defined(__GNUC__) || defined(__clang__)
#define NIL_NORETURN __attribute__((noreturn))
#elif defined(_MSC_VER)
#define NIL_NORETURN __declspec(noreturn)
#else
#define NIL_NORETURN
#endif /* defined(__GNUC__) || defined(__clang__) */
#endif /* !defined(NIL_NORETURN) */

// -----------------------------------------------------------------------------
// WARNING:
// Changing the types of NilCell or NilJump can have far-reaching effects:
//   1. NilCell is used for all stack cells, frame fields, program counter (IP),
//      and memory offsets. Changing its size affects the entire VM layout.
//   2. NilJump is used for absolute jump addresses in bytecode.
//      Changing its size affects bytecode encoding, serialization, and jump
//      range.
// -----------------------------------------------------------------------------
#if !defined(NIL_CELLTYPE)
#define NIL_CELLTYPE uintptr_t
#endif

#if !defined(NIL_JUMPTYPE)
#define NIL_JUMPTYPE uint32_t
#endif

// Heap memory
#if !defined(NIL_HEAP_SIZE)
#define NIL_HEAP_SIZE (NilCell)(1024 * 1024)
#endif

// Code memory
#if !defined(NIL_CODE_SIZE)
#define NIL_CODE_SIZE 16384
#endif

// Data memory
#if !defined(NIL_DATA_SIZE)
#define NIL_DATA_SIZE 32768
#endif

// Strings memory (interned)
#if !defined(NIL_STRINGS_SIZE)
#define NIL_STRINGS_SIZE 16384
#endif

// Data Stack
#if !defined(NIL_DSTACK_CELLS)
#define NIL_DSTACK_CELLS 1024
#endif

// Word Stack
#if !defined(NIL_WSTACK_CELLS)
#define NIL_WSTACK_CELLS 1024
#endif

// Compilation Stack
#if !defined(NIL_CSTACK_CELLS)
#define NIL_CSTACK_CELLS 32
#endif

// How many exit-points (eg. 'break') in a loop
#if !defined(NIL_LOOP_BREAKS)
#define NIL_LOOP_EXITS 32
#endif

// Format macros for printf.
// These correspond to the current NilCell / NilJump type
#if !defined(NIL_CELLFMT)
#define NIL_CELLFMT PRIdPTR
#define NIL_CELLXFMT PRIxPTR
#endif

#if !defined(NIL_JUMPFMT)
#define NIL_JUMPFMT PRId32
#define NIL_JUMPXFMT PRIx32
#endif
