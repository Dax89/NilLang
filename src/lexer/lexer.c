#include "lexer.h"
#include <string.h>

static inline bool _nillexer_isdelimiter(char ch) {
    return ch == '\t' || ch == ' ' || ch == '\n';
}

static bool _nillexer_isnumber(char ch, int base) {
    if(base < 2 || base > 16) return false;

    // Digit 0–9
    if(ch >= '0' && ch <= '9') {
        int v = ch - '0';
        return v < base;
    }

    // Normalize A–F → a–f
    if(ch >= 'A' && ch <= 'F') ch = (char)(ch - 'A' + 'a');

    // Digit a–f
    if(ch >= 'a' && ch <= 'f') {
        int v = ch - 'a' + 10;
        return v < base;
    }

    return false;
}

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

static NilToken _nillexer_maketoken(const NilLexer* self, NilTokenType type) {
    return (NilToken){
        .line = self->start_line,
        .col = self->start_col,
        .type = type,
        .value = self->start,
        .length = self->current - self->start,
    };
}

static void _nillexer_checknumber(NilToken* t) {
    int idx = 0;
    int base = 10;

    // Check sign
    if(t->value[idx] == '-' || t->value[idx] == '+') idx++;

    // Check base prefix
    if(t->length > 3 && t->value[idx] == '0') {
        idx++;

        if(t->value[idx] == 'b' || t->value[idx] == 'B')
            base = 2;
        else if(t->value[idx] == 'o' || t->value[idx] == 'O')
            base = 8;
        else if(t->value[idx] == 'x' || t->value[idx] == 'X')
            base = 16;
    }
    else if(idx >= t->length)
        return;

    while(idx < t->length) {
        if(!_nillexer_isnumber(t->value[idx++], base)) return;
    }

    t->type = NILT_INT;
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

static NilToken _nillexer_char(NilLexer* self) {
    _nillexer_advance(self); // Skip initial '\''
    if(*self->current == '\'')
        return _nillexer_error(self, "empty character literal");

    _nillexer_advance(self);
    if(*self->current != '\'')
        return _nillexer_error(self, "invalid character literal");

    _nillexer_advance(self); // Skip final '\''
    return _nillexer_maketoken(self, NILT_CHAR);
}

static NilToken _nillexer_string(NilLexer* self) {
    _nillexer_advance(self); // Skip initial '\"'

    while(self->length && *self->current != '\"') {
        if(*self->current == '\n')
            return _nillexer_error(self, "multiline strings are not supported");

        _nillexer_advance(self);
    }

    if(_nillexer_atend(self))
        return _nillexer_error(self, "unterminated string");

    _nillexer_advance(self); // Skip final '\"'
    return _nillexer_maketoken(self, NILT_STR);
}

static NilToken _nillexer_tokenize(NilLexer* self) {
    while(self->length && !_nillexer_isdelimiter(*self->current))
        _nillexer_advance(self);

    NilToken token = _nillexer_maketoken(self, NILT_IDENTIFIER);
    _nillexer_checknumber(&token);
    return token;
}

static void _nillexer_skipuntil(NilLexer* self, char ch) {
    while(self->length && *self->current != ch)
        _nillexer_advance(self);
    _nillexer_advance(self); // Skip 'ch'
}

static void _nillexer_skipall(NilLexer* self) {
    while(!_nillexer_atend(self)) {
        switch(*self->current) {
            case ' ':
            case '\t':
            case '\n':
            case '\r': _nillexer_advance(self); break;
            case '\\': _nillexer_skipuntil(self, '\n'); break;
            case '(': _nillexer_skipuntil(self, ')'); break;
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

    _nillexer_skipall(self);

    self->start_line = self->line;
    self->start_col = self->col;
    self->start = self->current;

    if(_nillexer_atend(self)) return _nillexer_maketoken(self, NILT_EOF);
    if(*self->start == '\'') return _nillexer_char(self);
    if(*self->start == '\"') return _nillexer_string(self);
    return _nillexer_tokenize(self);
}
