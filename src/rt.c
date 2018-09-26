/* beware the runtime code */
#include "eu_rt.h"

#include <setjmp.h>

struct europa_jmplist {
	struct europa_jmplist* previous;
	jmp_buf buf;
	eu_result res;
};

eu_result eurt_runcprotected(europa* s, eu_pfunc f, void* ud) {
	struct europa_jmplist jmp;

	/* jump buffer list setup */
	jmp.previous = s->error_jmp;
	jmp.res = EU_RESULT_OK;
	s->error_jmp = &jmp;

	/* protected call */
	if (setjmp(jmp.buf) == 0) {
		(f)(s, ud);
	}

	/* restore old jump list */
	s->error_jmp = jmp.previous;

	return jmp.res;
}
