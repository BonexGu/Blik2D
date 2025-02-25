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
#include BLIK_OPENSSL_V_openssl__evp_h //original-code:<openssl/evp.h>
#include BLIK_OPENSSL_V_openssl__objects_h //original-code:<openssl/objects.h>
#include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>
#include BLIK_OPENSSL_V_openssl__pkcs7_h //original-code:<openssl/pkcs7.h>

PKCS7_in_bio(PKCS7 *p7, BIO *in);
PKCS7_out_bio(PKCS7 *p7, BIO *out);

PKCS7_add_signer(PKCS7 *p7, X509 *cert, EVP_PKEY *key);
PKCS7_cipher(PKCS7 *p7, EVP_CIPHER *cipher);

PKCS7_Init(PKCS7 *p7);
PKCS7_Update(PKCS7 *p7);
PKCS7_Finish(PKCS7 *p7);
