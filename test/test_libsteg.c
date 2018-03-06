#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "cmocka.h"

#include "libsteg.h"

/* Tests */

void test_hello_libsteg_v2_failure(void **state)
{
    (void) state; /* Unused */
    int ret = hello_libsteg_v2(-1);
    assert_int_equal(-1, ret);
}

void test_hello_libsteg_v2_success(void **state)
{
    (void) state; /* Unused */
    int ret = hello_libsteg_v2(0);
    assert_int_equal(0, ret);
}

/* CMocka. */

const struct CMUnitTest hello_libsteg_v2_tests[] = {
    cmocka_unit_test(test_hello_libsteg_v2_failure),
    cmocka_unit_test(test_hello_libsteg_v2_success),
};

int main(void)
{
    return cmocka_run_group_tests(hello_libsteg_v2_tests, NULL, NULL);
}
