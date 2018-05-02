/* Inclusions minimales. */
#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "cmocka.h"

/* Inclusions supplémentaires. */
#include <errno.h>
#include <unistd.h>
#include "stegx.h"
#include "common.h"

extern type_e check_file_format(FILE * file);

void test_file_avi_v1(void **state)
{
    (void)state;                /* Unused */
    FILE *f = fopen("../../../env/test/test13.flv", "r");
    type_e test = stegx_test_file_avi(f);
    if (f != NULL)
        fclose(f);
    assert_int_equal(test, UNKNOWN);
}

void test_file_avi_v2(void **state)
{
    (void)state;                /* Unused */
    FILE *f = fopen("../../../env/test/test14.avi", "r");
    type_e test = stegx_test_file_avi(f);
    if (f != NULL)
        fclose(f);
    assert_int_equal(test, AVI_UNCOMPRESSED);
}

/* Structure CMocka contenant la liste des tests. */
const struct CMUnitTest check_compatibility_tests[] = {

    // tests unitaires AVI
    cmocka_unit_test(test_file_avi_v1),
    cmocka_unit_test(test_file_avi_v2),

};

int main(void)
{
    /* Exécute les tests. */
    return cmocka_run_group_tests(check_compatibility_tests, NULL, NULL);
}
