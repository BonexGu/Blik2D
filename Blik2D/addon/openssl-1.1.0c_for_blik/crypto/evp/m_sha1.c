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
#include BLIK_OPENSSL_V_openssl__sha_h //original-code:<openssl/sha.h>
#include BLIK_OPENSSL_V_openssl__rsa_h //original-code:<openssl/rsa.h>
#include BLIK_OPENSSL_U_internal__evp_int_h //original-code:"internal/evp_int.h"

static int init(EVP_MD_CTX *ctx)
{
    return SHA1_Init(EVP_MD_CTX_md_data(ctx));
}

static int update(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    return SHA1_Update(EVP_MD_CTX_md_data(ctx), data, count);
}

static int final(EVP_MD_CTX *ctx, unsigned char *md)
{
    return SHA1_Final(md, EVP_MD_CTX_md_data(ctx));
}

static int ctrl(EVP_MD_CTX *ctx, int cmd, int mslen, void *ms)
{
    unsigned char padtmp[40];
    unsigned char sha1tmp[SHA_DIGEST_LENGTH];

    SHA_CTX *sha1 = EVP_MD_CTX_md_data(ctx);

    if (cmd != EVP_CTRL_SSL3_MASTER_SECRET)
        return 0;

    /* SSLv3 client auth handling: see RFC-6101 5.6.8 */
    if (mslen != 48)
        return 0;

    /* At this point hash contains all handshake messages, update
     * with master secret and pad_1.
     */

    if (SHA1_Update(sha1, ms, mslen) <= 0)
        return 0;

    /* Set padtmp to pad_1 value */
    memset(padtmp, 0x36, sizeof(padtmp));

    if (!SHA1_Update(sha1, padtmp, sizeof(padtmp)))
        return 0;

    if (!SHA1_Final(sha1tmp, sha1))
        return 0;

    /* Reinitialise context */

    if (!SHA1_Init(sha1))
        return 0;

    if (SHA1_Update(sha1, ms, mslen) <= 0)
        return 0;

    /* Set padtmp to pad_2 value */
    memset(padtmp, 0x5c, sizeof(padtmp));

    if (!SHA1_Update(sha1, padtmp, sizeof(padtmp)))
        return 0;

    if (!SHA1_Update(sha1, sha1tmp, sizeof(sha1tmp)))
        return 0;

    /* Now when ctx is finalised it will return the SSL v3 hash value */
    OPENSSL_cleanse(sha1tmp, sizeof(sha1tmp));

    return 1;

}

static const EVP_MD sha1_md = {
    NID_sha1,
    NID_sha1WithRSAEncryption,
    SHA_DIGEST_LENGTH,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    init,
    update,
    final,
    NULL,
    NULL,
    SHA_CBLOCK,
    sizeof(EVP_MD *) + sizeof(SHA_CTX),
    ctrl
};

const EVP_MD *EVP_sha1(void)
{
    return (&sha1_md);
}

static int init224(EVP_MD_CTX *ctx)
{
    return SHA224_Init(EVP_MD_CTX_md_data(ctx));
}

static int init256(EVP_MD_CTX *ctx)
{
    return SHA256_Init(EVP_MD_CTX_md_data(ctx));
}

/*
 * Even though there're separate SHA224_[Update|Final], we call
 * SHA256 functions even in SHA224 context. This is what happens
 * there anyway, so we can spare few CPU cycles:-)
 */
static int update256(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    return SHA256_Update(EVP_MD_CTX_md_data(ctx), data, count);
}

static int final256(EVP_MD_CTX *ctx, unsigned char *md)
{
    return SHA256_Final(md, EVP_MD_CTX_md_data(ctx));
}

static const EVP_MD sha224_md = {
    NID_sha224,
    NID_sha224WithRSAEncryption,
    SHA224_DIGEST_LENGTH,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    init224,
    update256,
    final256,
    NULL,
    NULL,
    SHA256_CBLOCK,
    sizeof(EVP_MD *) + sizeof(SHA256_CTX),
};

const EVP_MD *EVP_sha224(void)
{
    return (&sha224_md);
}

static const EVP_MD sha256_md = {
    NID_sha256,
    NID_sha256WithRSAEncryption,
    SHA256_DIGEST_LENGTH,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    init256,
    update256,
    final256,
    NULL,
    NULL,
    SHA256_CBLOCK,
    sizeof(EVP_MD *) + sizeof(SHA256_CTX),
};

const EVP_MD *EVP_sha256(void)
{
    return (&sha256_md);
}

static int init384(EVP_MD_CTX *ctx)
{
    return SHA384_Init(EVP_MD_CTX_md_data(ctx));
}

static int init512(EVP_MD_CTX *ctx)
{
    return SHA512_Init(EVP_MD_CTX_md_data(ctx));
}

/* See comment in SHA224/256 section */
static int update512(EVP_MD_CTX *ctx, const void *data, size_t count)
{
    return SHA512_Update(EVP_MD_CTX_md_data(ctx), data, count);
}

static int final512(EVP_MD_CTX *ctx, unsigned char *md)
{
    return SHA512_Final(md, EVP_MD_CTX_md_data(ctx));
}

static const EVP_MD sha384_md = {
    NID_sha384,
    NID_sha384WithRSAEncryption,
    SHA384_DIGEST_LENGTH,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    init384,
    update512,
    final512,
    NULL,
    NULL,
    SHA512_CBLOCK,
    sizeof(EVP_MD *) + sizeof(SHA512_CTX),
};

const EVP_MD *EVP_sha384(void)
{
    return (&sha384_md);
}

static const EVP_MD sha512_md = {
    NID_sha512,
    NID_sha512WithRSAEncryption,
    SHA512_DIGEST_LENGTH,
    EVP_MD_FLAG_DIGALGID_ABSENT,
    init512,
    update512,
    final512,
    NULL,
    NULL,
    SHA512_CBLOCK,
    sizeof(EVP_MD *) + sizeof(SHA512_CTX),
};

const EVP_MD *EVP_sha512(void)
{
    return (&sha512_md);
}
