/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include BLIK_OPENSSL_V_openssl__stack_h //original-code:<openssl/stack.h>
#include BLIK_OPENSSL_U_internal__cryptlib_h //original-code:"internal/cryptlib.h"
#include BLIK_OPENSSL_V_openssl__asn1_h //original-code:<openssl/asn1.h>
#include BLIK_OPENSSL_V_openssl__objects_h //original-code:<openssl/objects.h>
#include BLIK_OPENSSL_V_openssl__evp_h //original-code:<openssl/evp.h>
#include BLIK_OPENSSL_V_openssl__x509_h //original-code:<openssl/x509.h>
#include BLIK_OPENSSL_U_internal__x509_int_h //original-code:"internal/x509_int.h"
#include BLIK_OPENSSL_V_openssl__x509v3_h //original-code:<openssl/x509v3.h>

int X509_CRL_get_ext_count(const X509_CRL *x)
{
    return (X509v3_get_ext_count(x->crl.extensions));
}

int X509_CRL_get_ext_by_NID(const X509_CRL *x, int nid, int lastpos)
{
    return (X509v3_get_ext_by_NID(x->crl.extensions, nid, lastpos));
}

int X509_CRL_get_ext_by_OBJ(const X509_CRL *x, const ASN1_OBJECT *obj,
                            int lastpos)
{
    return (X509v3_get_ext_by_OBJ(x->crl.extensions, obj, lastpos));
}

int X509_CRL_get_ext_by_critical(const X509_CRL *x, int crit, int lastpos)
{
    return (X509v3_get_ext_by_critical(x->crl.extensions, crit, lastpos));
}

X509_EXTENSION *X509_CRL_get_ext(const X509_CRL *x, int loc)
{
    return (X509v3_get_ext(x->crl.extensions, loc));
}

X509_EXTENSION *X509_CRL_delete_ext(X509_CRL *x, int loc)
{
    return (X509v3_delete_ext(x->crl.extensions, loc));
}

void *X509_CRL_get_ext_d2i(const X509_CRL *x, int nid, int *crit, int *idx)
{
    return X509V3_get_d2i(x->crl.extensions, nid, crit, idx);
}

int X509_CRL_add1_ext_i2d(X509_CRL *x, int nid, void *value, int crit,
                          unsigned long flags)
{
    return X509V3_add1_i2d(&x->crl.extensions, nid, value, crit, flags);
}

int X509_CRL_add_ext(X509_CRL *x, X509_EXTENSION *ex, int loc)
{
    return (X509v3_add_ext(&(x->crl.extensions), ex, loc) != NULL);
}

int X509_get_ext_count(const X509 *x)
{
    return (X509v3_get_ext_count(x->cert_info.extensions));
}

int X509_get_ext_by_NID(const X509 *x, int nid, int lastpos)
{
    return (X509v3_get_ext_by_NID(x->cert_info.extensions, nid, lastpos));
}

int X509_get_ext_by_OBJ(const X509 *x, const ASN1_OBJECT *obj, int lastpos)
{
    return (X509v3_get_ext_by_OBJ(x->cert_info.extensions, obj, lastpos));
}

int X509_get_ext_by_critical(const X509 *x, int crit, int lastpos)
{
    return (X509v3_get_ext_by_critical
            (x->cert_info.extensions, crit, lastpos));
}

X509_EXTENSION *X509_get_ext(const X509 *x, int loc)
{
    return (X509v3_get_ext(x->cert_info.extensions, loc));
}

X509_EXTENSION *X509_delete_ext(X509 *x, int loc)
{
    return (X509v3_delete_ext(x->cert_info.extensions, loc));
}

int X509_add_ext(X509 *x, X509_EXTENSION *ex, int loc)
{
    return (X509v3_add_ext(&(x->cert_info.extensions), ex, loc) != NULL);
}

void *X509_get_ext_d2i(const X509 *x, int nid, int *crit, int *idx)
{
    return X509V3_get_d2i(x->cert_info.extensions, nid, crit, idx);
}

int X509_add1_ext_i2d(X509 *x, int nid, void *value, int crit,
                      unsigned long flags)
{
    return X509V3_add1_i2d(&x->cert_info.extensions, nid, value, crit,
                           flags);
}

int X509_REVOKED_get_ext_count(const X509_REVOKED *x)
{
    return (X509v3_get_ext_count(x->extensions));
}

int X509_REVOKED_get_ext_by_NID(const X509_REVOKED *x, int nid, int lastpos)
{
    return (X509v3_get_ext_by_NID(x->extensions, nid, lastpos));
}

int X509_REVOKED_get_ext_by_OBJ(const X509_REVOKED *x, const ASN1_OBJECT *obj,
                                int lastpos)
{
    return (X509v3_get_ext_by_OBJ(x->extensions, obj, lastpos));
}

int X509_REVOKED_get_ext_by_critical(const X509_REVOKED *x, int crit, int lastpos)
{
    return (X509v3_get_ext_by_critical(x->extensions, crit, lastpos));
}

X509_EXTENSION *X509_REVOKED_get_ext(const X509_REVOKED *x, int loc)
{
    return (X509v3_get_ext(x->extensions, loc));
}

X509_EXTENSION *X509_REVOKED_delete_ext(X509_REVOKED *x, int loc)
{
    return (X509v3_delete_ext(x->extensions, loc));
}

int X509_REVOKED_add_ext(X509_REVOKED *x, X509_EXTENSION *ex, int loc)
{
    return (X509v3_add_ext(&(x->extensions), ex, loc) != NULL);
}

void *X509_REVOKED_get_ext_d2i(const X509_REVOKED *x, int nid, int *crit, int *idx)
{
    return X509V3_get_d2i(x->extensions, nid, crit, idx);
}

int X509_REVOKED_add1_ext_i2d(X509_REVOKED *x, int nid, void *value, int crit,
                              unsigned long flags)
{
    return X509V3_add1_i2d(&x->extensions, nid, value, crit, flags);
}
