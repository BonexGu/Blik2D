/*
 * Copyright 2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <string.h>
#include BLIK_OPENSSL_V_openssl__bio_h //original-code:<openssl/bio.h>

static int justprint = 0;

static char *fpexpected[][5] = {
    /*   0 */ { "0.0000e+00", "0.0000", "0", "0.0000E+00", "0" },
    /*   1 */ { "6.7000e-01", "0.6700", "0.67", "6.7000E-01", "0.67" },
    /*   2 */ { "6.6667e-01", "0.6667", "0.6667", "6.6667E-01", "0.6667" },
    /*   3 */ { "6.6667e-04", "0.0007", "0.0006667", "6.6667E-04", "0.0006667" },
    /*   4 */ { "6.6667e-05", "0.0001", "6.667e-05", "6.6667E-05", "6.667E-05" },
    /*   5 */ { "6.6667e+00", "6.6667", "6.667", "6.6667E+00", "6.667" },
    /*   6 */ { "6.6667e+01", "66.6667", "66.67", "6.6667E+01", "66.67" },
    /*   7 */ { "6.6667e+02", "666.6667", "666.7", "6.6667E+02", "666.7" },
    /*   8 */ { "6.6667e+03", "6666.6667", "6667", "6.6667E+03", "6667" },
    /*   9 */ { "6.6667e+04", "66666.6667", "6.667e+04", "6.6667E+04", "6.667E+04" },
    /*  10 */ { "0.00000e+00", "0.00000", "0", "0.00000E+00", "0" },
    /*  11 */ { "6.70000e-01", "0.67000", "0.67", "6.70000E-01", "0.67" },
    /*  12 */ { "6.66667e-01", "0.66667", "0.66667", "6.66667E-01", "0.66667" },
    /*  13 */ { "6.66667e-04", "0.00067", "0.00066667", "6.66667E-04", "0.00066667" },
    /*  14 */ { "6.66667e-05", "0.00007", "6.6667e-05", "6.66667E-05", "6.6667E-05" },
    /*  15 */ { "6.66667e+00", "6.66667", "6.6667", "6.66667E+00", "6.6667" },
    /*  16 */ { "6.66667e+01", "66.66667", "66.667", "6.66667E+01", "66.667" },
    /*  17 */ { "6.66667e+02", "666.66667", "666.67", "6.66667E+02", "666.67" },
    /*  18 */ { "6.66667e+03", "6666.66667", "6666.7", "6.66667E+03", "6666.7" },
    /*  19 */ { "6.66667e+04", "66666.66667", "66667", "6.66667E+04", "66667" },
    /*  20 */ { "  0.0000e+00", "      0.0000", "           0", "  0.0000E+00", "           0" },
    /*  21 */ { "  6.7000e-01", "      0.6700", "        0.67", "  6.7000E-01", "        0.67" },
    /*  22 */ { "  6.6667e-01", "      0.6667", "      0.6667", "  6.6667E-01", "      0.6667" },
    /*  23 */ { "  6.6667e-04", "      0.0007", "   0.0006667", "  6.6667E-04", "   0.0006667" },
    /*  24 */ { "  6.6667e-05", "      0.0001", "   6.667e-05", "  6.6667E-05", "   6.667E-05" },
    /*  25 */ { "  6.6667e+00", "      6.6667", "       6.667", "  6.6667E+00", "       6.667" },
    /*  26 */ { "  6.6667e+01", "     66.6667", "       66.67", "  6.6667E+01", "       66.67" },
    /*  27 */ { "  6.6667e+02", "    666.6667", "       666.7", "  6.6667E+02", "       666.7" },
    /*  28 */ { "  6.6667e+03", "   6666.6667", "        6667", "  6.6667E+03", "        6667" },
    /*  29 */ { "  6.6667e+04", "  66666.6667", "   6.667e+04", "  6.6667E+04", "   6.667E+04" },
    /*  30 */ { " 0.00000e+00", "     0.00000", "           0", " 0.00000E+00", "           0" },
    /*  31 */ { " 6.70000e-01", "     0.67000", "        0.67", " 6.70000E-01", "        0.67" },
    /*  32 */ { " 6.66667e-01", "     0.66667", "     0.66667", " 6.66667E-01", "     0.66667" },
    /*  33 */ { " 6.66667e-04", "     0.00067", "  0.00066667", " 6.66667E-04", "  0.00066667" },
    /*  34 */ { " 6.66667e-05", "     0.00007", "  6.6667e-05", " 6.66667E-05", "  6.6667E-05" },
    /*  35 */ { " 6.66667e+00", "     6.66667", "      6.6667", " 6.66667E+00", "      6.6667" },
    /*  36 */ { " 6.66667e+01", "    66.66667", "      66.667", " 6.66667E+01", "      66.667" },
    /*  37 */ { " 6.66667e+02", "   666.66667", "      666.67", " 6.66667E+02", "      666.67" },
    /*  38 */ { " 6.66667e+03", "  6666.66667", "      6666.7", " 6.66667E+03", "      6666.7" },
    /*  39 */ { " 6.66667e+04", " 66666.66667", "       66667", " 6.66667E+04", "       66667" },
    /*  40 */ { "0e+00", "0", "0", "0E+00", "0" },
    /*  41 */ { "7e-01", "1", "0.7", "7E-01", "0.7" },
    /*  42 */ { "7e-01", "1", "0.7", "7E-01", "0.7" },
    /*  43 */ { "7e-04", "0", "0.0007", "7E-04", "0.0007" },
    /*  44 */ { "7e-05", "0", "7e-05", "7E-05", "7E-05" },
    /*  45 */ { "7e+00", "7", "7", "7E+00", "7" },
    /*  46 */ { "7e+01", "67", "7e+01", "7E+01", "7E+01" },
    /*  47 */ { "7e+02", "667", "7e+02", "7E+02", "7E+02" },
    /*  48 */ { "7e+03", "6667", "7e+03", "7E+03", "7E+03" },
    /*  49 */ { "7e+04", "66667", "7e+04", "7E+04", "7E+04" },
    /*  50 */ { "0.000000e+00", "0.000000", "0", "0.000000E+00", "0" },
    /*  51 */ { "6.700000e-01", "0.670000", "0.67", "6.700000E-01", "0.67" },
    /*  52 */ { "6.666667e-01", "0.666667", "0.666667", "6.666667E-01", "0.666667" },
    /*  53 */ { "6.666667e-04", "0.000667", "0.000666667", "6.666667E-04", "0.000666667" },
    /*  54 */ { "6.666667e-05", "0.000067", "6.66667e-05", "6.666667E-05", "6.66667E-05" },
    /*  55 */ { "6.666667e+00", "6.666667", "6.66667", "6.666667E+00", "6.66667" },
    /*  56 */ { "6.666667e+01", "66.666667", "66.6667", "6.666667E+01", "66.6667" },
    /*  57 */ { "6.666667e+02", "666.666667", "666.667", "6.666667E+02", "666.667" },
    /*  58 */ { "6.666667e+03", "6666.666667", "6666.67", "6.666667E+03", "6666.67" },
    /*  59 */ { "6.666667e+04", "66666.666667", "66666.7", "6.666667E+04", "66666.7" },
    /*  60 */ { "0.0000e+00", "000.0000", "00000000", "0.0000E+00", "00000000" },
    /*  61 */ { "6.7000e-01", "000.6700", "00000.67", "6.7000E-01", "00000.67" },
    /*  62 */ { "6.6667e-01", "000.6667", "000.6667", "6.6667E-01", "000.6667" },
    /*  63 */ { "6.6667e-04", "000.0007", "0.0006667", "6.6667E-04", "0.0006667" },
    /*  64 */ { "6.6667e-05", "000.0001", "6.667e-05", "6.6667E-05", "6.667E-05" },
    /*  65 */ { "6.6667e+00", "006.6667", "0006.667", "6.6667E+00", "0006.667" },
    /*  66 */ { "6.6667e+01", "066.6667", "00066.67", "6.6667E+01", "00066.67" },
    /*  67 */ { "6.6667e+02", "666.6667", "000666.7", "6.6667E+02", "000666.7" },
    /*  68 */ { "6.6667e+03", "6666.6667", "00006667", "6.6667E+03", "00006667" },
    /*  69 */ { "6.6667e+04", "66666.6667", "6.667e+04", "6.6667E+04", "6.667E+04" },
};

static void dofptest(int test, double val, char *width, int prec, int *fail)
{
    char format[80], result[80];
    int i;

    for (i = 0; i < 5; i++) {
        char *fspec = NULL;
        switch (i) {
        case 0:
            fspec = "e";
            break;
        case 1:
            fspec = "f";
            break;
        case 2:
            fspec = "g";
            break;
        case 3:
            fspec = "E";
            break;
        case 4:
            fspec = "G";
            break;
        }

        if (prec >= 0)
            BIO_snprintf(format, sizeof(format), "%%%s.%d%s", width, prec,
                         fspec);
        else
            BIO_snprintf(format, sizeof(format), "%%%s%s", width, fspec);
        BIO_snprintf(result, sizeof(result), format, val);

        if (justprint) {
            if (i == 0) {
                printf("    /* %3d */ { \"%s\"", test, result);
            } else {
                printf(", \"%s\"", result);
            }
        } else {
            if (strcmp(fpexpected[test][i], result) != 0) {
                printf("Test %d(%d) failed. Expected \"%s\". Got \"%s\". "
                       "Format \"%s\"\n", test, i, fpexpected[test][i], result,
                       format);
                *fail = 1;
            }
        }
    }
    if (justprint) {
        printf(" },\n");
    }
}

int main(int argc, char **argv)
{
    int test = 0;
    int i;
    int fail = 0;
    int prec = -1;
    char *width = "";
    const double frac = 2.0/3.0;
    char buf[80];

    if (argc == 2 && strcmp(argv[1], "-expected") == 0) {
        justprint = 1;
    }

    CRYPTO_set_mem_debug(1);
    CRYPTO_mem_ctrl(CRYPTO_MEM_CHECK_ON);

    /* Tests for floating point format specifiers */
    for (i = 0; i < 7; i++) {
        switch (i) {
        case 0:
            prec = 4;
            width = "";
            break;
        case 1:
            prec = 5;
            width = "";
            break;
        case 2:
            prec = 4;
            width = "12";
            break;
        case 3:
            prec = 5;
            width = "12";
            break;
        case 4:
            prec = 0;
            width = "";
            break;
        case 5:
            prec = -1;
            width = "";
            break;
        case 6:
            prec = 4;
            width = "08";
            break;
        }

        dofptest(test++, 0.0, width, prec, &fail);
        dofptest(test++, 0.67, width, prec, &fail);
        dofptest(test++, frac, width, prec, &fail);
        dofptest(test++, frac / 1000, width, prec, &fail);
        dofptest(test++, frac / 10000, width, prec, &fail);
        dofptest(test++, 6.0 + frac, width, prec, &fail);
        dofptest(test++, 66.0 + frac, width, prec, &fail);
        dofptest(test++, 666.0 + frac, width, prec, &fail);
        dofptest(test++, 6666.0 + frac, width, prec, &fail);
        dofptest(test++, 66666.0 + frac, width, prec, &fail);
    }

    /* Test excessively big number. Should fail */
    if (BIO_snprintf(buf, sizeof(buf), "%f\n", 2 * (double)ULONG_MAX) != -1) {
        printf("Test %d failed. Unexpected success return from "
               "BIO_snprintf()\n", test);
        fail = 1;
    }

#ifndef OPENSSL_NO_CRYPTO_MDEBUG
    if (CRYPTO_mem_leaks_fp(stderr) <= 0)
        return 1;
# endif

    if (!justprint) {
        if (fail) {
            printf("FAIL\n");
            return 1;
        }
        printf ("PASS\n");
    }
    return 0;
}


