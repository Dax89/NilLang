#include "lexer.h"
#include <string.h>

static bool _nillexer_isdelimiter(char ch) {
    switch(ch) {
        case '\t':
        case '\n':
        case '\"':
        case ' ':
        case '#':
        case '(':
        case ')': return true;

        default: break;
    }

    return false;
}

static bool _nillexer_isnumber(char ch) { return ch >= '0' && ch <= '9'; }
static bool _nillexer_atend(const NilLexer* self) { return self->length <= 0; }

static char _nillexer_advance(NilLexer* self) {
    if(_nillexer_atend(self)) return 0;

    self->current++;
    self->length--;

    // Update line/col
    if(*self->current == '\n') {
        self->line++;
        self->col = 0;
    }
    else
        self->col++;

    return self->current[-1];
}

static char _nillexer_peek(const NilLexer* self) {
    if(_nillexer_atend(self)) return 0;
    return *self->current;
}

// static char _nillexer_peeknext(const NilLexer* self) {
//     if(_nillexer_atend(self)) return 0;
//     return self->current[1];
// }

static NilToken _nillexer_maketoken(const NilLexer* self, NilTokenType type) {
    return (NilToken){
        .line = self->start_line,
        .col = self->start_col,
        .type = type,
        .value = self->start,
        .length = self->current - self->start,
    };
}

static NilToken _nillexer_error(const NilLexer* self, const char* err) {
    return (NilToken){
        .line = self->start_line,
        .col = self->start_col,
        .type = NILT_ERROR,
        .value = err,
        .length = strlen(err),
    };
}

static void _nillexer_skipcomment(NilLexer* self) {
    while(self->length && _nillexer_peek(self) != '\n')
        _nillexer_advance(self);
    _nillexer_advance(self); // Skip '\n'
}

static NilToken _nillexer_string(NilLexer* self) {
    _nillexer_advance(self); // Skip initial '\"'

    while(self->length && _nillexer_peek(self) != '\"') {
        if(*self->current == '\n')
            return _nillexer_error(self, "Invalid string");

        _nillexer_advance(self);
    }

    if(_nillexer_atend(self))
        return _nillexer_error(self, "Unterminated string");

    _nillexer_advance(self); // Skip final '\"'
    return _nillexer_maketoken(self, NILT_STR);
}

static NilToken _nillexer_word(NilLexer* self) {
    NilTokenType type = NILT_WORD;
    bool alldigits = _nillexer_isnumber(*self->start) || *self->start == '-' ||
                     *self->start == '+';

    while(self->length && !_nillexer_isdelimiter(_nillexer_peek(self))) {
        char ch = _nillexer_advance(self);
        if(!_nillexer_isnumber(ch)) alldigits = false;
    }

    if(alldigits) type = NILT_INT;
    return _nillexer_maketoken(self, type);
}

// static NilToken _nillexer_number(NilLexer* self) {
//     if(*self->start == '-' || *self->start == '+')
//         _nillexer_advance(self); // Consume sign
//
//     while(_nillexer_isnumber(_nillexer_peek(self)))
//         _nillexer_advance(self);
//
//     // if(_nillexer_peek(self) == '.' &&
//     //    _nillexer_isnumber(_nillexer_peeknext(self))) {
//     //     _nillexer_advance(self); // Consume '.'
//     //
//     //     while(_nillexer_isnumber(_nillexer_peek(self)))
//     //         _nillexer_advance(self);
//     //
//     //     if(_nillexer_atend(self))
//     //         return _nillexer_error(self, "Unterminated real number");
//     //
//     //     return _nillexer_maketoken(self, NILT_REAL);
//     // }
//
//     return _nillexer_maketoken(self, NILT_INT);
// }

void _nillexer_skipwhitespace(NilLexer* self) {
    while(!_nillexer_atend(self)) {
        char ch = _nillexer_peek(self);

        switch(ch) {
            case ' ':
            case '\t':
            case '\n':
            case '\r': _nillexer_advance(self); break;
            case '#': _nillexer_skipcomment(self); break;
            default: return;
        }
    }
}

void nillexer_init(NilLexer* self, const char* source) {
    self->current = source;
    self->length = strlen(source);
    self->start = self->current;
    self->start_line = 0;
    self->start_col = 0;
    self->line = 0;
    self->col = 0;
}

NilToken nillexer_lex(NilLexer* self) {
    if(!self) return _nillexer_maketoken(self, NILT_EOF);

    _nillexer_skipwhitespace(self);
    self->start_line = self->line;
    self->start_col = self->col;
    self->start = self->current;

    if(_nillexer_atend(self)) return _nillexer_maketoken(self, NILT_EOF);

    char c = _nillexer_advance(self);

    switch(c) {
        case '\"': return _nillexer_string(self);
        case '(': return _nillexer_maketoken(self, NILT_ROUNDOPEN);
        case ')': return _nillexer_maketoken(self, NILT_ROUNDCLOSE);
        default: break;
    }

    return _nillexer_word(self);
}
