#pragma once

#include <nil/nil.h>
#include <stdint.h>

typedef struct NilHeapNode NilHeapNode;
typedef struct Nil Nil;

typedef struct NilHeap {
    NilHeapNode* current;
    NilCell size;
    char data[];
} NilHeap;

NilHeap* nilheap_create(NilCell n, Nil* nil);
void nilheap_destroy(NilHeap* self, Nil* nil);
void* nilheap_alloc(NilHeap* self, NilCell n);
void* nilheap_alloc0(NilHeap* self, NilCell n);
void* nilheap_realloc(NilHeap* self, void* ptr, NilCell n);
void* nilheap_realloc0(NilHeap* self, void* ptr, NilCell n);
void nilheap_free(NilHeap* self, void* ptr);
