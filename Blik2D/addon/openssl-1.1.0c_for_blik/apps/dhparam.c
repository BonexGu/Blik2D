/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include BLIK_OPENSSL_V_openssl__opensslconf_h //original-code:<openssl/opensslconf.h>
#ifdef OPENSSL_NO_DH
NON_EMPTY_TRANSLATION_UNIT
#else

# include <stdio.h>
# include <stdlib.h>
# include <time.h>
# include <string.h>
# include "apps.h"
# include BLIK_OPENSSL_V_openssl__bio_h //original-code:<openssl/bio.h>
# include BLIK_OPENSSL_V_openssl__err_h //original-code:<openssl/err.h>
# include BLIK_OPENSSL_V_openssl__bn_h //original-code:<openssl/bn.h>
# include BLIK_OPENSSL_V_openssl__dh_h //original-code:<openssl/dh.h>
# include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>
# include BLIK_OPENSSL_V_openssl__pem_h //original-code:<openssl/pem.h>

# ifndef OPENSSL_NO_DSA
#  include BLIK_OPENSSL_V_openssl__dsa_h //original-code:<openssl/dsa.h>
# endif

# define DEFBITS 2048

static int dh_cb(int p, int n, BN_GENCB *cb);

typedef enum OPTION_choice {
    OPT_ERR = -1, OPT_EOF = 0, OPT_HELP,
    OPT_INFORM, OPT_OUTFORM, OPT_IN, OPT_OUT,
    OPT_ENGINE, OPT_CHECK, OPT_TEXT, OPT_NOOUT,
    OPT_RAND, OPT_DSAPARAM, OPT_C, OPT_2, OPT_5
} OPTION_CHOICE;

OPTIONS dhparam_options[] = {
    {OPT_HELP_STR, 1, '-', "Usage: %s [flags] [numbits]\n"},
    {OPT_HELP_STR, 1, '-', "Valid options are:\n"},
    {"help", OPT_HELP, '-', "Display this summary"},
    {"in", OPT_IN, '<', "Input file"},
    {"inform", OPT_INFORM, 'F', "Input format, DER or PEM"},
    {"outform", OPT_OUTFORM, 'F', "Output format, DER or PEM"},
    {"out", OPT_OUT, '>', "Output file"},
    {"check", OPT_CHECK, '-', "Check the DH parameters"},
    {"text", OPT_TEXT, '-', "Print a text form of the DH parameters"},
    {"noout", OPT_NOOUT, '-', "Don't output any DH parameters"},
    {"rand", OPT_RAND, 's',
     "Load the file(s) into the random number generator"},
    {"C", OPT_C, '-', "Print C code"},
    {"2", OPT_2, '-', "Generate parameters using 2 as the generator value"},
    {"5", OPT_5, '-', "Generate parameters using 5 as the generator value"},
# ifndef OPENSSL_NO_DSA
    {"dsaparam", OPT_DSAPARAM, '-',
     "Read or generate DSA parameters, convert to DH"},
# endif
# ifndef OPENSSL_NO_ENGINE
    {"engine", OPT_ENGINE, 's', "Use engine e, possibly a hardware device"},
# endif
    {NULL}
};

int dhparam_main(int argc, char **argv)
{
    BIO *in = NULL, *out = NULL;
    DH *dh = NULL;
    char *infile = NULL, *outfile = NULL, *prog, *inrand = NULL;
    ENGINE *e = NULL;
#ifndef OPENSSL_NO_DSA
    int dsaparam = 0;
#endif
    int i, text = 0, C = 0, ret = 1, num = 0, g = 0;
    int informat = FORMAT_PEM, outformat = FORMAT_PEM, check = 0, noout = 0;
    OPTION_CHOICE o;

    prog = opt_init(argc, argv, dhparam_options);
    while ((o = opt_next()) != OPT_EOF) {
        switch (o) {
        case OPT_EOF:
        case OPT_ERR:
 opthelp:
            BIO_printf(bio_err, "%s: Use -help for summary.\n", prog);
            goto end;
        case OPT_HELP:
            opt_help(dhparam_options);
            ret = 0;
            goto end;
        case OPT_INFORM:
            if (!opt_format(opt_arg(), OPT_FMT_PEMDER, &informat))
                goto opthelp;
            break;
        case OPT_OUTFORM:
            if (!opt_format(opt_arg(), OPT_FMT_PEMDER, &outformat))
                goto opthelp;
            break;
        case OPT_IN:
            infile = opt_arg();
            break;
        case OPT_OUT:
            outfile = opt_arg();
            break;
        case OPT_ENGINE:
            e = setup_engine(opt_arg(), 0);
            break;
        case OPT_CHECK:
            check = 1;
            break;
        case OPT_TEXT:
            text = 1;
            break;
        case OPT_DSAPARAM:
#ifndef OPENSSL_NO_DSA
            dsaparam = 1;
#endif
            break;
        case OPT_C:
            C = 1;
            break;
        case OPT_2:
            g = 2;
            break;
        case OPT_5:
            g = 5;
            break;
        case OPT_NOOUT:
            noout = 1;
            break;
        case OPT_RAND:
            inrand = opt_arg();
            break;
        }
    }
    argc = opt_num_rest();
    argv = opt_rest();

    if (argv[0] && (!opt_int(argv[0], &num) || num <= 0))
        goto end;

    if (g && !num)
        num = DEFBITS;

# ifndef OPENSSL_NO_DSA
    if (dsaparam && g) {
        BIO_printf(bio_err,
                   "generator may not be chosen for DSA parameters\n");
        goto end;
    }
# endif
    /* DH parameters */
    if (num && !g)
        g = 2;

    if (num) {

        BN_GENCB *cb;
        cb = BN_GENCB_new();
        if (cb == NULL) {
            ERR_print_errors(bio_err);
            goto end;
        }

        BN_GENCB_set(cb, dh_cb, bio_err);
        if (!app_RAND_load_file(NULL, 1) && inrand == NULL) {
            BIO_printf(bio_err,
                       "warning, not much extra random data, consider using the -rand option\n");
        }
        if (inrand != NULL)
            BIO_printf(bio_err, "%ld semi-random bytes loaded\n",
                       app_RAND_load_files(inrand));

# ifndef OPENSSL_NO_DSA
        if (dsaparam) {
            DSA *dsa = DSA_new();

            BIO_printf(bio_err,
                       "Generating DSA parameters, %d bit long prime\n", num);
            if (dsa == NULL
                || !DSA_generate_parameters_ex(dsa, num, NULL, 0, NULL, NULL,
                                               cb)) {
                DSA_free(dsa);
                BN_GENCB_free(cb);
                ERR_print_errors(bio_err);
                goto end;
            }

            dh = DSA_dup_DH(dsa);
            DSA_free(dsa);
            if (dh == NULL) {
                BN_GENCB_free(cb);
                ERR_print_errors(bio_err);
                goto end;
            }
        } else
# endif
        {
            dh = DH_new();
            BIO_printf(bio_err,
                       "Generating DH parameters, %d bit long safe prime, generator %d\n",
                       num, g);
            BIO_printf(bio_err, "This is going to take a long time\n");
            if (dh == NULL || !DH_generate_parameters_ex(dh, num, g, cb)) {
                BN_GENCB_free(cb);
                ERR_print_errors(bio_err);
                goto end;
            }
        }

        BN_GENCB_free(cb);
        app_RAND_write_file(NULL);
    } else {

        in = bio_open_default(infile, 'r', informat);
        if (in == NULL)
            goto end;

# ifndef OPENSSL_NO_DSA
        if (dsaparam) {
            DSA *dsa;

            if (informat == FORMAT_ASN1)
                dsa = d2i_DSAparams_bio(in, NULL);
            else                /* informat == FORMAT_PEM */
                dsa = PEM_read_bio_DSAparams(in, NULL, NULL, NULL);

            if (dsa == NULL) {
                BIO_printf(bio_err, "unable to load DSA parameters\n");
                ERR_print_errors(bio_err);
                goto end;
            }

            dh = DSA_dup_DH(dsa);
            DSA_free(dsa);
            if (dh == NULL) {
                ERR_print_errors(bio_err);
                goto end;
            }
        } else
# endif
        {
            if (informat == FORMAT_ASN1)
                dh = d2i_DHparams_bio(in, NULL);
            else                /* informat == FORMAT_PEM */
                dh = PEM_read_bio_DHparams(in, NULL, NULL, NULL);

            if (dh == NULL) {
                BIO_printf(bio_err, "unable to load DH parameters\n");
                ERR_print_errors(bio_err);
                goto end;
            }
        }

        /* dh != NULL */
    }

    out = bio_open_default(outfile, 'w', outformat);
    if (out == NULL)
        goto end;

    if (text) {
        DHparams_print(out, dh);
    }

    if (check) {
        if (!DH_check(dh, &i)) {
            ERR_print_errors(bio_err);
            goto end;
        }
        if (i & DH_CHECK_P_NOT_PRIME)
            BIO_printf(bio_err, "WARNING: p value is not prime\n");
        if (i & DH_CHECK_P_NOT_SAFE_PRIME)
            BIO_printf(bio_err, "WARNING: p value is not a safe prime\n");
        if (i & DH_CHECK_Q_NOT_PRIME)
            BIO_printf(bio_err, "WARNING: q value is not a prime\n");
        if (i & DH_CHECK_INVALID_Q_VALUE)
            BIO_printf(bio_err, "WARNING: q value is invalid\n");
        if (i & DH_CHECK_INVALID_J_VALUE)
            BIO_printf(bio_err, "WARNING: j value is invalid\n");
        if (i & DH_UNABLE_TO_CHECK_GENERATOR)
            BIO_printf(bio_err,
                       "WARNING: unable to check the generator value\n");
        if (i & DH_NOT_SUITABLE_GENERATOR)
            BIO_printf(bio_err, "WARNING: the g value is not a generator\n");
        if (i == 0)
            BIO_printf(bio_err, "DH parameters appear to be ok.\n");
        if (num != 0 && i != 0) {
            /*
             * We have generated parameters but DH_check() indicates they are
             * invalid! This should never happen!
             */
            BIO_printf(bio_err, "ERROR: Invalid parameters generated\n");
            goto end;
        }
    }
    if (C) {
        unsigned char *data;
        int len, bits;
        const BIGNUM *pbn, *gbn;

        len = DH_size(dh);
        bits = DH_bits(dh);
        DH_get0_pqg(dh, &pbn, NULL, &gbn);
        data = app_malloc(len, "print a BN");
        BIO_printf(out, "#ifndef HEADER_DH_H\n"
                        "# include BLIK_OPENSSL_V_openssl__dh_h //original-code:<openssl/dh.h>\n"
                        "#endif\n"
                        "\n");
        BIO_printf(out, "DH *get_dh%d()\n{\n", bits);
        print_bignum_var(out, pbn, "dhp", bits, data);
        print_bignum_var(out, gbn, "dhg", bits, data);
        BIO_printf(out, "    DH *dh = DH_new();\n"
                        "    BIGNUM *dhp_bn, *dhg_bn;\n"
                        "\n"
                        "    if (dh == NULL)\n"
                        "        return NULL;\n");
        BIO_printf(out, "    dhp_bn = BN_bin2bn(dhp_%d, sizeof (dhp_%d), NULL);\n",
                   bits, bits);
        BIO_printf(out, "    dhg_bn = BN_bin2bn(dhg_%d, sizeof (dhg_%d), NULL);\n",
                   bits, bits);
        BIO_printf(out, "    if (dhp_bn == NULL || dhg_bn == NULL\n"
                        "            || !DH_set0_pqg(dh, dhp_bn, NULL, dhg_bn)) {\n"
                        "        DH_free(dh);\n"
                        "        BN_free(dhp_bn);\n"
                        "        BN_free(dhg_bn);\n"
                        "        return NULL;\n"
                        "    }\n");
        if (DH_get_length(dh) > 0)
            BIO_printf(out,
                        "    if (!DH_set_length(dh, %ld)) {\n"
                        "        DH_free(dh);\n"
                        "    }\n", DH_get_length(dh));
        BIO_printf(out, "    return dh;\n}\n");
        OPENSSL_free(data);
    }

    if (!noout) {
        const BIGNUM *q;
        DH_get0_pqg(dh, NULL, &q, NULL);
        if (outformat == FORMAT_ASN1)
            i = i2d_DHparams_bio(out, dh);
        else if (q != NULL)
            i = PEM_write_bio_DHxparams(out, dh);
        else
            i = PEM_write_bio_DHparams(out, dh);
        if (!i) {
            BIO_printf(bio_err, "unable to write DH parameters\n");
            ERR_print_errors(bio_err);
            goto end;
        }
    }
    ret = 0;
 end:
    BIO_free(in);
    BIO_free_all(out);
    DH_free(dh);
    release_engine(e);
    return (ret);
}

static int dh_cb(int p, int n, BN_GENCB *cb)
{
    char c = '*';

    if (p == 0)
        c = '.';
    if (p == 1)
        c = '+';
    if (p == 2)
        c = '*';
    if (p == 3)
        c = '\n';
    BIO_write(BN_GENCB_get_arg(cb), &c, 1);
    (void)BIO_flush(BN_GENCB_get_arg(cb));
    return 1;
}
#endif
