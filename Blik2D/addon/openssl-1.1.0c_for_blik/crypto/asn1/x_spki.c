/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

 /*
  * This module was send to me my Pat Richards <patr@x509.com> who wrote it.
  * It is under my Copyright with his permission
  */

#include <stdio.h>
#include BLIK_OPENSSL_U_internal__cryptlib_h //original-code:"internal/cryptlib.h"
#include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>
#include BLIK_OPENSSL_V_openssl__asn1t_h //original-code:<openssl/asn1t.h>

ASN1_SEQUENCE(NETSCAPE_SPKAC) = {
        ASN1_SIMPLE(NETSCAPE_SPKAC, pubkey, X509_PUBKEY),
        ASN1_SIMPLE(NETSCAPE_SPKAC, challenge, ASN1_IA5STRING)
} ASN1_SEQUENCE_END(NETSCAPE_SPKAC)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_SPKAC)

ASN1_SEQUENCE(NETSCAPE_SPKI) = {
        ASN1_SIMPLE(NETSCAPE_SPKI, spkac, NETSCAPE_SPKAC),
        ASN1_EMBED(NETSCAPE_SPKI, sig_algor, X509_ALGOR),
        ASN1_SIMPLE(NETSCAPE_SPKI, signature, ASN1_BIT_STRING)
} ASN1_SEQUENCE_END(NETSCAPE_SPKI)

IMPLEMENT_ASN1_FUNCTIONS(NETSCAPE_SPKI)
