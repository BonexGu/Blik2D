/*
 * Copyright (C) the libgit2 contributors. All rights reserved.
 *
 * This file is part of libgit2, distributed under the GNU GPL v2 with
 * a Linking Exception. For full terms see the included COPYING file.
 */

#include BLIK_LIBGIT2_U_buffer_h //original-code:"buffer.h"
#include BLIK_LIBGIT2_U_common_h //original-code:"common.h"
#include BLIK_LIBGIT2_U_global_h //original-code:"global.h"
#include "trace.h"
#include BLIK_LIBGIT2_U_git2__trace_h //original-code:"git2/trace.h"

#ifdef GIT_TRACE

struct git_trace_data git_trace__data = {0};

#endif

int git_trace_set(git_trace_level_t level, git_trace_callback callback)
{
#ifdef GIT_TRACE
	assert(level == 0 || callback != NULL);

	git_trace__data.level = level;
	git_trace__data.callback = callback;
	GIT_MEMORY_BARRIER;

	return 0;
#else
	GIT_UNUSED(level);
	GIT_UNUSED(callback);

	giterr_set(GITERR_INVALID,
		"This version of libgit2 was not built with tracing.");
	return -1;
#endif
}
