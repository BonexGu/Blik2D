/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2015, Daniel Stenberg, <daniel@haxx.se>, et al.
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

#include BLIK_CURL_U_curl_setup_h //original-code:"curl_setup.h"

#include BLIK_CURL_V_curl__curl_h //original-code:<curl/curl.h>

#include BLIK_CURL_U_urldata_h //original-code:"urldata.h"
#include BLIK_CURL_U_getinfo_h //original-code:"getinfo.h"

#include BLIK_CURL_U_vtls__vtls_h //original-code:"vtls/vtls.h"
#include BLIK_CURL_U_connect_h //original-code:"connect.h" /* Curl_getconnectinfo() */
#include BLIK_CURL_U_progress_h //original-code:"progress.h"

/* The last #include files should be: */
#include BLIK_CURL_U_curl_memory_h //original-code:"curl_memory.h"
#include BLIK_CURL_U_memdebug_h //original-code:"memdebug.h"

/*
 * This is supposed to be called in the beginning of a perform() session and
 * in curl_easy_reset() and should reset all session-info variables.
 */
CURLcode Curl_initinfo(struct Curl_easy *data)
{
  struct Progress *pro = &data->progress;
  struct PureInfo *info = &data->info;

  pro->t_nslookup = 0;
  pro->t_connect = 0;
  pro->t_appconnect = 0;
  pro->t_pretransfer = 0;
  pro->t_starttransfer = 0;
  pro->timespent = 0;
  pro->t_redirect = 0;

  info->httpcode = 0;
  info->httpproxycode = 0;
  info->httpversion = 0;
  info->filetime = -1; /* -1 is an illegal time and thus means unknown */
  info->timecond = FALSE;

  info->header_size = 0;
  info->request_size = 0;
  info->proxyauthavail = 0;
  info->httpauthavail = 0;
  info->numconnects = 0;

  free(info->contenttype);
  info->contenttype = NULL;

  free(info->wouldredirect);
  info->wouldredirect = NULL;

  info->conn_primary_ip[0] = '\0';
  info->conn_local_ip[0] = '\0';
  info->conn_primary_port = 0;
  info->conn_local_port = 0;

#ifdef USE_SSL
  Curl_ssl_free_certinfo(data);
#endif

  return CURLE_OK;
}

static CURLcode getinfo_char(struct Curl_easy *data, CURLINFO info,
                             char **param_charp)
{
  switch(info) {
  case CURLINFO_EFFECTIVE_URL:
    *param_charp = data->change.url?data->change.url:(char *)"";
    break;
  case CURLINFO_CONTENT_TYPE:
    *param_charp = data->info.contenttype;
    break;
  case CURLINFO_PRIVATE:
    *param_charp = (char *) data->set.private_data;
    break;
  case CURLINFO_FTP_ENTRY_PATH:
    /* Return the entrypath string from the most recent connection.
       This pointer was copied from the connectdata structure by FTP.
       The actual string may be free()ed by subsequent libcurl calls so
       it must be copied to a safer area before the next libcurl call.
       Callers must never free it themselves. */
    *param_charp = data->state.most_recent_ftp_entrypath;
    break;
  case CURLINFO_REDIRECT_URL:
    /* Return the URL this request would have been redirected to if that
       option had been enabled! */
    *param_charp = data->info.wouldredirect;
    break;
  case CURLINFO_PRIMARY_IP:
    /* Return the ip address of the most recent (primary) connection */
    *param_charp = data->info.conn_primary_ip;
    break;
  case CURLINFO_LOCAL_IP:
    /* Return the source/local ip address of the most recent (primary)
       connection */
    *param_charp = data->info.conn_local_ip;
    break;
  case CURLINFO_RTSP_SESSION_ID:
    *param_charp = data->set.str[STRING_RTSP_SESSION_ID];
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_long(struct Curl_easy *data, CURLINFO info,
                             long *param_longp)
{
  curl_socket_t sockfd;

  union {
    unsigned long *to_ulong;
    long          *to_long;
  } lptr;

  switch(info) {
  case CURLINFO_RESPONSE_CODE:
    *param_longp = data->info.httpcode;
    break;
  case CURLINFO_HTTP_CONNECTCODE:
    *param_longp = data->info.httpproxycode;
    break;
  case CURLINFO_FILETIME:
    *param_longp = data->info.filetime;
    break;
  case CURLINFO_HEADER_SIZE:
    *param_longp = data->info.header_size;
    break;
  case CURLINFO_REQUEST_SIZE:
    *param_longp = data->info.request_size;
    break;
  case CURLINFO_SSL_VERIFYRESULT:
    *param_longp = data->set.ssl.certverifyresult;
    break;
  case CURLINFO_REDIRECT_COUNT:
    *param_longp = data->set.followlocation;
    break;
  case CURLINFO_HTTPAUTH_AVAIL:
    lptr.to_long = param_longp;
    *lptr.to_ulong = data->info.httpauthavail;
    break;
  case CURLINFO_PROXYAUTH_AVAIL:
    lptr.to_long = param_longp;
    *lptr.to_ulong = data->info.proxyauthavail;
    break;
  case CURLINFO_OS_ERRNO:
    *param_longp = data->state.os_errno;
    break;
  case CURLINFO_NUM_CONNECTS:
    *param_longp = data->info.numconnects;
    break;
  case CURLINFO_LASTSOCKET:
    sockfd = Curl_getconnectinfo(data, NULL);

    /* note: this is not a good conversion for systems with 64 bit sockets and
       32 bit longs */
    if(sockfd != CURL_SOCKET_BAD)
      *param_longp = (long)sockfd;
    else
      /* this interface is documented to return -1 in case of badness, which
         may not be the same as the CURL_SOCKET_BAD value */
      *param_longp = -1;
    break;
  case CURLINFO_PRIMARY_PORT:
    /* Return the (remote) port of the most recent (primary) connection */
    *param_longp = data->info.conn_primary_port;
    break;
  case CURLINFO_LOCAL_PORT:
    /* Return the local port of the most recent (primary) connection */
    *param_longp = data->info.conn_local_port;
    break;
  case CURLINFO_CONDITION_UNMET:
    /* return if the condition prevented the document to get transferred */
    *param_longp = data->info.timecond ? 1L : 0L;
    break;
  case CURLINFO_RTSP_CLIENT_CSEQ:
    *param_longp = data->state.rtsp_next_client_CSeq;
    break;
  case CURLINFO_RTSP_SERVER_CSEQ:
    *param_longp = data->state.rtsp_next_server_CSeq;
    break;
  case CURLINFO_RTSP_CSEQ_RECV:
    *param_longp = data->state.rtsp_CSeq_recv;
    break;
  case CURLINFO_HTTP_VERSION:
    switch (data->info.httpversion) {
    case 10:
      *param_longp = CURL_HTTP_VERSION_1_0;
      break;
    case 11:
      *param_longp = CURL_HTTP_VERSION_1_1;
      break;
    case 20:
      *param_longp = CURL_HTTP_VERSION_2_0;
      break;
    default:
      *param_longp = CURL_HTTP_VERSION_NONE;
      break;
    }
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_double(struct Curl_easy *data, CURLINFO info,
                               double *param_doublep)
{
  switch(info) {
  case CURLINFO_TOTAL_TIME:
    *param_doublep = data->progress.timespent;
    break;
  case CURLINFO_NAMELOOKUP_TIME:
    *param_doublep = data->progress.t_nslookup;
    break;
  case CURLINFO_CONNECT_TIME:
    *param_doublep = data->progress.t_connect;
    break;
  case CURLINFO_APPCONNECT_TIME:
    *param_doublep = data->progress.t_appconnect;
    break;
  case CURLINFO_PRETRANSFER_TIME:
    *param_doublep =  data->progress.t_pretransfer;
    break;
  case CURLINFO_STARTTRANSFER_TIME:
    *param_doublep = data->progress.t_starttransfer;
    break;
  case CURLINFO_SIZE_UPLOAD:
    *param_doublep =  (double)data->progress.uploaded;
    break;
  case CURLINFO_SIZE_DOWNLOAD:
    *param_doublep = (double)data->progress.downloaded;
    break;
  case CURLINFO_SPEED_DOWNLOAD:
    *param_doublep =  (double)data->progress.dlspeed;
    break;
  case CURLINFO_SPEED_UPLOAD:
    *param_doublep = (double)data->progress.ulspeed;
    break;
  case CURLINFO_CONTENT_LENGTH_DOWNLOAD:
    *param_doublep = (data->progress.flags & PGRS_DL_SIZE_KNOWN)?
      (double)data->progress.size_dl:-1;
    break;
  case CURLINFO_CONTENT_LENGTH_UPLOAD:
    *param_doublep = (data->progress.flags & PGRS_UL_SIZE_KNOWN)?
      (double)data->progress.size_ul:-1;
    break;
  case CURLINFO_REDIRECT_TIME:
    *param_doublep =  data->progress.t_redirect;
    break;

  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_slist(struct Curl_easy *data, CURLINFO info,
                              struct curl_slist **param_slistp)
{
  union {
    struct curl_certinfo *to_certinfo;
    struct curl_slist    *to_slist;
  } ptr;

  switch(info) {
  case CURLINFO_SSL_ENGINES:
    *param_slistp = Curl_ssl_engines_list(data);
    break;
  case CURLINFO_COOKIELIST:
    *param_slistp = Curl_cookie_list(data);
    break;
  case CURLINFO_CERTINFO:
    /* Return the a pointer to the certinfo struct. Not really an slist
       pointer but we can pretend it is here */
    ptr.to_certinfo = &data->info.certs;
    *param_slistp = ptr.to_slist;
    break;
  case CURLINFO_TLS_SESSION:
  case CURLINFO_TLS_SSL_PTR:
    {
      struct curl_tlssessioninfo **tsip = (struct curl_tlssessioninfo **)
                                          param_slistp;
      struct curl_tlssessioninfo *tsi = &data->tsi;
      struct connectdata *conn = data->easy_conn;

      *tsip = tsi;
      tsi->backend = Curl_ssl_backend();
      tsi->internals = NULL;

      if(conn && tsi->backend != CURLSSLBACKEND_NONE) {
        unsigned int i;
        for(i = 0; i < (sizeof(conn->ssl) / sizeof(conn->ssl[0])); ++i) {
          if(conn->ssl[i].use) {
#if defined(USE_AXTLS)
            tsi->internals = (void *)conn->ssl[i].ssl;
#elif defined(USE_CYASSL)
            tsi->internals = (void *)conn->ssl[i].handle;
#elif defined(USE_DARWINSSL)
            tsi->internals = (void *)conn->ssl[i].ssl_ctx;
#elif defined(USE_GNUTLS)
            tsi->internals = (void *)conn->ssl[i].session;
#elif defined(USE_GSKIT)
            tsi->internals = (void *)conn->ssl[i].handle;
#elif defined(USE_MBEDTLS)
            tsi->internals = (void *)&conn->ssl[i].ssl;
#elif defined(USE_NSS)
            tsi->internals = (void *)conn->ssl[i].handle;
#elif defined(USE_OPENSSL)
            /* Legacy: CURLINFO_TLS_SESSION must return an SSL_CTX pointer. */
            tsi->internals = ((info == CURLINFO_TLS_SESSION) ?
                              (void *)conn->ssl[i].ctx :
                              (void *)conn->ssl[i].handle);
#elif defined(USE_POLARSSL)
            tsi->internals = (void *)&conn->ssl[i].ssl;
#elif defined(USE_SCHANNEL)
            tsi->internals = (void *)&conn->ssl[i].ctxt->ctxt_handle;
#elif defined(USE_SSL)
#error "SSL backend specific information missing for CURLINFO_TLS_SSL_PTR"
#endif
            break;
          }
        }
      }
    }
    break;
  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

static CURLcode getinfo_socket(struct Curl_easy *data, CURLINFO info,
                               curl_socket_t *param_socketp)
{
  switch(info) {
  case CURLINFO_ACTIVESOCKET:
    *param_socketp = Curl_getconnectinfo(data, NULL);
    break;
  default:
    return CURLE_UNKNOWN_OPTION;
  }

  return CURLE_OK;
}

CURLcode Curl_getinfo(struct Curl_easy *data, CURLINFO info, ...)
{
  va_list arg;
  long *param_longp = NULL;
  double *param_doublep = NULL;
  char **param_charp = NULL;
  struct curl_slist **param_slistp = NULL;
  curl_socket_t *param_socketp = NULL;
  int type;
  CURLcode result = CURLE_UNKNOWN_OPTION;

  if(!data)
    return result;

  va_start(arg, info);

  type = CURLINFO_TYPEMASK & (int)info;
  switch(type) {
  case CURLINFO_STRING:
    param_charp = va_arg(arg, char **);
    if(param_charp)
      result = getinfo_char(data, info, param_charp);
    break;
  case CURLINFO_LONG:
    param_longp = va_arg(arg, long *);
    if(param_longp)
      result = getinfo_long(data, info, param_longp);
    break;
  case CURLINFO_DOUBLE:
    param_doublep = va_arg(arg, double *);
    if(param_doublep)
      result = getinfo_double(data, info, param_doublep);
    break;
  case CURLINFO_SLIST:
    param_slistp = va_arg(arg, struct curl_slist **);
    if(param_slistp)
      result = getinfo_slist(data, info, param_slistp);
    break;
  case CURLINFO_SOCKET:
    param_socketp = va_arg(arg, curl_socket_t *);
    if(param_socketp)
      result = getinfo_socket(data, info, param_socketp);
    break;
  default:
    break;
  }

  va_end(arg);

  return result;
}
