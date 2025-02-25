/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#ifndef INCLUDE_refs_h__
#define INCLUDE_refs_h__

#include BLIK_LIBGIT2_U_common_h //original-code:"common.h"
#include BLIK_LIBGIT2_U_git2__oid_h //original-code:"git2/oid.h"
#include BLIK_LIBGIT2_U_git2__refs_h //original-code:"git2/refs.h"
#include BLIK_LIBGIT2_U_git2__refdb_h //original-code:"git2/refdb.h"
#include "strmap.h"
#include BLIK_LIBGIT2_U_buffer_h //original-code:"buffer.h"
#include "oid.h"

#define GIT_REFS_DIR "refs/"
#define GIT_REFS_HEADS_DIR GIT_REFS_DIR "heads/"
#define GIT_REFS_TAGS_DIR GIT_REFS_DIR "tags/"
#define GIT_REFS_REMOTES_DIR GIT_REFS_DIR "remotes/"
#define GIT_REFS_NOTES_DIR GIT_REFS_DIR "notes/"
#define GIT_REFS_DIR_MODE 0777
#define GIT_REFS_FILE_MODE 0666

#define GIT_RENAMED_REF_FILE GIT_REFS_DIR "RENAMED-REF"

#define GIT_SYMREF "ref: "
#define GIT_PACKEDREFS_FILE "packed-refs"
#define GIT_PACKEDREFS_HEADER "# pack-refs with: peeled fully-peeled "
#define GIT_PACKEDREFS_FILE_MODE 0666

#define GIT_HEAD_FILE "HEAD"
#define GIT_ORIG_HEAD_FILE "ORIG_HEAD"
#define GIT_FETCH_HEAD_FILE "FETCH_HEAD"
#define GIT_MERGE_HEAD_FILE "MERGE_HEAD"
#define GIT_REVERT_HEAD_FILE "REVERT_HEAD"
#define GIT_CHERRYPICK_HEAD_FILE "CHERRY_PICK_HEAD"
#define GIT_BISECT_LOG_FILE "BISECT_LOG"
#define GIT_REBASE_MERGE_DIR "rebase-merge/"
#define GIT_REBASE_MERGE_INTERACTIVE_FILE GIT_REBASE_MERGE_DIR "interactive"
#define GIT_REBASE_APPLY_DIR "rebase-apply/"
#define GIT_REBASE_APPLY_REBASING_FILE GIT_REBASE_APPLY_DIR "rebasing"
#define GIT_REBASE_APPLY_APPLYING_FILE GIT_REBASE_APPLY_DIR "applying"
#define GIT_REFS_HEADS_MASTER_FILE GIT_REFS_HEADS_DIR "master"

#define GIT_STASH_FILE "stash"
#define GIT_REFS_STASH_FILE GIT_REFS_DIR GIT_STASH_FILE

#define GIT_REF_FORMAT__PRECOMPOSE_UNICODE	(1u << 16)

#define GIT_REFNAME_MAX 1024

typedef char git_refname_t[GIT_REFNAME_MAX];

struct git_reference {
	git_refdb *db;
	git_ref_t type;

	union {
		git_oid oid;
		char *symbolic;
	} target;

	git_oid peel;
	char name[GIT_FLEX_ARRAY];
};

git_reference *git_reference__set_name(git_reference *ref, const char *name);

int git_reference__normalize_name(git_buf *buf, const char *name, unsigned int flags);
int git_reference__update_terminal(git_repository *repo, const char *ref_name, const git_oid *oid, const git_signature *signature, const char *log_message);
int git_reference__is_valid_name(const char *refname, unsigned int flags);
int git_reference__is_branch(const char *ref_name);
int git_reference__is_remote(const char *ref_name);
int git_reference__is_tag(const char *ref_name);

/**
 * Lookup a reference by name and try to resolve to an OID.
 *
 * You can control how many dereferences this will attempt to resolve the
 * reference with the `max_deref` parameter, or pass -1 to use a sane
 * default.  If you pass 0 for `max_deref`, this will not attempt to resolve
 * the reference.  For any value of `max_deref` other than 0, not
 * successfully resolving the reference will be reported as an error.

 * The generated reference must be freed by the user.
 *
 * @param reference_out Pointer to the looked-up reference
 * @param repo The repository to look up the reference
 * @param name The long name for the reference (e.g. HEAD, ref/heads/master, refs/tags/v0.1.0, ...)
 * @param max_deref Maximum number of dereferences to make of symbolic refs, 0 means simple lookup, < 0 means use default reasonable value
 * @return 0 on success or < 0 on error; not being able to resolve the reference is an error unless 0 was passed for max_deref
 */
int git_reference_lookup_resolved(
	git_reference **reference_out,
	git_repository *repo,
	const char *name,
	int max_deref);

int git_reference__log_signature(git_signature **out, git_repository *repo);

/** Update a reference after a commit. */
int git_reference__update_for_commit(
	git_repository *repo,
	git_reference *ref,
	const char *ref_name,
	const git_oid *id,
	const git_signature *committer,
	const char *operation);

#endif
