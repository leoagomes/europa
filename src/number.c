/** Numeric types operations and routines.
 *
 * @file number.c
 * @author Leonardo G.
 */
#include "europa/number.h"

#include "europa/error.h"
#include "europa/ccont.h"

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
int eunum_eqv(eu_value* a, eu_value* b, eu_value* out) {
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

int eunum_equal(europa* s, eu_value* a, eu_value* b) {
	if (_eunum_is_exact(a) == _eunum_is_exact(b)) {
		return _eunum_i(a) == _eunum_i(b);
	}
	return _eunum_to_real(a) == _eunum_to_real(b);
}

int eunum_greater(europa* s, eu_value* a, eu_value* b) {
	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		return _eunum_i(a) > _eunum_i(b);
	}
	return _eunum_to_real(a) > _eunum_to_real(b);
}

int eunum_lesser(europa* s, eu_value* a, eu_value* b) {
	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		return _eunum_i(a) < _eunum_i(b);
	}
	return _eunum_to_real(a) < _eunum_to_real(b);
}

int eunum_add(europa* s, eu_value* a, eu_value* b, eu_value* out) {
	if (!_euvalue_is_number(a) || !_euvalue_is_number(b))
		return EU_RESULT_BAD_ARGUMENT;

	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		_eu_makeint(out, _eunum_i(a) + _eunum_i(b));
		return EU_RESULT_OK;
	}

	_eu_makereal(out, _eunum_to_real(a) + _eunum_to_real(b));
	return EU_RESULT_OK;
}

int eunum_subtract(europa* s, eu_value* a, eu_value* b, eu_value* out) {
	if (!_euvalue_is_number(a) || !_euvalue_is_number(b))
		return EU_RESULT_BAD_ARGUMENT;

	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		_eu_makeint(out, _eunum_i(a) - _eunum_i(b));
		return EU_RESULT_OK;
	}

	_eu_makereal(out, _eunum_to_real(a) - _eunum_to_real(b));
	return EU_RESULT_OK;
}

int eunum_multiply(europa* s, eu_value* a, eu_value* b, eu_value* out) {
	if (!_euvalue_is_number(a) || !_euvalue_is_number(b))
		return EU_RESULT_BAD_ARGUMENT;

	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		_eu_makeint(out, _eunum_i(a) * _eunum_i(b));
		return EU_RESULT_OK;
	}

	_eu_makereal(out, _eunum_to_real(a) * _eunum_to_real(b));
	return EU_RESULT_OK;
}

int eunum_divide(europa* s, eu_value* a, eu_value* b, eu_value* out) {
	if (!_euvalue_is_number(a) || !_euvalue_is_number(b))
		return EU_RESULT_BAD_ARGUMENT;

	if (_eunum_is_exact(a) && _eunum_is_exact(b)) {
		_eu_makeint(out, _eunum_i(a) / _eunum_i(b));
		return EU_RESULT_OK;
	}

	_eu_makereal(out, _eunum_to_real(a) / _eunum_to_real(b));
	return EU_RESULT_OK;
}

int eunum_invert(europa* s, eu_value* a, eu_value* out) {
	if (!_euvalue_is_number(a))
		return EU_RESULT_BAD_ARGUMENT;

	/* check division my zero */
	if (_eunum_is_zero(a)) {
		_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
			"Tried inverting zero!"));
		return EU_RESULT_ERROR;
	}

	/* set output to inverted */
	_eu_makereal(out, ((eu_real)1) / _eunum_to_real(a));

	/* in case the result was integer, corret number into integer */
	if (_eunum_is_int(out)) {
		_eu_makeint(out, _eunum_to_int(out));
	}

	return EU_RESULT_OK;
}

int eunum_negate(europa* s, eu_value* a, eu_value* out) {
	if (!_euvalue_is_number(a))
		return EU_RESULT_BAD_ARGUMENT;

	if (_eunum_is_exact(a)) {
		_eu_makeint(out, -(_eunum_i(a)));
	} else {
		_eu_makereal(out, -(_eunum_r(a)));
	}

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
int eubool_eqv(eu_value* a, eu_value* b, eu_value* out) {
	int av, bv;
	av = _euvalue_to_bool(a);
	bv = _euvalue_to_bool(b);
	_eu_makebool(out, av == bv);
	return EU_RESULT_OK;
}



/**
 * @addtogroup language_library
 * @{
 */

int euapi_register_number(europa* s) {
	eu_table* env;

	env = s->env;

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "number?", euapi_numberQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "complex?", euapi_complexQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "real?", euapi_complexQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "rational?", euapi_rationalQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "integer?", euapi_integerQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "exact-integer?", euapi_exactQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "exact?", euapi_exactQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "inexact?", euapi_inexactQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "=", euapi_E));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "<", euapi_L));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, ">", euapi_G));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "<=", euapi_LE));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, ">=", euapi_GE));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "zero?", euapi_zeroQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "positive?", euapi_positiveQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "negative?", euapi_negativeQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "odd?", euapi_oddQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "even?", euapi_evenQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "min", euapi_min));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "max", euapi_max));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "+", euapi_P));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "*", euapi_S));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "-", euapi_M));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "/", euapi_D));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "abs", euapi_abs));

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "boolean?", euapi_booleanQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "not", euapi_not));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "boolean=?", euapi_booleanEQ));

	return EU_RESULT_OK;
}

int euapi_numberQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER));

	return EU_RESULT_OK;
}

int euapi_complexQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER));

	return EU_RESULT_OK;
}

int euapi_realQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER));

	return EU_RESULT_OK;
}

int euapi_rationalQ(europa* s) {
	eu_value* value;

	/* TODO: determine whether number is rational? */

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER));

	return EU_RESULT_OK;
}

int euapi_integerQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s),
		_euvalue_is_type(value, EU_TYPE_NUMBER) && _eunum_is_int(value));

	return EU_RESULT_OK;
}

int euapi_exactQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s),
		_euvalue_is_type(value, EU_TYPE_NUMBER) && _eunum_is_exact(value));

	return EU_RESULT_OK;
}

int euapi_inexactQ(europa *s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s),
		_euvalue_is_type(value, EU_TYPE_NUMBER) && !(_eunum_is_exact(value)));

	return EU_RESULT_OK;
}

int euapi_E(europa* s) {
	eu_value *previous, *current;
	eu_value *pv, *cv;

	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		if ((!_euvalue_is_type(pv, EU_TYPE_NUMBER) ||
			!_euvalue_is_type(cv, EU_TYPE_NUMBER)) ||
			(_eunum_is_exact(pv) != _eunum_is_exact(cv)) ||
			(_eunum_i(pv) != _eunum_i(cv))) {
			goto set_false;
		}

		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;

	set_false:
	_eu_makebool(_eucc_return(s), EU_FALSE);
	return EU_RESULT_OK;
}


int euapi_L(europa* s) {
	eu_value *previous, *current;
	eu_value *pv, *cv;

	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		if ((!_euvalue_is_type(pv, EU_TYPE_NUMBER) |
			!_euvalue_is_type(cv, EU_TYPE_NUMBER))) {
			goto set_false;
		}

		if (!eunum_lesser(s, pv, cv))
			goto set_false;

		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;

	set_false:
	_eu_makebool(_eucc_return(s), EU_FALSE);
	return EU_RESULT_OK;
}

int euapi_G(europa* s) {
	eu_value *previous, *current;
	eu_value *pv, *cv;

	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		if ((!_euvalue_is_type(pv, EU_TYPE_NUMBER) |
			!_euvalue_is_type(cv, EU_TYPE_NUMBER))) {
			goto set_false;
		}

		if (!eunum_greater(s, pv, cv))
			goto set_false;

		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;

	set_false:
	_eu_makebool(_eucc_return(s), EU_FALSE);
	return EU_RESULT_OK;
}

int euapi_LE(europa* s) {
	eu_value *previous, *current;
	eu_value *pv, *cv;

	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		if ((!_euvalue_is_type(pv, EU_TYPE_NUMBER) |
			!_euvalue_is_type(cv, EU_TYPE_NUMBER))) {
			goto set_false;
		}

		if (eunum_greater(s, pv, cv))
			goto set_false;

		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;

	set_false:
	_eu_makebool(_eucc_return(s), EU_FALSE);
	return EU_RESULT_OK;
}

int euapi_GE(europa* s) {
	eu_value *previous, *current;
	eu_value *pv, *cv;

	previous = _eucc_arguments(s);
	current = _eupair_tail(_euvalue_to_pair(previous));

	while (!_euvalue_is_null(current)) {
		pv = _eupair_head(_euvalue_to_pair(previous));
		cv = _eupair_head(_euvalue_to_pair(current));

		if ((!_euvalue_is_type(pv, EU_TYPE_NUMBER) |
			!_euvalue_is_type(cv, EU_TYPE_NUMBER))) {
			goto set_false;
		}

		if (eunum_lesser(s, pv, cv))
			goto set_false;

		previous = current;
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;

	set_false:
	_eu_makebool(_eucc_return(s), EU_FALSE);
	return EU_RESULT_OK;
}

int euapi_zeroQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER) &&
		_eunum_i(value) == 0);

	return EU_RESULT_OK;
}

int euapi_positiveQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER) &&
		_eunum_is_exact(value) ? (_eunum_i(value) >= 0) : (_eunum_r(value) >= 0));

	return EU_RESULT_OK;
}

int euapi_negativeQ(europa* s) {
	eu_value* value;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, value, 0);
	_eu_makebool(_eucc_return(s), _euvalue_is_type(value, EU_TYPE_NUMBER) &&
		_eunum_is_exact(value) ? (_eunum_i(value) < 0) : (_eunum_r(value) < 0));

	return EU_RESULT_OK;
}

int euapi_oddQ(europa* s) {
	eu_value* value;
	eu_integer i;
	eu_real r;

	_eucc_arity_proper(s, 1);
	_eucc_argument_type(s, value, 0, EU_TYPE_NUMBER);

	i = _eunum_i(value);
	if (!_eunum_is_exact(value)) {
		r = _eunum_r(value);
		i = (eu_integer)r;
		if ((r - (eu_real)i) != (eu_real)0) {
			_eu_makebool(_eucc_return(s), EU_FALSE);
			return EU_RESULT_OK;
		}
	}

	_eu_makebool(_eucc_return(s), i % 2 ? EU_TRUE : EU_FALSE);
	return EU_RESULT_OK;
}

int euapi_evenQ(europa* s) {
	eu_value* value;
	eu_integer i;
	eu_real r;

	_eucc_arity_proper(s, 1);
	_eucc_argument_type(s, value, 0, EU_TYPE_NUMBER);

	i = _eunum_i(value);
	if (!_eunum_is_exact(value)) {
		r = _eunum_r(value);
		i = (eu_integer)r;
		if ((r - (eu_real)i) != (eu_real)0) {
			_eu_makebool(_eucc_return(s), EU_FALSE);
			return EU_RESULT_OK;
		}
	}

	_eu_makebool(_eucc_return(s), i % 2 ? EU_FALSE : EU_TRUE);
	return EU_RESULT_OK;
}

int euapi_min(europa* s) {
	eu_value *current, *min;

	/* check arity */
	_eucc_arity_improper(s, 1);
	current = _eucc_arguments(s);

	/* intialize minimum and current */
	min = current;
	current = _eupair_tail(_euvalue_to_pair(current));

	/* loop searching for minimum */
	while (!_euvalue_is_null(current)) {
		if (eunum_lesser(s, _eupair_head(_euvalue_to_pair(current)), min)) {
			min = _eupair_head(_euvalue_to_pair(current));
		}

		current = _eupair_tail(_euvalue_to_pair(current));
	}

	/* set result to minimum */
	*_eucc_return(s) = *min;

	return EU_RESULT_OK;
}

int euapi_max(europa* s) {
	eu_value *current, *max;

	/* check arity */
	_eucc_arity_improper(s, 1);
	current = _eucc_arguments(s);

	/* intialize maximum and current */
	max = current;
	current = _eupair_tail(_euvalue_to_pair(current));

	/* loop searching for maximum */
	while (!_euvalue_is_null(current)) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* check whether it is greater than maximum */
		if (eunum_greater(s, _eupair_head(_euvalue_to_pair(current)), max)) {
			max = _eupair_head(_euvalue_to_pair(current));
		}

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	/* set result to maximum */
	*_eucc_return(s) = *max;

	return EU_RESULT_OK;
}

int euapi_P(europa* s) {
	eu_value *current;

	/* setup inital result as zero */
	_eu_makeint(_eucc_return(s), 0);

	/* add up the arguments */
	current = _eucc_arguments(s);
	while (!_euvalue_is_null(current)) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* add it with the current value */
		eunum_add(s, _eucc_return(s), _eupair_head(_euvalue_to_pair(current)),
			_eucc_return(s));

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	return EU_RESULT_OK;
}

int euapi_S(europa* s) {
	eu_value *current;

	/* setup inital result as one */
	_eu_makeint(_eucc_return(s), 1);

	/* multiply up the arguments */
	current = _eucc_arguments(s);
	while (!_euvalue_is_null(current)) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* multiply with result */
		eunum_multiply(s, _eucc_return(s), _eupair_head(_euvalue_to_pair(current)),
			_eucc_return(s));

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	return EU_RESULT_OK;
}

int euapi_M(europa* s) {
	eu_value *current;

	/* check arity for at least one value */
	_eucc_arity_improper(s, 1);

	/* first argument */
	current = _eucc_arguments(s);

	/* check for single argument */
	if (_euvalue_is_null(_eupair_tail(_euvalue_to_pair(current)))) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* set result to negation of value */
		eunum_negate(s, _eupair_head(_euvalue_to_pair(current)), _eucc_return(s));
		return EU_RESULT_OK;
	}

	/* setup initial result as first argument */
	*_eucc_return(s) = *_eupair_head(_euvalue_to_pair(current));

	/* subtract the arguments */
	current = _eupair_tail(_euvalue_to_pair(current));
	while (!_euvalue_is_null(current)) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* subtract arguments */
		eunum_subtract(s, _eucc_return(s), _eupair_head(_euvalue_to_pair(current)),
			_eucc_return(s));

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	return EU_RESULT_OK;
}

int euapi_D(europa* s) {
	eu_value *current;

	/* check arity for at least one value */
	_eucc_arity_improper(s, 1);

	/* first argument */
	current = _eucc_arguments(s);

	/* check for single argument */
	if (_euvalue_is_null(_eupair_tail(_euvalue_to_pair(current)))) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* set result to inverting argument */
		_eu_checkreturn(eunum_invert(s, _eupair_head(_euvalue_to_pair(current)),
			_eucc_return(s)));
		return EU_RESULT_OK;
	}

	/* setup initial result as first argument */
	*_eucc_return(s) = *_eupair_head(_euvalue_to_pair(current));

	/* divide the arguments */
	current = _eupair_tail(_euvalue_to_pair(current));
	while (!_euvalue_is_null(current)) {
		/* check argument type */
		_eucc_check_type(s, _eupair_head(_euvalue_to_pair(current)), "argument",
			EU_TYPE_NUMBER);

		/* divide arguments */
		eunum_divide(s, _eucc_return(s), _eupair_head(_euvalue_to_pair(current)),
			_eucc_return(s));

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	return EU_RESULT_OK;
}

int euapi_abs(europa* s) {
	eu_value* val;

	_eucc_arity_proper(s, 1); /* check arity */
	_eucc_argument_type(s, val, 0, EU_TYPE_NUMBER); /* get argument */

	if ((_eunum_is_exact(val) && _eunum_i(val) < 0) ||
		(!_eunum_is_exact(val) && _eunum_r(val) < (eu_real)0)) {
		eunum_negate(s, val, _eucc_return(s));
		return EU_RESULT_OK;
	}

	*_eucc_return(s) = *val;
	return EU_RESULT_OK;
}

int euapi_not(europa* s) {
	eu_value* obj;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, obj, 0);

	if (_euvalue_is_type(obj, EU_TYPE_BOOLEAN) && _eubool_is_false(obj)) {
		_eu_makebool(_eucc_return(s), EU_FALSE);
		return EU_RESULT_OK;
	}

	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;
}

int euapi_booleanQ(europa* s) {
	eu_value* obj;

	_eucc_arity_improper(s, 1);
	_eucc_argument(s, obj, 0);

	_eu_makebool(_eucc_return(s), _euvalue_is_type(obj, EU_TYPE_BOOLEAN) ?
		EU_TRUE : EU_FALSE);
	return EU_RESULT_OK;
}

#define zerooneify(b) ((b) ? 1 : 0)

int euapi_booleanEQ(europa* s) {
	eu_value *current, *cv, *bool1;
	int fbv, cbv;

	/* check for at least two arguments */
	_eucc_arity_improper(s, 2);

	/* read first argument */
	_eucc_argument_type(s, bool1, 0, EU_TYPE_BOOLEAN);
	fbv = zerooneify(_euvalue_to_bool(bool1));

	/* get cell for next argument */
	current = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));

	/* assert that all are equal */
	while (_euvalue_is_pair(current)) {
		/* get current argument */
		cv = _eupair_head(_euvalue_to_pair(current));
		_eucc_check_type(s, cv, "argument", EU_TYPE_BOOLEAN);

		/* turn it into 0 or 1 */
		cbv = zerooneify(_euvalue_to_bool(current));

		if (fbv ^ cbv) { /* values don't match */
			_eu_makebool(_eucc_return(s), EU_FALSE);
			return EU_RESULT_OK;
		}
	}

	/* all arguments matched */
	_eu_makebool(_eucc_return(s), EU_TRUE);
	return EU_RESULT_OK;
}


/**
 * @}
 */
