/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_sys_git_reflog_h__
#define INCLUDE_sys_git_reflog_h__

#include BLIK_LIBGIT2_U_git2__common_h //original-code:"git2/common.h"
#include BLIK_LIBGIT2_U_git2__types_h //original-code:"git2/types.h"
#include BLIK_LIBGIT2_U_git2__oid_h //original-code:"git2/oid.h"

GIT_BEGIN_DECL

GIT_EXTERN(git_reflog_entry *) git_reflog_entry__alloc(void);
GIT_EXTERN(void) git_reflog_entry__free(git_reflog_entry *entry);

GIT_END_DECL

#endif
