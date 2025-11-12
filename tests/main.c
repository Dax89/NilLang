#include <nil/nil.h>
#include <stdio.h>
#include <stdlib.h>

#define _TEST_RED(x) "\x1b[31m" x "\x1b[0m"
#define _TEST_GREEN(x) "\x1b[32m" x "\x1b[0m"
#define _TEST_YELLOW(x) "\x1b[33m" x "\x1b[0m"

#define TEST_CASE(name) static bool name(void)

#define TEST_REQUIRE(expr)                                                     \
    do {                                                                       \
        if(!(expr)) {                                                          \
            printf(_TEST_RED("FAIL: %s:%d: %s\n"), __FILE__, __LINE__, #expr); \
            exit(EXIT_FAILURE);                                                \
        }                                                                      \
    } while(0)

#define TEST_CHECK(expr)                                                       \
    do {                                                                       \
        if(!(expr)) {                                                          \
            printf(_TEST_YELLOW("FAIL: %s:%d: %s\n"), __FILE__, __LINE__,      \
                   #expr);                                                     \
            return false;                                                      \
        }                                                                      \
    } while(0)

#define TEST_RUN(test)                                                         \
    do {                                                                       \
        printf("Running %s... ", #test);                                       \
        if(test())                                                             \
            printf(_TEST_GREEN("PASSED\n"));                                   \
        else                                                                   \
            printf(_TEST_RED("FAILED\n"));                                     \
    } while(0)

static void execute_test(const char* filepath) {
    Nil* nil = nil_create();
    nil_loadfile(nil, filepath);
    nil_destroy(nil);
}

TEST_CASE(stack_operations) {
    Nil* nil = nil_create();

    TEST_REQUIRE(nil_runstring(nil, "5")); // (5)
    TEST_REQUIRE(nil_size(nil) == 1);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "10")); // (5 10)
    TEST_REQUIRE(nil_size(nil) == 2);
    TEST_REQUIRE(nil_top(nil) == 10);

    TEST_REQUIRE(nil_runstring(nil, "15")); // (5 10 15)
    TEST_REQUIRE(nil_size(nil) == 3);
    TEST_REQUIRE(nil_top(nil) == 15);

    TEST_REQUIRE(nil_runstring(nil, "swap")); // (5 15 10)
    TEST_REQUIRE(nil_size(nil) == 3);
    TEST_REQUIRE(nil_top(nil) == 10);

    TEST_REQUIRE(nil_runstring(nil, "rot")); // (15 10 5)
    TEST_REQUIRE(nil_size(nil) == 3);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "dup")); // (15 10 5 5)
    TEST_REQUIRE(nil_size(nil) == 4);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "rot")); // (15 5 5 10)
    TEST_REQUIRE(nil_size(nil) == 4);
    TEST_REQUIRE(nil_top(nil) == 10);

    TEST_REQUIRE(nil_runstring(nil, "over")); // (15 5 5 10 5)
    TEST_REQUIRE(nil_size(nil) == 5);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "nip")); // (15 5 5 5)
    TEST_REQUIRE(nil_size(nil) == 4);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "drop")); // (15 5 5)
    TEST_REQUIRE(nil_size(nil) == 3);
    TEST_REQUIRE(nil_top(nil) == 5);

    TEST_REQUIRE(nil_runstring(nil, "tuck")); // (15 5 5 5)
    TEST_REQUIRE(nil_size(nil) == 4);
    TEST_REQUIRE(nil_top(nil) == 5);

    nil_destroy(nil);
    return true;
}

TEST_CASE(math_operations) {
    Nil* nil = nil_create();

    TEST_REQUIRE(nil_runstring(nil, "5 3 +"));
    TEST_REQUIRE(nil_pop(nil) == 8);
    TEST_REQUIRE(nil_runstring(nil, "10 20 30 + +"));
    TEST_REQUIRE(nil_pop(nil) == 60);
    TEST_REQUIRE(nil_runstring(nil, "10 20 + 30 +"));
    TEST_REQUIRE(nil_pop(nil) == 60);

    TEST_REQUIRE(nil_runstring(nil, "7 5 -"));
    TEST_REQUIRE(nil_pop(nil) == 2);
    TEST_REQUIRE(nil_runstring(nil, "60 10 40 - -"));
    TEST_REQUIRE(nil_pop(nil) == 90);
    TEST_REQUIRE(nil_runstring(nil, "60 10 - 40 -"));
    TEST_REQUIRE(nil_pop(nil) == 10);

    TEST_REQUIRE(nil_runstring(nil, "10 5 *"));
    TEST_REQUIRE(nil_pop(nil) == 50);
    TEST_REQUIRE(nil_runstring(nil, "30 25 2 * *"));
    TEST_REQUIRE(nil_pop(nil) == 1500);
    TEST_REQUIRE(nil_runstring(nil, "30 25 * 2 *"));
    TEST_REQUIRE(nil_pop(nil) == 1500);

    TEST_REQUIRE(nil_runstring(nil, "10 5 /"));
    TEST_REQUIRE(nil_pop(nil) == 2);
    TEST_REQUIRE(nil_runstring(nil, "2000 4 2 / /"));
    TEST_REQUIRE(nil_pop(nil) == 1000);
    TEST_REQUIRE(nil_runstring(nil, "2000 4 / 2 /"));
    TEST_REQUIRE(nil_pop(nil) == 250);

    nil_destroy(nil);
    return true;
}

int main(int argc, char** argv) {
    TEST_RUN(stack_operations);
    TEST_RUN(math_operations);
    return 0;
}
