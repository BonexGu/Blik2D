/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include BLIK_OPENSSL_U_internal__cryptlib_h //original-code:"internal/cryptlib.h"
#include BLIK_OPENSSL_V_openssl__rsa_h //original-code:<openssl/rsa.h>
#include BLIK_OPENSSL_V_openssl__evp_h //original-code:<openssl/evp.h>
#include BLIK_OPENSSL_V_openssl__objects_h //original-code:<openssl/objects.h>
#include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>

int EVP_PKEY_decrypt_old(unsigned char *key, const unsigned char *ek, int ekl,
                         EVP_PKEY *priv)
{
    int ret = -1;

#ifndef OPENSSL_NO_RSA
    if (EVP_PKEY_id(priv) != EVP_PKEY_RSA) {
#endif
        EVPerr(EVP_F_EVP_PKEY_DECRYPT_OLD, EVP_R_PUBLIC_KEY_NOT_RSA);
#ifndef OPENSSL_NO_RSA
        goto err;
    }

    ret =
        RSA_private_decrypt(ekl, ek, key, EVP_PKEY_get0_RSA(priv),
                            RSA_PKCS1_PADDING);
 err:
#endif
    return (ret);
}
