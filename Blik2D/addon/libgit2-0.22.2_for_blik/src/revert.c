/*
* Copyright (C) the libgit2 contributors. All rights reserved.
*
* This file is part of libgit2, distributed under the GNU GPL v2 with
* a Linking Exception. For full terms see the included COPYING file.
*/

#include BLIK_LIBGIT2_U_common_h //original-code:"common.h"
#include BLIK_LIBGIT2_U_repository_h //original-code:"repository.h"
#include "filebuf.h"
#include "merge.h"

#include BLIK_LIBGIT2_U_git2__types_h //original-code:"git2/types.h"
#include BLIK_LIBGIT2_U_git2__merge_h //original-code:"git2/merge.h"
#include BLIK_LIBGIT2_U_git2__revert_h //original-code:"git2/revert.h"
#include BLIK_LIBGIT2_U_git2__commit_h //original-code:"git2/commit.h"
#include BLIK_LIBGIT2_U_git2__sys__commit_h //original-code:"git2/sys/commit.h"

#define GIT_REVERT_FILE_MODE		0666

static int write_revert_head(
	git_repository *repo,
	const char *commit_oidstr)
{
	git_filebuf file = GIT_FILEBUF_INIT;
	git_buf file_path = GIT_BUF_INIT;
	int error = 0;

	if ((error = git_buf_joinpath(&file_path, repo->path_repository, GIT_REVERT_HEAD_FILE)) >= 0 &&
		(error = git_filebuf_open(&file, file_path.ptr, GIT_FILEBUF_FORCE, GIT_REVERT_FILE_MODE)) >= 0 &&
		(error = git_filebuf_printf(&file, "%s\n", commit_oidstr)) >= 0)
		error = git_filebuf_commit(&file);

	if (error < 0)
		git_filebuf_cleanup(&file);

	git_buf_free(&file_path);

	return error;
}

static int write_merge_msg_revert( //oz
	git_repository *repo,
	const char *commit_oidstr,
	const char *commit_msgline)
{
	git_filebuf file = GIT_FILEBUF_INIT;
	git_buf file_path = GIT_BUF_INIT;
	int error = 0;

	if ((error = git_buf_joinpath(&file_path, repo->path_repository, GIT_MERGE_MSG_FILE)) < 0 ||
		(error = git_filebuf_open(&file, file_path.ptr, GIT_FILEBUF_FORCE, GIT_REVERT_FILE_MODE)) < 0 ||
		(error = git_filebuf_printf(&file, "Revert \"%s\"\n\nThis reverts commit %s.\n",
		commit_msgline, commit_oidstr)) < 0)
		goto cleanup;

	error = git_filebuf_commit(&file);

cleanup:
	if (error < 0)
		git_filebuf_cleanup(&file);

	git_buf_free(&file_path);

	return error;
}

static int revert_normalize_opts(
	git_repository *repo,
	git_revert_options *opts,
	const git_revert_options *given,
	const char *their_label)
{
	int error = 0;
	unsigned int default_checkout_strategy = GIT_CHECKOUT_SAFE_CREATE |
		GIT_CHECKOUT_ALLOW_CONFLICTS;

	GIT_UNUSED(repo);

	if (given != NULL)
		memcpy(opts, given, sizeof(git_revert_options));
	else {
		git_revert_options default_opts = GIT_REVERT_OPTIONS_INIT;
		memcpy(opts, &default_opts, sizeof(git_revert_options));
	}

	if (!opts->checkout_opts.checkout_strategy)
		opts->checkout_opts.checkout_strategy = default_checkout_strategy;

	if (!opts->checkout_opts.our_label)
		opts->checkout_opts.our_label = "HEAD";

	if (!opts->checkout_opts.their_label)
		opts->checkout_opts.their_label = their_label;

	return error;
}

static int revert_state_cleanup(git_repository *repo)
{
	const char *state_files[] = { GIT_REVERT_HEAD_FILE, GIT_MERGE_MSG_FILE };

	return git_repository__cleanup_files(repo, state_files, ARRAY_SIZE(state_files));
}

static int revert_seterr(git_commit *commit, const char *fmt)
{
	char commit_oidstr[GIT_OID_HEXSZ + 1];

	git_oid_fmt(commit_oidstr, git_commit_id(commit));
	commit_oidstr[GIT_OID_HEXSZ] = '\0';

	giterr_set(GITERR_REVERT, fmt, commit_oidstr);

	return -1;
}

int git_revert_commit(
	git_index **out,
	git_repository *repo,
	git_commit *revert_commit,
	git_commit *our_commit,
	unsigned int mainline,
	const git_merge_options *merge_opts)
{
	git_commit *parent_commit = NULL;
	git_tree *parent_tree = NULL, *our_tree = NULL, *revert_tree = NULL;
	int parent = 0, error = 0;

	assert(out && repo && revert_commit && our_commit);

	if (git_commit_parentcount(revert_commit) > 1) {
		if (!mainline)
			return revert_seterr(revert_commit,
				"Mainline branch is not specified but %s is a merge commit");

		parent = mainline;
	} else {
		if (mainline)
			return revert_seterr(revert_commit,
				"Mainline branch specified but %s is not a merge commit");

		parent = git_commit_parentcount(revert_commit);
	}

	if (parent &&
		((error = git_commit_parent(&parent_commit, revert_commit, (parent - 1))) < 0 ||
		(error = git_commit_tree(&parent_tree, parent_commit)) < 0))
		goto done;

	if ((error = git_commit_tree(&revert_tree, revert_commit)) < 0 ||
		(error = git_commit_tree(&our_tree, our_commit)) < 0)
		goto done;

	error = git_merge_trees(out, repo, revert_tree, our_tree, parent_tree, merge_opts);

done:
	git_tree_free(parent_tree);
	git_tree_free(our_tree);
	git_tree_free(revert_tree);
	git_commit_free(parent_commit);

	return error;
}

int git_revert(
	git_repository *repo,
	git_commit *commit,
	const git_revert_options *given_opts)
{
	git_revert_options opts;
	git_reference *our_ref = NULL;
	git_commit *our_commit = NULL;
	char commit_oidstr[GIT_OID_HEXSZ + 1];
	const char *commit_msg;
	git_buf their_label = GIT_BUF_INIT;
	git_index *index_new = NULL;
	int error;

	assert(repo && commit);

	GITERR_CHECK_VERSION(given_opts, GIT_REVERT_OPTIONS_VERSION, "git_revert_options");

	if ((error = git_repository__ensure_not_bare(repo, "revert")) < 0)
		return error;

	git_oid_fmt(commit_oidstr, git_commit_id(commit));
	commit_oidstr[GIT_OID_HEXSZ] = '\0';

	if ((commit_msg = git_commit_summary(commit)) == NULL) {
		error = -1;
		goto on_error;
	}

	if ((error = git_buf_printf(&their_label, "parent of %.7s... %s", commit_oidstr, commit_msg)) < 0 ||
		(error = revert_normalize_opts(repo, &opts, given_opts, git_buf_cstr(&their_label))) < 0 ||
		(error = write_revert_head(repo, commit_oidstr)) < 0 ||
		(error = write_merge_msg_revert(repo, commit_oidstr, commit_msg)) < 0 || //oz
		(error = git_repository_head(&our_ref, repo)) < 0 ||
		(error = git_reference_peel((git_object **)&our_commit, our_ref, GIT_OBJ_COMMIT)) < 0 ||
		(error = git_revert_commit(&index_new, repo, commit, our_commit, opts.mainline, &opts.merge_opts)) < 0 ||
		(error = git_merge__check_result(repo, index_new)) < 0 ||
		(error = git_merge__append_conflicts_to_merge_msg(repo, index_new)) < 0 ||
		(error = git_checkout_index(repo, index_new, &opts.checkout_opts)) < 0)
		goto on_error;

	goto done;

on_error:
	revert_state_cleanup(repo);

done:
	git_index_free(index_new);
	git_commit_free(our_commit);
	git_reference_free(our_ref);
	git_buf_free(&their_label);

	return error;
}

int git_revert_init_options(git_revert_options *opts, unsigned int version)
{
	GIT_INIT_STRUCTURE_FROM_TEMPLATE(
		opts, version, git_revert_options, GIT_REVERT_OPTIONS_INIT);
	return 0;
}
