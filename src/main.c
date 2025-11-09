#include <assert.h>
#include <nil/nil.h>
#include <stddef.h>
#include <stdio.h>

#if !defined(__has_feature)
#define __has_feature(x) 0
#endif

#if __has_feature(address_sanitizer) || defined(__SANITIZE_ADDRESS__)
const char* __asan_default_options(void) {
    return "suppressions=asan.supp:print_suppressions=0";
}

const char* __lsan_default_options(void) {
    return "suppressions=lsan.supp:print_suppressions=0";
}

const char* __lsan_default_suppressions(void) { return ""; }
#endif

int main(int argc, char** argv) {
    const char* filepath = "/home/davide/test.nil";

    Nil* n = nil_create();
    printf("~~ Executing '%s'\n", filepath);
    nil_disasmfile(n, filepath);

    if(nil_loadfile(n, filepath))
        printf("## Execution OK\n");
    else
        printf("!! Execution failed\n");

    if(nil_size(n)) {
        NilCell v = nil_pop(n);
        printf("RESULT: %lu\n", v);
    }
    else
        printf("RESULT: <NO VALUE>\n");

    nil_destroy(n);
    return 0;
}
