/*
 * Copyright 2006-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include BLIK_OPENSSL_V_openssl__asn1_h //original-code:<openssl/asn1.h>
#include BLIK_OPENSSL_V_openssl__pkcs7_h //original-code:<openssl/pkcs7.h>
#include BLIK_OPENSSL_V_openssl__bio_h //original-code:<openssl/bio.h>

#if !defined(OPENSSL_SYS_VXWORKS)
# include <memory.h>
#endif
#include <stdio.h>

/* Streaming encode support for PKCS#7 */

BIO *BIO_new_PKCS7(BIO *out, PKCS7 *p7)
{
    return BIO_new_NDEF(out, (ASN1_VALUE *)p7, ASN1_ITEM_rptr(PKCS7));
}
