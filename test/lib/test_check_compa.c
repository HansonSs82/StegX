/*
 * This file is part of the StegX project.
 * Copyright (C) 2018  StegX Team
 * 
 * StegX is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <stdio.h>
#include <setjmp.h>
#include "cmocka.h"

#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "stegx.h"
#include "common.h"

/*
 * Setup et Teardown des tests.
 * =====================================================================
 *  */

static int test_file_compa__setup(void **state)
{
    info_s *infos = malloc(sizeof(info_s));
    infos->mode = STEGX_MODE_INSERT;
    infos->algo = STEGX_ALGO_EOF;
    infos->method = STEGX_WITH_PASSWD;
    infos->res = fopen("res.bmp", "w"), assert_non_null(infos->res);
    infos->hidden_name = malloc((strlen("test2.bmp") + 1) * sizeof(char)),
        assert_non_null(infos->hidden_name);
    strcpy(infos->hidden_name, "test2.bmp");
    infos->passwd = malloc((strlen("stegx") + 1) * sizeof(char)), assert_non_null(infos->passwd);;
    strcpy(infos->passwd, "stegx");
    *state = infos;
    return 0;
}

static int test_file_compa__teardown(void **state)
{
    info_s *infos = *state;
    fclose(infos->res);
    free(infos->hidden_name);
    free(infos->passwd);
    free(infos);
    return 0;
}

/*
 * Tests sur la compatibilité des fichiers.
 * =====================================================================
 * */

// Test sur un format BMP compressé
void test_file_compa_v1(void **state)
{
    info_s *infos = *state;
    infos->host.host =
        fopen("../../../env/test/bmp/test1.bmp", "r"), assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, BMP_COMPRESSED);

    fclose(infos->host.host);
}

// Test sur un format BMP non compressé
void test_file_compa_v2(void **state)
{
    info_s *infos = *state;
    infos->host.host =
        fopen("../../../env/test/bmp/test4.bmp", "r"), assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, BMP_UNCOMPRESSED);

    fclose(infos->host.host);
}

// Test sur un format PNG
void test_file_compa_v3(void **state)
{
    info_s *infos = *state;
    infos->host.host =
        fopen("../../../env/test/png/test8.png", "r"), assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, PNG);

    fclose(infos->host.host);
}

// Test sur un format WAV non PCM
void test_file_compa_v4(void **state)
{
    info_s *infos = *state;
    infos->host.host =
        fopen("../../../env/test/wave/WAVE_PCM(ALAW_16)_Mono_44,1kHz_1.wav", "r"),
        assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, WAV_NO_PCM);

    fclose(infos->host.host);
}

// Test sur un format MP3
void test_file_compa_v5(void **state)
{
    info_s *infos = *state;
    infos->host.host = fopen("../../../env/test/mp3/MP3_ID3v2.4_Mono_44,1kHz_64kbps.mp3", "r"),
        assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, MP3);

    fclose(infos->host.host);
}

// Test sur un format FLV
void test_file_compa_v6(void **state)
{
    info_s *infos = *state;
    infos->host.host = fopen("../../../env/test/flv/test13.flv", "r"),
        assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, FLV);

    fclose(infos->host.host);

}

// Test sur un format AVI
void test_file_compa_v7(void **state)
{
    info_s *infos = *state;
    infos->host.host = fopen("../../../env/test/avi/test14.avi", "r"),
        assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 0);
    assert_int_equal(infos->host.type, AVI_UNCOMPRESSED);

    fclose(infos->host.host);

}

// Test sur un format inconnu
void test_file_compa_v8(void **state)
{
    info_s *infos = *state;
    infos->host.host = fopen("../../../env/test/others/test16.txt", "r"),
        assert_non_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 1);
    assert_int_equal(infos->host.type, UNKNOWN);

    fclose(infos->host.host);

}

/* Test sur un fichier qui n'existe pas. */
void test_file_compa_v9(void **state)
{
    info_s *infos = *state;
    infos->host.host = fopen("../../../env/test/inconnu", "r"), assert_null(infos->host.host);

    assert_int_equal(stegx_check_compatibility(infos), 1);
    assert_int_equal(infos->host.type, UNKNOWN);
}

int main(void)
{
    /* Structure CMocka contenant la liste des tests. */
    const struct CMUnitTest check_compatibility_tests[] = {
        cmocka_unit_test(test_file_compa_v1),
        cmocka_unit_test(test_file_compa_v2),
        cmocka_unit_test(test_file_compa_v3),
        cmocka_unit_test(test_file_compa_v4),
        cmocka_unit_test(test_file_compa_v5),
        cmocka_unit_test(test_file_compa_v6),
        cmocka_unit_test(test_file_compa_v7),
        cmocka_unit_test(test_file_compa_v8),
        cmocka_unit_test(test_file_compa_v9)
    };

    /* Exécute les tests. */
    return cmocka_run_group_tests(check_compatibility_tests, test_file_compa__setup,
                                  test_file_compa__teardown);
}
