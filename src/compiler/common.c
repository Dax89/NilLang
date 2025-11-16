#include "common.h"
#include "../stringtable.h"
#include "../types.h"
#include "../utils.h"
#include "../vm/memory.h"
#include "intrinsics/intrinsics.h"
#include <stdio.h>
#include <stdlib.h>

static void _nilcompiler_report(const NilToken* token, const char* severity,
                                const char* msg) {
    fprintf(stderr, "%d:%d %s: %s\n", token->line + 1, token->col, severity,
            msg);

    switch(token->type) {
        case NILT_EOF: fprintf(stderr, "  Reached EOF\n"); break;

        default:
            fprintf(stderr, "  Near '%.*s'\n", token->length, token->value);
            break;
    }
}

void nilcompiler_errorat(const NilToken* token, const char* msg) {
    _nilcompiler_report(token, "ERROR", msg);
    exit(2);
}

void nilcompiler_internalerror(const char* msg) {
    fprintf(stderr, "INTERNAL ERROR: %s\n", msg);
    exit(2);
}

void nilcompiler_warningat(const NilToken* token, const char* msg) {
    _nilcompiler_report(token, "WARNING", msg);
}

void nilcompiler_warning(Nil* self, const char* msg) {
    nilcompiler_warningat(&self->c.current, msg);
}

void nilcompiler_error(Nil* self, const char* msg) {
    nilcompiler_errorat(&self->c.current, msg);
}

bool nilcompiler_check(const Nil* self, NilTokenType type) {
    return self->c.current.type == type;
}

void nilcompiler_advance(Nil* self) {
    self->c.previous = self->c.current;
    self->c.current = nillexer_lex(&self->c.lexer);

    if(nilcompiler_check(self, NILT_ERROR))
        nilcompiler_error(self, self->c.current.value);
}

NilEntry* nilcompiler_findentry(Nil* self, const char* name, NilCell n) {
    if(!name || !n) return NULL;

    NilEntry* e = nilmemory_fromcell(self, self->latest);

    while(e) {
        if(str_iequals_n(name, n, nilmemory_fromcell(self, e->name))) break;
        e = nilmemory_fromcell(self, e->link);
    }

    return e;
}

static NilLocalEntry* _nilcompiler_checklocal(Nil* self,
                                              const NilCompileInfo* nci,
                                              NilCell localidx,
                                              const char* name, NilCell n) {

    NilLocalEntry* e = nilmemory_fromcell(self, nci->word.entry->pfa[localidx]);

    while(e) {
        if(str_iequals_n(name, n, nilmemory_fromcell(self, e->name))) return e;
        e = nilmemory_fromcell(self, e->link);
    }

    return NULL;
}

void nilcompiler_checkoverride(Nil* self, const char* name, NilCell n) {
    const NilCompileInfo* nci = nilcstack_closest(self, NCI_WORD);

    if(nci) {
        if(_nilcompiler_checklocal(self, nci, NPFA_LOCALS, name, n)) {
            nilcompiler_warning(self, "shadowing local");
            return;
        }

        if(_nilcompiler_checklocal(self, nci, NPFA_ARGS, name, n)) {
            nilcompiler_warning(self, "shadowing argument");
            return;
        }
    }

    if(nilcompiler_findentry(self, name, n))
        nilcompiler_warning(self, "redefined identifier");
    else if(nilintrinsic_find(self->c.current.value, self->c.current.length))
        nilcompiler_warning(self, "redefined intrinsic");
}

void nilcompiler_definelocal(Nil* self, NilCompileInfo* nci, NilCell ncells,
                             NilCell counteridx, NilCell listidx) {

    nilcompiler_checkoverride(self, self->c.current.value,
                              self->c.current.length);

    nci->word.entry->pfa[counteridx] += ncells;

    NilLocalEntry* arg = nilmemory_alloc(self, sizeof(NilLocalEntry));
    arg->link = nci->word.entry->pfa[listidx];
    arg->ncells = ncells;
    arg->name = nilstringtable_intern(self, self->c.current.value,
                                      self->c.current.length);

    nci->word.entry->pfa[listidx] = nilmemory_tocell(self, arg);
}
