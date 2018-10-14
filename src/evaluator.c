#include "eu_eval.h"

eu_result eu_evaluate(europa* s, eu_value* v, eu_value* out) {
	/* setup initial environment */
	s->acc = _null;
	s->next = _null;
	s->env = s->global->env;
}

eu_result eu_eval(europa* s, eu_value* v, eu_value* out) {
	return EU_RESULT_OK;
}