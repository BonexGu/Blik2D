/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_posix__w32_h__
#define INCLUDE_posix__w32_h__

#include BLIK_LIBGIT2_U_common_h //original-code:"common.h"
#include "../posix.h"
#include "path_w32.h"
#include "utf-conv.h"
#include "dir.h"

typedef SOCKET GIT_SOCKET;

#define p_lseek(f,n,w) _lseeki64(f, n, w)
#define p_fstat(f,b) _fstat64(f, b)
extern int p_lstat(const char *file_name, struct stat *buf);
extern int p_stat(const char* path, struct stat* buf);

extern int p_readlink(const char *path, char *buf, size_t bufsiz);
extern int p_symlink(const char *old, const char *new);
extern int p_link(const char *old, const char *new);
extern int p_unlink(const char *path);
extern int p_mkdir(const char *path, mode_t mode);
extern int p_fsync(int fd);
extern char *p_realpath(const char *orig_path, char *buffer);

extern int p_recv(GIT_SOCKET socket, void *buffer, size_t length, int flags);
extern int p_send(GIT_SOCKET socket, const void *buffer, size_t length, int flags);
extern int p_inet_pton(int af, const char* src, void* dst);

#define strcasecmp(s1, s2) _stricmp(s1, s2)
#define strncasecmp(s1, s2, c) _strnicmp(s1, s2, c)
extern int p_vsnprintf(char *buffer, size_t count, const char *format, va_list argptr);
extern int p_snprintf(char *buffer, size_t count, const char *format, ...) GIT_FORMAT_PRINTF(3, 4);
extern int p_mkstemp(char *tmp_path);
extern int p_chdir(const char* path);
extern int p_chmod(const char* path, mode_t mode);
extern int p_rmdir(const char* path);
extern int p_access(const char* path, mode_t mode);
extern int p_ftruncate(int fd, long size);

/* p_lstat is almost but not quite POSIX correct.  Specifically, the use of
 * ENOTDIR is wrong, in that it does not mean precisely that a non-directory
 * entry was encountered.  Making it correct is potentially expensive,
 * however, so this is a separate version of p_lstat to use when correct
 * POSIX ENOTDIR semantics is required.
 */
extern int p_lstat_posixly(const char *filename, struct stat *buf);

extern struct tm * p_localtime_r (const time_t *timer, struct tm *result);
extern struct tm * p_gmtime_r (const time_t *timer, struct tm *result);

#endif
