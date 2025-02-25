/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */
#include BLIK_LIBGIT2_U_common_h //original-code:"common.h"
#include BLIK_LIBGIT2_U_git2__blob_h //original-code:"git2/blob.h"
#include BLIK_LIBGIT2_U_git2__submodule_h //original-code:"git2/submodule.h"
#include "diff.h"
#include "diff_file.h"
#include BLIK_LIBGIT2_U_odb_h //original-code:"odb.h"
#include "fileops.h"
#include "filter.h"

#define DIFF_MAX_FILESIZE 0x20000000

static bool diff_file_content_binary_by_size(git_diff_file_content *fc)
{
	/* if we have diff opts, check max_size vs file size */
	if ((fc->file->flags & DIFF_FLAGS_KNOWN_BINARY) == 0 &&
		fc->opts_max_size > 0 &&
		fc->file->size > fc->opts_max_size)
		fc->file->flags |= GIT_DIFF_FLAG_BINARY;

	return ((fc->file->flags & GIT_DIFF_FLAG_BINARY) != 0);
}

static void diff_file_content_binary_by_content(git_diff_file_content *fc)
{
	if ((fc->file->flags & DIFF_FLAGS_KNOWN_BINARY) != 0)
		return;

	switch (git_diff_driver_content_is_binary(
		fc->driver, fc->map.data, fc->map.len)) {
	case 0: fc->file->flags |= GIT_DIFF_FLAG_NOT_BINARY; break;
	case 1: fc->file->flags |= GIT_DIFF_FLAG_BINARY; break;
	default: break;
	}
}

static int diff_file_content_init_common(
	git_diff_file_content *fc, const git_diff_options *opts)
{
	fc->opts_flags = opts ? opts->flags : GIT_DIFF_NORMAL;

	if (opts && opts->max_size >= 0)
		fc->opts_max_size = opts->max_size ?
			opts->max_size : DIFF_MAX_FILESIZE;

	if (fc->src == GIT_ITERATOR_TYPE_EMPTY)
		fc->src = GIT_ITERATOR_TYPE_TREE;

	if (!fc->driver &&
		git_diff_driver_lookup(&fc->driver, fc->repo, fc->file->path) < 0)
		return -1;

	/* give driver a chance to modify options */
	git_diff_driver_update_options(&fc->opts_flags, fc->driver);

	/* make sure file is conceivable mmap-able */
	if ((git_off_t)((size_t)fc->file->size) != fc->file->size)
		fc->file->flags |= GIT_DIFF_FLAG_BINARY;
	/* check if user is forcing text diff the file */
	else if (fc->opts_flags & GIT_DIFF_FORCE_TEXT) {
		fc->file->flags &= ~GIT_DIFF_FLAG_BINARY;
		fc->file->flags |= GIT_DIFF_FLAG_NOT_BINARY;
	}
	/* check if user is forcing binary diff the file */
	else if (fc->opts_flags & GIT_DIFF_FORCE_BINARY) {
		fc->file->flags &= ~GIT_DIFF_FLAG_NOT_BINARY;
		fc->file->flags |= GIT_DIFF_FLAG_BINARY;
	}

	diff_file_content_binary_by_size(fc);

	if ((fc->flags & GIT_DIFF_FLAG__NO_DATA) != 0) {
		fc->flags |= GIT_DIFF_FLAG__LOADED;
		fc->map.len  = 0;
		fc->map.data = "";
	}

	if ((fc->flags & GIT_DIFF_FLAG__LOADED) != 0)
		diff_file_content_binary_by_content(fc);

	return 0;
}

int git_diff_file_content__init_from_diff(
	git_diff_file_content *fc,
	git_diff *diff,
	size_t delta_index,
	bool use_old)
{
	git_diff_delta *delta = git_vector_get(&diff->deltas, delta_index);
	bool has_data = true;

	memset(fc, 0, sizeof(*fc));
	fc->repo = diff->repo;
	fc->file = use_old ? &delta->old_file : &delta->new_file;
	fc->src  = use_old ? diff->old_src : diff->new_src;

	if (git_diff_driver_lookup(&fc->driver, fc->repo, fc->file->path) < 0)
		return -1;

	switch (delta->status) {
	case GIT_DELTA_ADDED:
		has_data = !use_old; break;
	case GIT_DELTA_DELETED:
		has_data = use_old; break;
	case GIT_DELTA_UNTRACKED:
		has_data = !use_old &&
			(diff->opts.flags & GIT_DIFF_SHOW_UNTRACKED_CONTENT) != 0;
		break;
	case GIT_DELTA_UNREADABLE:
	case GIT_DELTA_MODIFIED:
	case GIT_DELTA_COPIED:
	case GIT_DELTA_RENAMED:
		break;
	default:
		has_data = false;
		break;
	}

	if (!has_data)
		fc->flags |= GIT_DIFF_FLAG__NO_DATA;

	return diff_file_content_init_common(fc, &diff->opts);
}

int git_diff_file_content__init_from_src(
	git_diff_file_content *fc,
	git_repository *repo,
	const git_diff_options *opts,
	const git_diff_file_content_src *src,
	git_diff_file *as_file)
{
	memset(fc, 0, sizeof(*fc));
	fc->repo = repo;
	fc->file = as_file;
	fc->blob = src->blob;

	if (!src->blob && !src->buf) {
		fc->flags |= GIT_DIFF_FLAG__NO_DATA;
	} else {
		fc->flags |= GIT_DIFF_FLAG__LOADED;
		fc->file->flags |= GIT_DIFF_FLAG_VALID_ID;
		fc->file->mode = GIT_FILEMODE_BLOB;

		if (src->blob) {
			fc->file->size = git_blob_rawsize(src->blob);
			git_oid_cpy(&fc->file->id, git_blob_id(src->blob));

			fc->map.len  = (size_t)fc->file->size;
			fc->map.data = (char *)git_blob_rawcontent(src->blob);
		} else {
			fc->file->size = src->buflen;
			git_odb_hash(&fc->file->id, src->buf, src->buflen, GIT_OBJ_BLOB);

			fc->map.len  = src->buflen;
			fc->map.data = (char *)src->buf;
		}
	}

	return diff_file_content_init_common(fc, opts);
}

static int diff_file_content_commit_to_str(
	git_diff_file_content *fc, bool check_status)
{
	char oid[GIT_OID_HEXSZ+1];
	git_buf content = GIT_BUF_INIT;
	const char *status = "";

	if (check_status) {
		int error = 0;
		git_submodule *sm = NULL;
		unsigned int sm_status = 0;
		const git_oid *sm_head;

		if ((error = git_submodule_lookup(&sm, fc->repo, fc->file->path)) < 0) {
			/* GIT_EEXISTS means a "submodule" that has not been git added */
			if (error == GIT_EEXISTS) {
				giterr_clear();
				error = 0;
			}
			return error;
		}

		if ((error = git_submodule_status(&sm_status, sm)) < 0) {
			git_submodule_free(sm);
			return error;
		}

		/* update OID if we didn't have it previously */
		if ((fc->file->flags & GIT_DIFF_FLAG_VALID_ID) == 0 &&
			((sm_head = git_submodule_wd_id(sm)) != NULL ||
			 (sm_head = git_submodule_head_id(sm)) != NULL))
		{
			git_oid_cpy(&fc->file->id, sm_head);
			fc->file->flags |= GIT_DIFF_FLAG_VALID_ID;
		}

		if (GIT_SUBMODULE_STATUS_IS_WD_DIRTY(sm_status))
			status = "-dirty";

		git_submodule_free(sm);
	}

	git_oid_tostr(oid, sizeof(oid), &fc->file->id);
	if (git_buf_printf(&content, "Subproject commit %s%s\n", oid, status) < 0)
		return -1;

	fc->map.len  = git_buf_len(&content);
	fc->map.data = git_buf_detach(&content);
	fc->flags |= GIT_DIFF_FLAG__FREE_DATA;

	return 0;
}

static int diff_file_content_load_blob(git_diff_file_content *fc)
{
	int error = 0;
	git_odb_object *odb_obj = NULL;

	if (git_oid_iszero(&fc->file->id))
		return 0;

	if (fc->file->mode == GIT_FILEMODE_COMMIT)
		return diff_file_content_commit_to_str(fc, false);

	/* if we don't know size, try to peek at object header first */
	if (!fc->file->size) {
		if ((error = git_diff_file__resolve_zero_size(
				fc->file, &odb_obj, fc->repo)) < 0)
			return error;
	}

	if (diff_file_content_binary_by_size(fc))
		return 0;

	if (odb_obj != NULL) {
		error = git_object__from_odb_object(
			(git_object **)&fc->blob, fc->repo, odb_obj, GIT_OBJ_BLOB);
		git_odb_object_free(odb_obj);
	} else {
		error = git_blob_lookup(
			(git_blob **)&fc->blob, fc->repo, &fc->file->id);
	}

	if (!error) {
		fc->flags |= GIT_DIFF_FLAG__FREE_BLOB;
		fc->map.data = (void *)git_blob_rawcontent(fc->blob);
		fc->map.len  = (size_t)git_blob_rawsize(fc->blob);
	}

	return error;
}

static int diff_file_content_load_workdir_symlink(
	git_diff_file_content *fc, git_buf *path)
{
	ssize_t alloc_len, read_len;

	/* link path on disk could be UTF-16, so prepare a buffer that is
	 * big enough to handle some UTF-8 data expansion
	 */
	alloc_len = (ssize_t)(fc->file->size * 2) + 1;

	fc->map.data = git__calloc(alloc_len, sizeof(char));
	GITERR_CHECK_ALLOC(fc->map.data);

	fc->flags |= GIT_DIFF_FLAG__FREE_DATA;

	read_len = p_readlink(git_buf_cstr(path), fc->map.data, alloc_len);
	if (read_len < 0) {
		giterr_set(GITERR_OS, "Failed to read symlink '%s'", fc->file->path);
		return -1;
	}

	fc->map.len = read_len;
	return 0;
}

static int diff_file_content_load_workdir_file(
	git_diff_file_content *fc, git_buf *path)
{
	int error = 0;
	git_filter_list *fl = NULL;
	git_file fd = git_futils_open_ro(git_buf_cstr(path));
	git_buf raw = GIT_BUF_INIT;

	if (fd < 0)
		return fd;

	if (!fc->file->size &&
		!(fc->file->size = git_futils_filesize(fd)))
		goto cleanup;

	if (diff_file_content_binary_by_size(fc))
		goto cleanup;

	if ((error = git_filter_list_load(
			&fl, fc->repo, NULL, fc->file->path,
			GIT_FILTER_TO_ODB, GIT_FILTER_OPT_ALLOW_UNSAFE)) < 0)
		goto cleanup;

	/* if there are no filters, try to mmap the file */
	if (fl == NULL) {
		if (!(error = git_futils_mmap_ro(
				&fc->map, fd, 0, (size_t)fc->file->size))) {
			fc->flags |= GIT_DIFF_FLAG__UNMAP_DATA;
			goto cleanup;
		}

		/* if mmap failed, fall through to try readbuffer below */
		giterr_clear();
	}

	if (!(error = git_futils_readbuffer_fd(&raw, fd, (size_t)fc->file->size))) {
		git_buf out = GIT_BUF_INIT;

		error = git_filter_list_apply_to_data(&out, fl, &raw);

		if (out.ptr != raw.ptr)
			git_buf_free(&raw);

		if (!error) {
			fc->map.len  = out.size;
			fc->map.data = out.ptr;
			fc->flags |= GIT_DIFF_FLAG__FREE_DATA;
		}
	}

cleanup:
	git_filter_list_free(fl);
	p_close(fd);

	return error;
}

static int diff_file_content_load_workdir(git_diff_file_content *fc)
{
	int error = 0;
	git_buf path = GIT_BUF_INIT;

	if (fc->file->mode == GIT_FILEMODE_COMMIT)
		return diff_file_content_commit_to_str(fc, true);

	if (fc->file->mode == GIT_FILEMODE_TREE)
		return 0;

	if (git_buf_joinpath(
			&path, git_repository_workdir(fc->repo), fc->file->path) < 0)
		return -1;

	if (S_ISLNK(fc->file->mode))
		error = diff_file_content_load_workdir_symlink(fc, &path);
	else
		error = diff_file_content_load_workdir_file(fc, &path);

	/* once data is loaded, update OID if we didn't have it previously */
	if (!error && (fc->file->flags & GIT_DIFF_FLAG_VALID_ID) == 0) {
		error = git_odb_hash(
			&fc->file->id, fc->map.data, fc->map.len, GIT_OBJ_BLOB);
		fc->file->flags |= GIT_DIFF_FLAG_VALID_ID;
	}

	git_buf_free(&path);
	return error;
}

int git_diff_file_content__load(git_diff_file_content *fc)
{
	int error = 0;

	if ((fc->flags & GIT_DIFF_FLAG__LOADED) != 0)
		return 0;

	if ((fc->file->flags & GIT_DIFF_FLAG_BINARY) != 0)
		return 0;

	if (fc->src == GIT_ITERATOR_TYPE_WORKDIR)
		error = diff_file_content_load_workdir(fc);
	else
		error = diff_file_content_load_blob(fc);
	if (error)
		return error;

	fc->flags |= GIT_DIFF_FLAG__LOADED;

	diff_file_content_binary_by_content(fc);

	return 0;
}

void git_diff_file_content__unload(git_diff_file_content *fc)
{
	if ((fc->flags & GIT_DIFF_FLAG__LOADED) == 0)
		return;

	if (fc->flags & GIT_DIFF_FLAG__FREE_DATA) {
		git__free(fc->map.data);
		fc->map.data = "";
		fc->map.len  = 0;
		fc->flags &= ~GIT_DIFF_FLAG__FREE_DATA;
	}
	else if (fc->flags & GIT_DIFF_FLAG__UNMAP_DATA) {
		git_futils_mmap_free(&fc->map);
		fc->map.data = "";
		fc->map.len  = 0;
		fc->flags &= ~GIT_DIFF_FLAG__UNMAP_DATA;
	}

	if (fc->flags & GIT_DIFF_FLAG__FREE_BLOB) {
		git_blob_free((git_blob *)fc->blob);
		fc->blob = NULL;
		fc->flags &= ~GIT_DIFF_FLAG__FREE_BLOB;
	}

	fc->flags &= ~GIT_DIFF_FLAG__LOADED;
}

void git_diff_file_content__clear(git_diff_file_content *fc)
{
	git_diff_file_content__unload(fc);

	/* for now, nothing else to do */
}
