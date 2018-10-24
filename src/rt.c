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
		_eu_checkreturn((f)(s, ud));
	}

	/* restore old jump list */
	s->error_jmp = jmp.previous;

	return jmp.res;
}

eu_result eurt_evaluate(europa* s, eu_value* v, eu_value* out) {
	eu_value chunk;

	/* compile the value */
	_eu_checkreturn(eucode_compile(s, v, &chunk));

	/* run it in the vm */
	_eu_checkreturn(euvm_doclosure(s, _euvalue_to_closure(&chunk), &_null,
		out));

	return EU_RESULT_OK;
}
