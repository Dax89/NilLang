#include <assert.h>
#include <nil/nil.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

#ifdef _WIN32
#define PATH_SEP '\\'
#else
#define PATH_SEP '/'
#endif

char* get_import_filepath(const char* s, const char* rhs) {
    int folderend = strlen(s);

    for(int i = folderend; i-- > 0;) {
        if(s[i] == PATH_SEP) {
            folderend = i + 1;
            break;
        }
    }

    int len = folderend + strlen(rhs);
    char* filepath = malloc(len + 1);
    strncpy(filepath, s, folderend);
    strcpy(filepath + folderend, rhs);
    return filepath;
}

int main(int argc, char** argv) {
    if(argc != 2) {
        fprintf(stderr, "Usage:\n  NilLang <filename>\n");
        return 1;
    }

    printf(">> %s\n", argv[0]);

    Nil* n = nil_create();
    printf("~~ Executing '%s'\n", argv[1]);

    char* corepath = get_import_filepath(argv[0], "nil/core.nil");
    nil_include(n, corepath);
    free(corepath);

    if(!nil_loadfile(n, argv[1])) {
        printf("!! Loading failed\n");
        return EXIT_FAILURE;
    }

    printf("## Loading OK\n");
    // nil_disasm(n);

    if(!nil_run(n)) {
        printf("!! Run failed\n");
        return EXIT_FAILURE;
    }

    if(nil_size(n)) {
        NilCell v = nil_pop(n);
        printf("RESULT: %lu\n", v);
    }
    else
        printf("RESULT: <NO VALUE>\n");

    nil_destroy(n);
    return 0;
}
