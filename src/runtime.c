#include <setjmp.h>

#include "europa/runtime.h"
#include "europa/internal.h"

struct europa_jump_list {
	struct europa_jump_list* previous;

	jmp_buf buffer;
};

int europa_runtime_run_protected(
	struct europa* europa,
	europa_protected_fn fn,
	void* userdata
) {
	struct europa_jump_list node;

	node.previous = europa->jump_list;
	europa->jump_list = &node;

	int status;
	if ((status = setjmp(node.buffer))) {
		status = fn(europa, userdata);
	}

	europa->jump_list = node.previous;

	return status;
}
