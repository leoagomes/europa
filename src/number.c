#include "eu_number.h"

/* == **number** functions == */

/** Calculates a hash for a given number.
 * 
 * @param v The number.
 * @return The hash.
 */
eu_uinteger eunum_hash(eu_value* v) {
	eu_integer ival;

	ival = (_euvalue_rtype(v) & EU_NUMBER_REAL) ? cast(int, v->value.r) :
		v->value.i;

	return ival << 1;
}

/** Determines whether two numeric values are `eqv?`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @param c Where to place the result.
 * @return Whether the operation was successful.
 */
eu_result eunum_eqv(eu_value* a, eu_value* b, eu_value* out) {
	int v;

	/* a and b exact: a == b is #t */
	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		v = _eunum_i(a) == _eunum_i(b);
		goto end;
	}

	/* a exact and b inexact (or vice-versa): #f */
	if (_eunum_is_exact(a) || _eunum_is_exact(b)) {
		v = EU_FALSE;
		goto end;
	}

	/* a and b inexact: a == b is #t */
	v = _eunum_r(a) == _eunum_r(b);

	end:
	_eu_makebool(out, v);
	return EU_RESULT_OK;
}

/* == **boolean** functions == */

/** Calculates a hash for a boolean.
 * 
 * @param v The boolean value.
 * @return The hash.
 */
eu_uinteger eubool_hash(eu_value* v) {
	return v->value.boolean ? 0xAA : ~0xAA;
}

/** Determines whether two boolean values are `eqv?`.
 * 
 * @param a The first value.
 * @param b The second value.
 * @param c Where to place the result.
 * @return Whether the operation was successful.
 */
eu_result eubool_eqv(eu_value* a, eu_value* b, eu_value* out) {
	int av, bv;
	av = _euvalue_to_bool(a);
	bv = _euvalue_to_bool(b);
	_eu_makebool(out, av == bv);
	return EU_RESULT_OK;
}