#include "stack.h"
#include "types.h"
#include "utils.h"
#include "vm/memory.h"
#include <stdio.h>
#include <stdlib.h>

#define NIL_DEFINE_STACK_OPS(id, sp, N)                                        \
    NilCell nil##id##_size(const Nil* self) { return self->vm.sp; }            \
    void nil##id##_push(Nil* self, NilCell v) {                                \
        _nilstack_push(self->vm.id, N, &self->vm.sp, v);                       \
    }                                                                          \
    void nil##id##_dup(Nil* self) {                                            \
        _nilstack_dup(self->vm.id, N, &self->vm.sp);                           \
    }                                                                          \
    void nil##id##_drop(Nil* self, NilCell c) {                                \
        _nilstack_drop(&self->vm.sp, c);                                       \
    }                                                                          \
    void nil##id##_swap(Nil* self) {                                           \
        _nilstack_swap(self->vm.id, self->vm.sp);                              \
    }                                                                          \
    void nil##id##_over(Nil* self) {                                           \
        _nilstack_over(self->vm.id, N, &self->vm.sp);                          \
    }                                                                          \
    void nil##id##_rot(Nil* self) {                                            \
        _nilstack_rot(self->vm.id, N, &self->vm.sp);                           \
    }                                                                          \
    NilCell nil##id##_top(const Nil* self) {                                   \
        return _nilstack_top(self->vm.id, self->vm.sp);                        \
    }                                                                          \
    NilCell nil##id##_pop(Nil* self) {                                         \
        return _nilstack_pop(self->vm.id, &self->vm.sp);                       \
    }                                                                          \
    NilCell nil##id##_get(Nil* self, NilCell idx) {                            \
        return _nilstack_get(self->vm.id, self->vm.sp, idx);                   \
    }                                                                          \
    NilCell* nil##id##_reserve(Nil* self, NilCell c) {                         \
        return _nilstack_reserve(self->vm.id, N, &self->vm.sp, c);             \
    }

static NIL_NORETURN void _nil_stackerror(const char* msg) {
    fprintf(stderr, "STACK ERROR: %s\n", msg);
    exit(3);
}

static void _nilstack_push(NilCell* stack, NilCell n, NilCell* sp, NilCell v) {
    if(*sp >= n) _nil_stackerror("stack overflow (push)");
    stack[(*sp)++] = v;
}

static NilCell _nilstack_pop(NilCell* stack, NilCell* sp) {
    if(*sp == 0) _nil_stackerror("stack underflow (pop)");
    return stack[--(*sp)];
}

static NilCell _nilstack_top(const NilCell* stack, NilCell sp) {
    return (sp > 0) ? stack[sp - 1] : 0;
}

static void _nilstack_dup(NilCell* stack, NilCell n, NilCell* sp) {
    _nilstack_push(stack, n, sp, _nilstack_top(stack, *sp));
}

static void _nilstack_drop(NilCell* sp, NilCell c) {
    if(c > *sp) _nil_stackerror("stack underflow (drop)");
    *sp -= c;
}

static void _nilstack_swap(NilCell* stack, NilCell sp) {
    if(sp < 2) _nil_stackerror("stack underflow (swap)");
    NilCell a = stack[sp - 1];
    NilCell b = stack[sp - 2];
    stack[sp - 1] = b;
    stack[sp - 2] = a;
}

static void _nilstack_over(NilCell* stack, NilCell n, NilCell* sp) {
    if(*sp < 2) _nil_stackerror("stack underflow (over)");
    NilCell v = stack[*sp - 2];
    _nilstack_push(stack, n, sp, v);
}

static void _nilstack_rot(NilCell* stack, NilCell n, NilCell* sp) {
    if(*sp < 3) _nil_stackerror("stack underflow (rot)");
    NilCell a = _nilstack_pop(stack, sp);
    NilCell b = _nilstack_pop(stack, sp);
    NilCell c = _nilstack_pop(stack, sp);
    _nilstack_push(stack, n, sp, b);
    _nilstack_push(stack, n, sp, a);
    _nilstack_push(stack, n, sp, c);
}

static NilCell _nilstack_get(const NilCell* stack, NilCell sp, NilCell idx) {
    if(idx >= sp) _nil_stackerror("stack overflow (get)");
    return stack[idx];
}

static NilCell* _nilstack_reserve(NilCell* stack, NilCell n, NilCell* sp,
                                  NilCell c) {
    if(*sp + c >= n) _nil_stackerror("stack underflow (reserve)");
    *sp += c;
    return stack + (*sp - c);
}

NIL_DEFINE_STACK_OPS(dstack, dsp, NIL_DSTACK_CELLS)
NIL_DEFINE_STACK_OPS(wstack, wsp, NIL_WSTACK_CELLS)

NilCompileInfo* nilcstack_push(Nil* self, NilCompileType t) {
    if(self->c.sp + 1 >= NIL_CSTACK_CELLS)
        _nil_stackerror("compile-stack overflow");

    NilCompileInfo* nci = &self->c.stack[self->c.sp++];
    nci->type = t;
    nci->token = self->c.current;
    return nci;
}

NilCompileInfo nilcstack_pop(Nil* self) {
    if(self->c.sp == 0) _nil_stackerror("compile-stack underflow");
    return self->c.stack[--self->c.sp];
}

NilCompileInfo* nilcstack_top(Nil* self) {
    if(!self->c.sp) return NULL;
    return &self->c.stack[self->c.sp - 1];
}

NilCompileInfo* nilcstack_closest(Nil* self, NilCompileType t) {
    for(int i = self->c.sp; i-- > 0;) {
        if(self->c.stack[i].type == t) return &self->c.stack[i];
    }

    return NULL;
}

NilCell nilcstack_size(Nil* self) { return self->c.sp; }

bool nilcompileinfo_addexit(NilCompileInfo* self, NilCell e) {
    if(self->loop.n_exits + 1 >= NIL_LOOP_EXITS) return false;
    self->loop.exits[self->loop.n_exits++] = e;
    return true;
}

NilFrameIndex nilcompileinfo_frameindex(const NilCompileInfo* self, Nil* nil,
                                        const char* name, NilCell n) {
    if(self->type != NCI_WORD)
        return (NilFrameIndex){.value = NIL_WSTACK_CELLS};

    NilCell nargs = self->word.entry->pfa[NPFA_NARGS];

    // Check locals
    const NilLocalEntry* le =
        nilmemory_fromcell(nil, self->word.entry->pfa[NPFA_LOCALS]);
    NilCell localidx = 0;

    while(le) {
        const char* argname = nilmemory_fromcell(nil, le->name);

        if(argname && str_iequals_n(name, n, argname))
            return (NilFrameIndex){.value = nargs + localidx, .isarg = false};

        le = nilmemory_fromcell(nil, le->link);
        localidx++;
    }

    // Check arguments
    le = nilmemory_fromcell(nil, self->word.entry->pfa[NPFA_ARGS]);
    NilCell argidx = nargs - 1;

    while(le) {
        const char* argname = nilmemory_fromcell(nil, le->name);

        if(argname && str_iequals_n(name, n, argname))
            return (NilFrameIndex){.value = argidx, .isarg = true};

        le = nilmemory_fromcell(nil, le->link);
        argidx--;
    }

    return (NilFrameIndex){.value = NIL_WSTACK_CELLS};
}
