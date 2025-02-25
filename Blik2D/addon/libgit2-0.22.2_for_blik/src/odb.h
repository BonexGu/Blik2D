/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_odb_h__
#define INCLUDE_odb_h__

#include BLIK_LIBGIT2_U_git2__odb_h //original-code:"git2/odb.h"
#include BLIK_LIBGIT2_U_git2__oid_h //original-code:"git2/oid.h"
#include BLIK_LIBGIT2_U_git2__types_h //original-code:"git2/types.h"

#include BLIK_LIBGIT2_U_vector_h //original-code:"vector.h"
#include "cache.h"
#include BLIK_LIBGIT2_U_posix_h //original-code:"posix.h"
#include "filter.h"

#define GIT_OBJECTS_DIR "objects/"
#define GIT_OBJECT_DIR_MODE 0777
#define GIT_OBJECT_FILE_MODE 0444

/* DO NOT EXPORT */
typedef struct {
	void *data;			/**< Raw, decompressed object data. */
	size_t len;			/**< Total number of bytes in data. */
	git_otype type;		/**< Type of this object. */
} git_rawobj;

/* EXPORT */
struct git_odb_object {
	git_cached_obj cached;
	void *buffer;
};

/* EXPORT */
struct git_odb {
	git_refcount rc;
	git_vector backends;
	git_cache own_cache;
};

/*
 * Hash a git_rawobj internally.
 * The `git_rawobj` is supposed to be previously initialized
 */
int git_odb__hashobj(git_oid *id, git_rawobj *obj);

/*
 * Format the object header such as it would appear in the on-disk object
 */
int git_odb__format_object_header(char *hdr, size_t n, size_t obj_len, git_otype obj_type);
/*
 * Hash an open file descriptor.
 * This is a performance call when the contents of a fd need to be hashed,
 * but the fd is already open and we have the size of the contents.
 *
 * Saves us some `stat` calls.
 *
 * The fd is never closed, not even on error. It must be opened and closed
 * by the caller
 */
int git_odb__hashfd(git_oid *out, git_file fd, size_t size, git_otype type);

/*
 * Hash an open file descriptor applying an array of filters
 * Acts just like git_odb__hashfd with the addition of filters...
 */
int git_odb__hashfd_filtered(
	git_oid *out, git_file fd, size_t len, git_otype type, git_filter_list *fl);

/*
 * Hash a `path`, assuming it could be a POSIX symlink: if the path is a
 * symlink, then the raw contents of the symlink will be hashed. Otherwise,
 * this will fallback to `git_odb__hashfd`.
 *
 * The hash type for this call is always `GIT_OBJ_BLOB` because symlinks may
 * only point to blobs.
 */
int git_odb__hashlink(git_oid *out, const char *path);

/*
 * Generate a GIT_ENOTFOUND error for the ODB.
 */
int git_odb__error_notfound(const char *message, const git_oid *oid);

/*
 * Generate a GIT_EAMBIGUOUS error for the ODB.
 */
int git_odb__error_ambiguous(const char *message);

/*
 * Attempt to read object header or just return whole object if it could
 * not be read.
 */
int git_odb__read_header_or_object(
	git_odb_object **out, size_t *len_p, git_otype *type_p,
	git_odb *db, const git_oid *id);

/* fully free the object; internal method, DO NOT EXPORT */
void git_odb_object__free(void *object);

#endif
