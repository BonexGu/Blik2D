/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 2012 - 2016, Daniel Stenberg, <daniel@haxx.se>, et al.
 * Copyright (C) 2010 - 2011, Hoi-Ho Chan, <hoiho.chan@gmail.com>
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at https://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/

/*
 * Source file for all PolarSSL-specific code for the TLS/SSL layer. No code
 * but vtls.c should ever call or use these functions.
 *
 */

#include BLIK_CURL_U_curl_setup_h //original-code:"curl_setup.h"

#ifdef USE_POLARSSL

#include <polarssl/net.h>
#include <polarssl/ssl.h>
#include <polarssl/certs.h>
#include <polarssl/x509.h>
#include <polarssl/version.h>
#include <polarssl/sha256.h>

#if POLARSSL_VERSION_NUMBER < 0x01030000
#error too old PolarSSL
#endif

#include <polarssl/error.h>
#include <polarssl/entropy.h>
#include <polarssl/ctr_drbg.h>

#include BLIK_CURL_U_urldata_h //original-code:"urldata.h"
#include BLIK_CURL_U_sendf_h //original-code:"sendf.h"
#include BLIK_CURL_U_inet_pton_h //original-code:"inet_pton.h"
#include "polarssl.h"
#include "vtls.h"
#include BLIK_CURL_U_parsedate_h //original-code:"parsedate.h"
#include BLIK_CURL_U_connect_h //original-code:"connect.h" /* for the connect timeout */
#include BLIK_CURL_U_select_h //original-code:"select.h"
#include BLIK_CURL_U_strcase_h //original-code:"strcase.h"
#include "polarssl_threadlock.h"
#include BLIK_CURL_U_curl_printf_h //original-code:"curl_printf.h"
#include BLIK_CURL_U_curl_memory_h //original-code:"curl_memory.h"
/* The last #include file should be: */
#include BLIK_CURL_U_memdebug_h //original-code:"memdebug.h"

/* See https://tls.mbed.org/discussions/generic/
   howto-determine-exact-buffer-len-for-mbedtls_pk_write_pubkey_der
*/
#define RSA_PUB_DER_MAX_BYTES   (38 + 2 * POLARSSL_MPI_MAX_SIZE)
#define ECP_PUB_DER_MAX_BYTES   (30 + 2 * POLARSSL_ECP_MAX_BYTES)

#define PUB_DER_MAX_BYTES   (RSA_PUB_DER_MAX_BYTES > ECP_PUB_DER_MAX_BYTES ? \
                             RSA_PUB_DER_MAX_BYTES : ECP_PUB_DER_MAX_BYTES)

/* apply threading? */
#if defined(USE_THREADS_POSIX) || defined(USE_THREADS_WIN32)
#define THREADING_SUPPORT
#endif

#ifndef POLARSSL_ERROR_C
#define error_strerror(x,y,z)
#endif /* POLARSSL_ERROR_C */


#if defined(THREADING_SUPPORT)
static entropy_context entropy;

static int  entropy_init_initialized  = 0;

/* start of entropy_init_mutex() */
static void entropy_init_mutex(entropy_context *ctx)
{
  /* lock 0 = entropy_init_mutex() */
  Curl_polarsslthreadlock_lock_function(0);
  if(entropy_init_initialized == 0) {
    entropy_init(ctx);
    entropy_init_initialized = 1;
  }
  Curl_polarsslthreadlock_unlock_function(0);
}
/* end of entropy_init_mutex() */

/* start of entropy_func_mutex() */
static int entropy_func_mutex(void *data, unsigned char *output, size_t len)
{
  int ret;
  /* lock 1 = entropy_func_mutex() */
  Curl_polarsslthreadlock_lock_function(1);
  ret = entropy_func(data, output, len);
  Curl_polarsslthreadlock_unlock_function(1);

  return ret;
}
/* end of entropy_func_mutex() */

#endif /* THREADING_SUPPORT */

/* Define this to enable lots of debugging for PolarSSL */
#undef POLARSSL_DEBUG

#ifdef POLARSSL_DEBUG
static void polarssl_debug(void *context, int level, const char *line)
{
  struct Curl_easy *data = NULL;

  if(!context)
    return;

  data = (struct Curl_easy *)context;

  infof(data, "%s", line);
  (void) level;
}
#else
#endif

/* ALPN for http2? */
#ifdef POLARSSL_SSL_ALPN
#  define HAS_ALPN
#endif

static Curl_recv polarssl_recv;
static Curl_send polarssl_send;


static CURLcode
polarssl_connect_step1(struct connectdata *conn,
                       int sockindex)
{
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data* connssl = &conn->ssl[sockindex];
  int ret = -1;
  char errorbuf[128];
  errorbuf[0]=0;

  /* PolarSSL only supports SSLv3 and TLSv1 */
  if(data->set.ssl.version == CURL_SSLVERSION_SSLv2) {
    failf(data, "PolarSSL does not support SSLv2");
    return CURLE_SSL_CONNECT_ERROR;
  }

#ifdef THREADING_SUPPORT
  entropy_init_mutex(&entropy);

  if((ret = ctr_drbg_init(&connssl->ctr_drbg, entropy_func_mutex, &entropy,
                          NULL, 0)) != 0) {
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
          -ret, errorbuf);
  }
#else
  entropy_init(&connssl->entropy);

  if((ret = ctr_drbg_init(&connssl->ctr_drbg, entropy_func, &connssl->entropy,
                          NULL, 0)) != 0) {
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "Failed - PolarSSL: ctr_drbg_init returned (-0x%04X) %s\n",
          -ret, errorbuf);
  }
#endif /* THREADING_SUPPORT */

  /* Load the trusted CA */
  memset(&connssl->cacert, 0, sizeof(x509_crt));

  if(data->set.str[STRING_SSL_CAFILE]) {
    ret = x509_crt_parse_file(&connssl->cacert,
                              data->set.str[STRING_SSL_CAFILE]);

    if(ret<0) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading ca cert file %s - PolarSSL: (-0x%04X) %s",
            data->set.str[STRING_SSL_CAFILE], -ret, errorbuf);

      if(data->set.ssl.verifypeer)
        return CURLE_SSL_CACERT_BADFILE;
    }
  }

  if(data->set.str[STRING_SSL_CAPATH]) {
    ret = x509_crt_parse_path(&connssl->cacert,
                              data->set.str[STRING_SSL_CAPATH]);

    if(ret<0) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading ca cert path %s - PolarSSL: (-0x%04X) %s",
            data->set.str[STRING_SSL_CAPATH], -ret, errorbuf);

      if(data->set.ssl.verifypeer)
        return CURLE_SSL_CACERT_BADFILE;
    }
  }

  /* Load the client certificate */
  memset(&connssl->clicert, 0, sizeof(x509_crt));

  if(data->set.str[STRING_CERT]) {
    ret = x509_crt_parse_file(&connssl->clicert,
                              data->set.str[STRING_CERT]);

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading client cert file %s - PolarSSL: (-0x%04X) %s",
            data->set.str[STRING_CERT], -ret, errorbuf);

      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Load the client private key */
  if(data->set.str[STRING_KEY]) {
    pk_context pk;
    pk_init(&pk);
    ret = pk_parse_keyfile(&pk, data->set.str[STRING_KEY],
                           data->set.str[STRING_KEY_PASSWD]);
    if(ret == 0 && !pk_can_do(&pk, POLARSSL_PK_RSA))
      ret = POLARSSL_ERR_PK_TYPE_MISMATCH;
    if(ret == 0)
      rsa_copy(&connssl->rsa, pk_rsa(pk));
    else
      rsa_free(&connssl->rsa);
    pk_free(&pk);

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading private key %s - PolarSSL: (-0x%04X) %s",
            data->set.str[STRING_KEY], -ret, errorbuf);

      return CURLE_SSL_CERTPROBLEM;
    }
  }

  /* Load the CRL */
  memset(&connssl->crl, 0, sizeof(x509_crl));

  if(data->set.str[STRING_SSL_CRLFILE]) {
    ret = x509_crl_parse_file(&connssl->crl,
                              data->set.str[STRING_SSL_CRLFILE]);

    if(ret) {
      error_strerror(ret, errorbuf, sizeof(errorbuf));
      failf(data, "Error reading CRL file %s - PolarSSL: (-0x%04X) %s",
            data->set.str[STRING_SSL_CRLFILE], -ret, errorbuf);

      return CURLE_SSL_CRL_BADFILE;
    }
  }

  infof(data, "PolarSSL: Connecting to %s:%d\n",
        conn->host.name, conn->remote_port);

  if(ssl_init(&connssl->ssl)) {
    failf(data, "PolarSSL: ssl_init failed");
    return CURLE_SSL_CONNECT_ERROR;
  }

  switch(data->set.ssl.version) {
  default:
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    break;
  case CURL_SSLVERSION_SSLv3:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_0);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_0);
    infof(data, "PolarSSL: Forced min. SSL Version to be SSLv3\n");
    break;
  case CURL_SSLVERSION_TLSv1_0:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_1);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.0\n");
    break;
  case CURL_SSLVERSION_TLSv1_1:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_2);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_2);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.1\n");
    break;
  case CURL_SSLVERSION_TLSv1_2:
    ssl_set_min_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_3);
    ssl_set_max_version(&connssl->ssl, SSL_MAJOR_VERSION_3,
                        SSL_MINOR_VERSION_3);
    infof(data, "PolarSSL: Forced min. SSL Version to be TLS 1.2\n");
    break;
  }

  ssl_set_endpoint(&connssl->ssl, SSL_IS_CLIENT);
  ssl_set_authmode(&connssl->ssl, SSL_VERIFY_OPTIONAL);

  ssl_set_rng(&connssl->ssl, ctr_drbg_random,
              &connssl->ctr_drbg);
  ssl_set_bio(&connssl->ssl,
              net_recv, &conn->sock[sockindex],
              net_send, &conn->sock[sockindex]);

  ssl_set_ciphersuites(&connssl->ssl, ssl_list_ciphersuites());

  /* Check if there's a cached ID we can/should use here! */
  if(conn->ssl_config.sessionid) {
    void *old_session = NULL;

    Curl_ssl_sessionid_lock(conn);
    if(!Curl_ssl_getsessionid(conn, &old_session, NULL)) {
      ret = ssl_set_session(&connssl->ssl, old_session);
      if(ret) {
        Curl_ssl_sessionid_unlock(conn);
        failf(data, "ssl_set_session returned -0x%x", -ret);
        return CURLE_SSL_CONNECT_ERROR;
      }
      infof(data, "PolarSSL re-using session\n");
    }
    Curl_ssl_sessionid_unlock(conn);
  }

  ssl_set_ca_chain(&connssl->ssl,
                   &connssl->cacert,
                   &connssl->crl,
                   conn->host.name);

  ssl_set_own_cert_rsa(&connssl->ssl,
                       &connssl->clicert, &connssl->rsa);

  if(ssl_set_hostname(&connssl->ssl, conn->host.name)) {
    /* ssl_set_hostname() sets the name to use in CN/SAN checks *and* the name
       to set in the SNI extension. So even if curl connects to a host
       specified as an IP address, this function must be used. */
    failf(data, "couldn't set hostname in PolarSSL");
    return CURLE_SSL_CONNECT_ERROR;
  }

#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    static const char* protocols[3];
    int cur = 0;

#ifdef USE_NGHTTP2
    if(data->set.httpversion >= CURL_HTTP_VERSION_2) {
      protocols[cur++] = NGHTTP2_PROTO_VERSION_ID;
      infof(data, "ALPN, offering %s\n", NGHTTP2_PROTO_VERSION_ID);
    }
#endif

    protocols[cur++] = ALPN_HTTP_1_1;
    infof(data, "ALPN, offering %s\n", ALPN_HTTP_1_1);

    protocols[cur] = NULL;

    ssl_set_alpn_protocols(&connssl->ssl, protocols);
  }
#endif

#ifdef POLARSSL_DEBUG
  ssl_set_dbg(&connssl->ssl, polarssl_debug, data);
#endif

  connssl->connecting_state = ssl_connect_2;

  return CURLE_OK;
}

static CURLcode
polarssl_connect_step2(struct connectdata *conn,
                       int sockindex)
{
  int ret;
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data* connssl = &conn->ssl[sockindex];
  char buffer[1024];

  char errorbuf[128];
  errorbuf[0] = 0;

  conn->recv[sockindex] = polarssl_recv;
  conn->send[sockindex] = polarssl_send;

  ret = ssl_handshake(&connssl->ssl);

  switch(ret) {
  case 0:
    break;

  case POLARSSL_ERR_NET_WANT_READ:
    connssl->connecting_state = ssl_connect_2_reading;
    return CURLE_OK;

  case POLARSSL_ERR_NET_WANT_WRITE:
    connssl->connecting_state = ssl_connect_2_writing;
    return CURLE_OK;

  default:
    error_strerror(ret, errorbuf, sizeof(errorbuf));
    failf(data, "ssl_handshake returned - PolarSSL: (-0x%04X) %s",
          -ret, errorbuf);
    return CURLE_SSL_CONNECT_ERROR;
  }

  infof(data, "PolarSSL: Handshake complete, cipher is %s\n",
        ssl_get_ciphersuite(&conn->ssl[sockindex].ssl) );

  ret = ssl_get_verify_result(&conn->ssl[sockindex].ssl);

  if(ret && data->set.ssl.verifypeer) {
    if(ret & BADCERT_EXPIRED)
      failf(data, "Cert verify failed: BADCERT_EXPIRED");

    if(ret & BADCERT_REVOKED) {
      failf(data, "Cert verify failed: BADCERT_REVOKED");
      return CURLE_SSL_CACERT;
    }

    if(ret & BADCERT_CN_MISMATCH)
      failf(data, "Cert verify failed: BADCERT_CN_MISMATCH");

    if(ret & BADCERT_NOT_TRUSTED)
      failf(data, "Cert verify failed: BADCERT_NOT_TRUSTED");

    return CURLE_PEER_FAILED_VERIFICATION;
  }

  if(ssl_get_peer_cert(&(connssl->ssl))) {
    /* If the session was resumed, there will be no peer certs */
    memset(buffer, 0, sizeof(buffer));

    if(x509_crt_info(buffer, sizeof(buffer), (char *)"* ",
                     ssl_get_peer_cert(&(connssl->ssl))) != -1)
      infof(data, "Dumping cert info:\n%s\n", buffer);
  }

  /* adapted from mbedtls.c */
  if(data->set.str[STRING_SSL_PINNEDPUBLICKEY]) {
    int size;
    CURLcode result;
    x509_crt *p;
    unsigned char pubkey[PUB_DER_MAX_BYTES];
    const x509_crt *peercert;

    peercert = ssl_get_peer_cert(&connssl->ssl);

    if(!peercert || !peercert->raw.p || !peercert->raw.len) {
      failf(data, "Failed due to missing peer certificate");
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    p = calloc(1, sizeof(*p));

    if(!p)
      return CURLE_OUT_OF_MEMORY;

    x509_crt_init(p);

    /* Make a copy of our const peercert because pk_write_pubkey_der
       needs a non-const key, for now.
       https://github.com/ARMmbed/mbedtls/issues/396 */
    if(x509_crt_parse_der(p, peercert->raw.p, peercert->raw.len)) {
      failf(data, "Failed copying peer certificate");
      x509_crt_free(p);
      free(p);
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    size = pk_write_pubkey_der(&p->pk, pubkey, PUB_DER_MAX_BYTES);

    if(size <= 0) {
      failf(data, "Failed copying public key from peer certificate");
      x509_crt_free(p);
      free(p);
      return CURLE_SSL_PINNEDPUBKEYNOTMATCH;
    }

    /* pk_write_pubkey_der writes data at the end of the buffer. */
    result = Curl_pin_peer_pubkey(data,
                                  data->set.str[STRING_SSL_PINNEDPUBLICKEY],
                                  &pubkey[PUB_DER_MAX_BYTES - size], size);
    if(result) {
      x509_crt_free(p);
      free(p);
      return result;
    }

    x509_crt_free(p);
    free(p);
  }

#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    const char *next_protocol = ssl_get_alpn_protocol(&connssl->ssl);

    if(next_protocol != NULL) {
      infof(data, "ALPN, server accepted to use %s\n", next_protocol);

#ifdef USE_NGHTTP2
      if(!strncmp(next_protocol, NGHTTP2_PROTO_VERSION_ID,
                  NGHTTP2_PROTO_VERSION_ID_LEN)) {
        conn->negnpn = CURL_HTTP_VERSION_2;
      }
      else
#endif
        if(!strncmp(next_protocol, ALPN_HTTP_1_1, ALPN_HTTP_1_1_LENGTH)) {
          conn->negnpn = CURL_HTTP_VERSION_1_1;
        }
    }
    else
      infof(data, "ALPN, server did not agree to a protocol\n");
  }
#endif

  connssl->connecting_state = ssl_connect_3;
  infof(data, "SSL connected\n");

  return CURLE_OK;
}

static CURLcode
polarssl_connect_step3(struct connectdata *conn,
                       int sockindex)
{
  CURLcode retcode = CURLE_OK;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  struct Curl_easy *data = conn->data;

  DEBUGASSERT(ssl_connect_3 == connssl->connecting_state);

  if(conn->ssl_config.sessionid) {
    int ret;
    ssl_session *our_ssl_sessionid;
    void *old_ssl_sessionid = NULL;

    our_ssl_sessionid = malloc(sizeof(ssl_session));
    if(!our_ssl_sessionid)
      return CURLE_OUT_OF_MEMORY;

    ssl_session_init(our_ssl_sessionid);

    ret = ssl_get_session(&connssl->ssl, our_ssl_sessionid);
    if(ret) {
      failf(data, "ssl_get_session returned -0x%x", -ret);
      return CURLE_SSL_CONNECT_ERROR;
    }

    /* If there's already a matching session in the cache, delete it */
    Curl_ssl_sessionid_lock(conn);
    if(!Curl_ssl_getsessionid(conn, &old_ssl_sessionid, NULL))
      Curl_ssl_delsessionid(conn, old_ssl_sessionid);

    retcode = Curl_ssl_addsessionid(conn, our_ssl_sessionid, 0);
    Curl_ssl_sessionid_unlock(conn);
    if(retcode) {
      free(our_ssl_sessionid);
      failf(data, "failed to store ssl session");
      return retcode;
    }
  }

  connssl->connecting_state = ssl_connect_done;

  return CURLE_OK;
}

static ssize_t polarssl_send(struct connectdata *conn,
                             int sockindex,
                             const void *mem,
                             size_t len,
                             CURLcode *curlcode)
{
  int ret = -1;

  ret = ssl_write(&conn->ssl[sockindex].ssl,
                  (unsigned char *)mem, len);

  if(ret < 0) {
    *curlcode = (ret == POLARSSL_ERR_NET_WANT_WRITE) ?
      CURLE_AGAIN : CURLE_SEND_ERROR;
    ret = -1;
  }

  return ret;
}

void Curl_polarssl_close(struct connectdata *conn, int sockindex)
{
  rsa_free(&conn->ssl[sockindex].rsa);
  x509_crt_free(&conn->ssl[sockindex].clicert);
  x509_crt_free(&conn->ssl[sockindex].cacert);
  x509_crl_free(&conn->ssl[sockindex].crl);
  ssl_free(&conn->ssl[sockindex].ssl);
}

static ssize_t polarssl_recv(struct connectdata *conn,
                             int num,
                             char *buf,
                             size_t buffersize,
                             CURLcode *curlcode)
{
  int ret = -1;
  ssize_t len = -1;

  memset(buf, 0, buffersize);
  ret = ssl_read(&conn->ssl[num].ssl, (unsigned char *)buf, buffersize);

  if(ret <= 0) {
    if(ret == POLARSSL_ERR_SSL_PEER_CLOSE_NOTIFY)
      return 0;

    *curlcode = (ret == POLARSSL_ERR_NET_WANT_READ) ?
      CURLE_AGAIN : CURLE_RECV_ERROR;
    return -1;
  }

  len = ret;

  return len;
}

void Curl_polarssl_session_free(void *ptr)
{
  ssl_session_free(ptr);
  free(ptr);
}

/* 1.3.10 was the first rebranded version. All new releases (in 1.3 branch and
   higher) will be mbed TLS branded.. */

size_t Curl_polarssl_version(char *buffer, size_t size)
{
  unsigned int version = version_get_number();
  return snprintf(buffer, size, "%s/%d.%d.%d",
                  version >= 0x01030A00?"mbedTLS":"PolarSSL",
                  version>>24, (version>>16)&0xff, (version>>8)&0xff);
}

static CURLcode
polarssl_connect_common(struct connectdata *conn,
                        int sockindex,
                        bool nonblocking,
                        bool *done)
{
  CURLcode result;
  struct Curl_easy *data = conn->data;
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  curl_socket_t sockfd = conn->sock[sockindex];
  long timeout_ms;
  int what;

  /* check if the connection has already been established */
  if(ssl_connection_complete == connssl->state) {
    *done = TRUE;
    return CURLE_OK;
  }

  if(ssl_connect_1 == connssl->connecting_state) {
    /* Find out how much more time we're allowed */
    timeout_ms = Curl_timeleft(data, NULL, TRUE);

    if(timeout_ms < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }

    result = polarssl_connect_step1(conn, sockindex);
    if(result)
      return result;
  }

  while(ssl_connect_2 == connssl->connecting_state ||
        ssl_connect_2_reading == connssl->connecting_state ||
        ssl_connect_2_writing == connssl->connecting_state) {

    /* check allowed time left */
    timeout_ms = Curl_timeleft(data, NULL, TRUE);

    if(timeout_ms < 0) {
      /* no need to continue if time already is up */
      failf(data, "SSL connection timeout");
      return CURLE_OPERATION_TIMEDOUT;
    }

    /* if ssl is expecting something, check if it's available. */
    if(connssl->connecting_state == ssl_connect_2_reading ||
       connssl->connecting_state == ssl_connect_2_writing) {

      curl_socket_t writefd = ssl_connect_2_writing==
        connssl->connecting_state?sockfd:CURL_SOCKET_BAD;
      curl_socket_t readfd = ssl_connect_2_reading==
        connssl->connecting_state?sockfd:CURL_SOCKET_BAD;

      what = Curl_socket_check(readfd, CURL_SOCKET_BAD, writefd,
                               nonblocking?0:timeout_ms);
      if(what < 0) {
        /* fatal error */
        failf(data, "select/poll on SSL socket, errno: %d", SOCKERRNO);
        return CURLE_SSL_CONNECT_ERROR;
      }
      else if(0 == what) {
        if(nonblocking) {
          *done = FALSE;
          return CURLE_OK;
        }
        else {
          /* timeout */
          failf(data, "SSL connection timeout");
          return CURLE_OPERATION_TIMEDOUT;
        }
      }
      /* socket is readable or writable */
    }

    /* Run transaction, and return to the caller if it failed or if
     * this connection is part of a multi handle and this loop would
     * execute again. This permits the owner of a multi handle to
     * abort a connection attempt before step2 has completed while
     * ensuring that a client using select() or epoll() will always
     * have a valid fdset to wait on.
     */
    result = polarssl_connect_step2(conn, sockindex);
    if(result || (nonblocking &&
                  (ssl_connect_2 == connssl->connecting_state ||
                   ssl_connect_2_reading == connssl->connecting_state ||
                   ssl_connect_2_writing == connssl->connecting_state)))
      return result;

  } /* repeat step2 until all transactions are done. */

  if(ssl_connect_3 == connssl->connecting_state) {
    result = polarssl_connect_step3(conn, sockindex);
    if(result)
      return result;
  }

  if(ssl_connect_done == connssl->connecting_state) {
    connssl->state = ssl_connection_complete;
    conn->recv[sockindex] = polarssl_recv;
    conn->send[sockindex] = polarssl_send;
    *done = TRUE;
  }
  else
    *done = FALSE;

  /* Reset our connect state machine */
  connssl->connecting_state = ssl_connect_1;

  return CURLE_OK;
}

CURLcode
Curl_polarssl_connect_nonblocking(struct connectdata *conn,
                                  int sockindex,
                                  bool *done)
{
  return polarssl_connect_common(conn, sockindex, TRUE, done);
}


CURLcode
Curl_polarssl_connect(struct connectdata *conn,
                      int sockindex)
{
  CURLcode result;
  bool done = FALSE;

  result = polarssl_connect_common(conn, sockindex, FALSE, &done);
  if(result)
    return result;

  DEBUGASSERT(done);

  return CURLE_OK;
}

/*
 * return 0 error initializing SSL
 * return 1 SSL initialized successfully
 */
int Curl_polarssl_init(void)
{
  return Curl_polarsslthreadlock_thread_setup();
}

void Curl_polarssl_cleanup(void)
{
  (void)Curl_polarsslthreadlock_thread_cleanup();
}

#endif /* USE_POLARSSL */
