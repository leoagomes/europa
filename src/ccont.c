#include "eu_ccont.h"

#include "eu_error.h"

eu_result eucc_frame(europa* s) {
	eu_continuation* cont;

	/* create the continuation */
	cont = eucont_new(s, s->previous, s->env, &(s->rib), s->rib_lastpos, s->ccl,
		s->pc);
	if (cont == NULL) {
		_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
			"Could not create continuation."));
		return EU_RESULT_ERROR;
	}

	/* update state */
	s->previous = cont;
	s->rib = _null;
	s->rib_lastpos = &s->rib;

	return EU_RESULT_OK;
}

eu_result eucc_define_cclosure(europa* s, eu_table* t, eu_table* env, void* text,
	eu_cfunc cf) {
	eu_closure* cl;
	eu_value* tv, closure;

	/* check parameters */
	if (!s || !t || !cf)
		return EU_RESULT_NULL_ARGUMENT;

	/* create a closure for the cfunction */
	cl = eucl_new(s, cf, NULL, env);
	if (cl == NULL) 
		return EU_RESULT_BAD_ALLOC;

	/* set the value up */
	_eu_makeclosure(&closure, cl);
	tv = &closure;

	/* define this closure into the table */
	return eutable_define_symbol(s, t, text, &tv);
}
