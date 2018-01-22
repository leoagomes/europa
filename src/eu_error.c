#include "eu_error.h"
#include <stdio.h>

eu_error* euerr_new(europa* s, const char* fmt, ...) {
	eu_gcobj* obj;
	eu_error* err;
	va_list args;
	va_start(args, fmt);

	obj = eugc_new_object(eu_get_gc(s), EU_TYPE_ERROR, euerr_size());
	if (!obj)
		return NULL;

	err = eu_gcobj2error(obj);
	snprintf(err->message, EU_ERROR_MSG_LEN, fmt, args);
	va_end(args);

	return err;
}

void euerr_destroy(europa_gc* gc, eu_error* e) {
}


eu_value euerr_tovalue(eu_error* e) {
	eu_value v;
	v.type = EU_TYPE_OBJECT;
	v.value.object = eu_error2gcobj(e);
	return v;
}

eu_error* euerr_bad_value_type(europa* s, eu_value given, eu_byte expected) {
	eu_error* err;
	
	err = euerr_new(s, "bad passed value type");
	err->error_type = EU_ERROR_BAD_TYPE_VT;

	err->et = expected;
	err->gv = given;

	return err;
}

eu_error* euerr_bad_argument_count(europa* s, const char* to, int count) {
	eu_error*  err;

	err = euerr_new(s, "bad argument count (%d) to %s", count, to);
	err->error_type = EU_ERROR_BAD_ARGUMENT_COUNT;
	err->count = count;

	return err;
}

eu_error* euerr_arity_mismatch(europa* s, const char* to, int ex, int given) {
	eu_error* err;

	err = euerr_new(s, "arity mismatch at '%s'; expected %d, "
		"gotten %d arguments", to, ex, given);

	err->error_type = EU_ERROR_ARITY_MISMATCH;
	err->ev = euvalue_from_intM(ex);
	err->gv = euvalue_from_intM(given);

	return err;
}