/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#ifndef HEADER_CRYPTLIB_H
# define HEADER_CRYPTLIB_H

# include <stdlib.h>
# include <string.h>

# include BLIK_OPENSSL_U_e_os_h //original-code:"e_os.h"

# ifdef OPENSSL_USE_APPLINK
#  undef BIO_FLAGS_UPLINK
#  define BIO_FLAGS_UPLINK 0x8000
#  include "ms/uplink.h"
# endif

# include BLIK_OPENSSL_V_openssl__crypto_h //original-code:<openssl/crypto.h>
# include BLIK_OPENSSL_V_openssl__buffer_h //original-code:<openssl/buffer.h>
# include BLIK_OPENSSL_V_openssl__bio_h //original-code:<openssl/bio.h>
# include BLIK_OPENSSL_V_openssl__err_h //original-code:<openssl/err.h>

#ifdef  __cplusplus
extern "C" {
#endif

typedef struct ex_callback_st EX_CALLBACK;

DEFINE_STACK_OF(EX_CALLBACK)

typedef struct app_mem_info_st APP_INFO;

typedef struct mem_st MEM;
DEFINE_LHASH_OF(MEM);

# ifndef OPENSSL_SYS_VMS
#  define X509_CERT_AREA          OPENSSLDIR
#  define X509_CERT_DIR           OPENSSLDIR "/certs"
#  define X509_CERT_FILE          OPENSSLDIR "/cert.pem"
#  define X509_PRIVATE_DIR        OPENSSLDIR "/private"
#  define CTLOG_FILE              OPENSSLDIR "/ct_log_list.cnf"
# else
#  define X509_CERT_AREA          "OSSL$DATAROOT:[000000]"
#  define X509_CERT_DIR           "OSSL$DATAROOT:[CERTS]"
#  define X509_CERT_FILE          "OSSL$DATAROOT:[000000]cert.pem"
#  define X509_PRIVATE_DIR        "OSSL$DATAROOT:[PRIVATE]"
#  define CTLOG_FILE              "OSSL$DATAROOT:[000000]ct_log_list.cnf"
# endif

# define X509_CERT_DIR_EVP        "SSL_CERT_DIR"
# define X509_CERT_FILE_EVP       "SSL_CERT_FILE"
# define CTLOG_FILE_EVP           "CTLOG_FILE"

/* size of string representations */
# define DECIMAL_SIZE(type)      ((sizeof(type)*8+2)/3+1)
# define HEX_SIZE(type)          (sizeof(type)*2)

void OPENSSL_cpuid_setup(void);
extern unsigned int OPENSSL_ia32cap_P[];
void OPENSSL_showfatal(const char *fmta, ...);
extern int OPENSSL_NONPIC_relocated;
void crypto_cleanup_all_ex_data_int(void);

int openssl_strerror_r(int errnum, char *buf, size_t buflen);
# if !defined(OPENSSL_NO_STDIO)
FILE *openssl_fopen(const char *filename, const char *mode);
# else
void *openssl_fopen(const char *filename, const char *mode);
# endif

#ifdef  __cplusplus
}
#endif

#endif
