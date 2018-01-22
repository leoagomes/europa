#include "eu_object.h"

#include "eu_error.h"
#include "eu_cell.h"

eu_value euvalue_from_boolean(int b) {
	eu_value v;

	v.type = EU_TYPE_BOOLEAN;
	v.value.boolean = (b != 0) ? EU_BOOL_TRUE : EU_BOOL_FALSE;

	return v;
}

eu_value euapi_value_is_null(europa* s, eu_cell* args) {
	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "null?", 0));

	return euval_from_boolean(euvalue_is_null(ccar(args)));
}