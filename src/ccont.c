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
