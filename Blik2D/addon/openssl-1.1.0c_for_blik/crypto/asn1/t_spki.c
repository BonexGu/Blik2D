/*
 * Copyright 1999-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include BLIK_OPENSSL_U_internal__cryptlib_h //original-code:"internal/cryptlib.h"
#include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>
#include BLIK_OPENSSL_V_openssl__asn1_h //original-code:<openssl/asn1.h>
#include BLIK_OPENSSL_V_openssl__rsa_h //original-code:<openssl/rsa.h>
#include BLIK_OPENSSL_V_openssl__dsa_h //original-code:<openssl/dsa.h>
#include BLIK_OPENSSL_V_openssl__bn_h //original-code:<openssl/bn.h>

/* Print out an SPKI */

int NETSCAPE_SPKI_print(BIO *out, NETSCAPE_SPKI *spki)
{
    EVP_PKEY *pkey;
    ASN1_IA5STRING *chal;
    ASN1_OBJECT *spkioid;
    int i, n;
    char *s;
    BIO_printf(out, "Netscape SPKI:\n");
    X509_PUBKEY_get0_param(&spkioid, NULL, NULL, NULL, spki->spkac->pubkey);
    i = OBJ_obj2nid(spkioid);
    BIO_printf(out, "  Public Key Algorithm: %s\n",
               (i == NID_undef) ? "UNKNOWN" : OBJ_nid2ln(i));
    pkey = X509_PUBKEY_get(spki->spkac->pubkey);
    if (!pkey)
        BIO_printf(out, "  Unable to load public key\n");
    else {
        EVP_PKEY_print_public(out, pkey, 4, NULL);
        EVP_PKEY_free(pkey);
    }
    chal = spki->spkac->challenge;
    if (chal->length)
        BIO_printf(out, "  Challenge String: %s\n", chal->data);
    i = OBJ_obj2nid(spki->sig_algor.algorithm);
    BIO_printf(out, "  Signature Algorithm: %s",
               (i == NID_undef) ? "UNKNOWN" : OBJ_nid2ln(i));

    n = spki->signature->length;
    s = (char *)spki->signature->data;
    for (i = 0; i < n; i++) {
        if ((i % 18) == 0)
            BIO_write(out, "\n      ", 7);
        BIO_printf(out, "%02x%s", (unsigned char)s[i],
                   ((i + 1) == n) ? "" : ":");
    }
    BIO_write(out, "\n", 1);
    return 1;
}
