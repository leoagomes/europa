#include "eu_eval.h"

#include "eu_error.h"
#include "eu_symbol.h"
#include "eu_pair.h"

eu_result eu_evaluate(europa* s, eu_value* v, eu_value* out) {
	/* setup initial environment */
	s->acc = _null;
	s->next = _null;
	s->env = s->global->env;

	/* evaluate the expression */
	_eu_checkreturn(eu_eval(s, v));

	/* set the output to whatever's in the accumulator */
	*out = s->acc;

	return EU_RESULT_OK;
}




eu_result eval_application(europa* s, eu_value* v) {
	eu_pair *c;
	eu_value *car, *cdr;

	if (!_euvalue_is_pair(v))
		return EU_RESULT_BAD_ARGUMENT;

	c = _euvalue_to_pair(v);



	return EU_RESULT_OK;
}

eu_result eu_eval(europa* s, eu_value* val) {
	eu_value v, *tv;

	/* copy value into v */
	v = *val;

	do {
		switch (_euvalue_type(&v)) {
		case EU_TYPE_SYMBOL: /* symbol, find it in the environment */
			_eu_checkreturn(eutable_get(s, _eu_env(s), &v, &tv));
			if (tv == NULL) { /* symbol not found in environment */
				europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"%s not found in environment.",
					_eusymbol_text(_euvalue_to_symbol(&v)));
				return EU_RESULT_ERROR;
			}
			/* symbol found, set the accumulator to its value */
			s->acc = *tv;
			break;
		case EU_TYPE_PAIR: /* function application */

			break;
		default: /* constant */
			s->acc = v; /* put it in the accumulator */
			break;
		}

		v = s->next;
	} while (!_euvalue_is_null(&(s->next)));


	return EU_RESULT_OK;
}
