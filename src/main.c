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

void repl(Nil* nil) {
    fprintf(stdout, "Nil REPL (type 'exit' to quit)\n");

    char* source = NULL;
    size_t len;

    for(;;) {
        fprintf(stdout, "> ");

        ssize_t nread = getline(&source, &len, stdin);
        if(nread == -1) break;
        if(source[nread - 1] == '\n') source[nread - 1] = '\0';
        nil_runstring(nil, source);
    }

    free(source);
}

void help(void) {
    // clang-format off
    static const char HELP_TEXT[] = {
        NIL_VERSION "\n"
        "  -h        print this help and exit\n"
        "  -v        print version and exit\n"
        "  -l <file> load core from file\n"
        "Options must come before files to execute\n"
    };
    // clang-format on

    fputs(HELP_TEXT, stdout);
}

int main(int argc, char** argv) {
    int nopt;

    for(nopt = 1; nopt < argc && argv[nopt][0] == '-'; nopt++) {
        switch(argv[nopt][1]) {
            case 'h':
                help();
                exit(EXIT_SUCCESS);
                break;

            case 'v': fputs(NIL_VERSION "\n", stdout); break;

            default:
                fprintf(stderr, "Usage: %s [-options...] [files...]\n",
                        argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    argv += nopt;

    Nil* nil = nil_create();

    if(nopt != argc)
        nil_runfile(nil, *argv);
    else
        repl(nil);

    nil_destroy(nil);
    return 0;
}
