#include "heap.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

#define NILHEAP_SMALLEST_DATA sizeof(NilCell)
#define NILHEAP_SMALLEST_NODE (sizeof(NilHeapNode) + NILHEAP_SMALLEST_DATA)
#define NILHEAP_DATA_SIZE 1024

#define NILHEAP_NODE(x)                                                        \
    ((NilHeapNode*)((char*)(x) - offsetof(NilHeapNode, data)))

typedef struct NilHeapNode {
    struct NilHeapNode* prev;
    struct NilHeapNode* next;
    intptr_t size; // < 0 = free
    char data[];
} NilHeapNode;

static inline bool _nilheapnode_isfree(const NilHeapNode* self) {
    return self->size < 0;
}

static inline NilCell _nilheapnode_size(const NilHeapNode* self) {
    return self->size < 0 ? -self->size : self->size;
}

static inline bool _nilheapnode_iscontiguous(const NilHeapNode* self,
                                             const NilHeapNode* b) {
    return (char*)self + sizeof(NilHeapNode) + _nilheapnode_size(self) ==
           (char*)b;
}

NilHeap* nilheap_create(NilCell n, Nil* nil) {
    NilHeap* self = nil_alloc(nil, sizeof(NilHeap) + n);
    self->size = n;
    self->current = (NilHeapNode*)self->data;
    self->current->prev = self->current;
    self->current->next = self->current;
    self->current->size = -(n - sizeof(NilHeapNode));
    return self;
}

void nilheap_destroy(NilHeap* self, Nil* nil) {
    nil_free(nil, self, sizeof(NilHeap) + self->size);
}

void* nilheap_alloc(NilHeap* self, NilCell n) {
    if(n < NILHEAP_SMALLEST_DATA) n = NILHEAP_SMALLEST_DATA;
    n = NIL_ALIGNUP(n, sizeof(NilCell));

    NilHeapNode* node = self->current;

    do {
        if(_nilheapnode_isfree(node) && (NilCell)(-node->size) >= n) {
            NilCell leftover = -node->size - n;

            if(leftover >= NILHEAP_SMALLEST_NODE) // Split the block
            {
                NilHeapNode* slice = (NilHeapNode*)(node->data + n);
                slice->size = -leftover;
                slice->prev = node;
                slice->next = node->next;
                slice->next->prev = slice;

                node->next = slice;
            }

            node->size = n; // Mark as used
            self->current = node->next;
            return node->data;
        }

        node = node->next;
    } while(node != self->current);

    return NULL;
}

void* nilheap_alloc0(NilHeap* self, NilCell n) {
    void* p = nilheap_alloc(self, n);
    if(p) memset(p, 0, NILHEAP_NODE(p)->size);
    return p;
}

void* nilheap_realloc(NilHeap* self, void* ptr, NilCell n) {
    if(!ptr) return nilheap_alloc(self, n); // Just alloc

    NilHeapNode* node = NILHEAP_NODE(ptr);
    if(n <= _nilheapnode_size(node)) return ptr; // Fits in current block

    // Try to expand into next node if free and contiguous
    NilHeapNode* next = node->next;

    if(_nilheapnode_isfree(next) && _nilheapnode_iscontiguous(node, next)) {
        NilCell total = _nilheapnode_size(node) + sizeof(NilHeapNode) +
                        _nilheapnode_size(next);

        if(total >= n) { // Merge node + next
            node->size = n;
            NilCell leftover = total - n;

            if(leftover >= NILHEAP_SMALLEST_NODE) { // Create leftover node
                NilHeapNode* slice = (NilHeapNode*)(node->data + n);
                slice->size = -leftover;
                slice->prev = node;
                slice->next = next->next;
                slice->next->prev = slice;
                node->next = slice;
            }
            else {
                node->size = total; // Use entire merged space
                node->next = next->next;
                next->next->prev = node;
            }

            return ptr;
        }
    }

    // Fallback: allocate new block
    NilCell oldsize = _nilheapnode_size(node);
    void* newptr = nilheap_alloc(self, n);

    if(newptr) {
        memcpy(newptr, ptr, oldsize);
        nilheap_free(self, ptr);
    }

    return newptr;
}

void* nilheap_realloc0(NilHeap* self, void* ptr, NilCell n) {
    NilHeapNode* node = ptr ? NILHEAP_NODE(ptr) : NULL;
    NilCell oldsize = node ? _nilheapnode_size(node) : 0;

    void* newptr = nilheap_realloc(self, ptr, n);
    if(newptr && n > oldsize) memset((char*)newptr + oldsize, 0, n - oldsize);
    return newptr;
}

void nilheap_free(NilHeap* self, void* ptr) {

    NilHeapNode* node = NILHEAP_NODE(ptr);
    node->size = -node->size; // Mark as free

    NilHeapNode* prev = node->prev;

    if(_nilheapnode_isfree(prev) && _nilheapnode_iscontiguous(prev, node)) {
        prev->size = -(_nilheapnode_size(prev) + sizeof(NilHeapNode) +
                       _nilheapnode_size(node));
        prev->next = node->next;
        node->next->prev = prev;
        node = prev;
    }

    NilHeapNode* next = node->next;

    if(_nilheapnode_isfree(next) && _nilheapnode_iscontiguous(node, next)) {
        node->size = -(_nilheapnode_size(node) + sizeof(NilHeapNode) +
                       _nilheapnode_size(next));
        node->next = next->next;
        next->next->prev = node;
    }

    self->current = node;
}
