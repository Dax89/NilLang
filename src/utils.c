#include "utils.h"
#include "types.h"
#include <limits.h>

char str_toupper(char s) {
    if(s >= 'a' && s <= 'z') s -= 0x20;
    return s;
}

NilCell uleb128_read(const char* p, NilCell n, NilCell* outsz) {
    NilCell result = 0, i = 0;
    int shift = 0;
    unsigned char byte;

    do {
        if(i >= n) break;
        byte = (unsigned char)p[i++];
        result |= ((NilCell)(byte & 0x7F) << shift);
        shift += 7;
    } while(byte & 0x80);

    if(outsz) *outsz = i;
    return result;
}

bool str_iequals(const char* s1, const char* s2) {
    while(*s1 && *s2) {
        if(str_toupper(*s1) != str_toupper(*s2)) return false;
        s1++;
        s2++;
    }

    return *s1 == *s2;
}

bool str_iequals_n(const char* s1, int n, const char* s2) {
    for(int i = 0; i < n; i++) {
        if(!s2[i]) return false; // s2 too short
        if(str_toupper(s1[i]) != str_toupper(s2[i])) return false;
    }

    return !s2[n];
}

NilCell str_toint(const char* s, int n) {
    NilCell res = 0, sign = 1;

    if(n > 0 && (*s == '-' || *s == '+')) {
        if(*s == '-') sign = -1;
        s++;
        n--;
    }

    for(int i = 0; i < n; i++)
        res = (res * 10) + (s[i] - '0');

    return res * sign;
}

double str_toreal(const char* s, int n) {
    double res = 0.0;
    int sign = 1;
    int i = 0;

    if(i < n && (s[i] == '-' || s[i] == '+')) {
        if(s[i] == '-') sign = -1;
        i++;
    }

    // Integer part
    while(i < n && s[i] >= '0' && s[i] <= '9') {
        res = (res * 10.0) + (s[i] - '0');
        i++;
    }

    // Fractional part
    if(i < n && s[i] == '.') {
        i++;
        double frac = 0.0, div = 1.0;
        while(i < n && s[i] >= '0' && s[i] <= '9') {
            frac = (frac * 10.0) + (s[i] - '0');
            div *= 10.0;
            i++;
        }
        res += frac / div;
    }

    return sign * res;
}

char* str_tobase(const Nil* nil, NilCell v) {
    static const char DIGITS[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static char out[67]; // 66+1 for negative sign

    if(nil->vm.base < 2 || nil->vm.base > 36)
        nil_error("invalid base: %" NIL_CELLFMT, nil->vm.base);

    int c = sizeof(out) - 1;
    out[c] = 0;

    do {
        NilCell rem = v % nil->vm.base;
        v /= nil->vm.base;
        out[--c] = DIGITS[rem];
    } while(v > 0);

    return &out[c];
}
