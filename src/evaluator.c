#include "eu_eval.h"

#include "eu_error.h"
#include "eu_symbol.h"
#include "eu_pair.h"

#include "eu_rt.h"

eu_result eu_evaluate(europa* s, eu_value* v, eu_value* out) {
	eu_value chunk;

	/* compile the value */
	_eu_checkreturn(eucode_compile(s, v, &chunk));

	/* run it in the vm */
	_eu_checkreturn(euvm_doclosure(s, _euvalue_to_closure(&chunk), &_null,
		out));

	return EU_RESULT_OK;
}
