#include "eu_number.h"

#include <math.h>

eu_value euvalue_from_number(eu_number num) {
	return euvalue_from_numberM(num);
}

eu_value euvalue_from_int(int i) {
	return euvalue_from_intM(i);
}

eu_value euvalue_from_double(double d) {
	return euvalue_from_doubleM(d);
}

eu_number eunum_add(eu_number a, eu_number b) {
	eu_number c;

	if (!eunum_is_fixnum(a) || !eunum_is_fixnum(b)) {
		c.is_fixnum = EU_BOOL_FALSE;
		eunum_real(c) = eunum_get_real(a) + eunum_get_real(b);
	} else {
		c.is_fixnum = EU_BOOL_TRUE;
		eunum_integer(c) = eunum_integer(a) + eunum_integer(b);
	}

	return c;
}

eu_number eunum_sub(eu_number a, eu_number b) {
	eu_number c;

	if (!eunum_is_fixnum(a) || !eunum_is_fixnum(b)) {
		c.is_fixnum = EU_BOOL_FALSE;
		eunum_real(c) = eunum_get_real(a) - eunum_get_real(b);
	} else {
		c.is_fixnum = EU_BOOL_TRUE;
		eunum_integer(c) = eunum_integer(a) - eunum_integer(b);
	}

	return c;
}

eu_number eunum_mul(eu_number a, eu_number b) {
	eu_number c;

	if (!eunum_is_fixnum(a) || !eunum_is_fixnum(b)) {
		c.is_fixnum = EU_BOOL_FALSE;
		eunum_real(c) = eunum_get_real(a) * eunum_get_real(b);
	} else {
		c.is_fixnum = EU_BOOL_TRUE;
		eunum_integer(c) = eunum_integer(a) * eunum_integer(b);
	}

	return c;
}

eu_number eunum_div(eu_number a, eu_number b) {
	eu_number c;

	if (!eunum_is_fixnum(a) || !eunum_is_fixnum(b)) {
		c.is_fixnum = EU_BOOL_FALSE;
		eunum_real(c) = eunum_get_real(a) / eunum_get_real(b);
	} else {
		c.is_fixnum = EU_BOOL_TRUE;
		eunum_integer(c) = eunum_integer(a) / eunum_integer(b);
	}

	return c;
}
eu_number eunum_intdiv(eu_number a, eu_number b) {
	eu_number c;

	c.is_fixnum = EU_BOOL_TRUE;
	c.value.integer = eunum_get_integer(a) + eunum_get_integer(b);

	return c;
}

eu_number eunum_rem(eu_number a, eu_number b) {
	eu_number c;
	long e1, e2, res;
	
	c.is_fixnum = a.is_fixnum && b.is_fixnum;
	e1 = eunum_get_integer(a);
	e2 = eunum_get_integer(b);
	res = e1 % e2;

	if (res > 0)
		if (e1 < 0)
			res -= labs(e2);
	else
		if (e1 > 0)
			res += labs(e2);

	eunum_integer(c) = res;

	return c;
}

eu_number eunum_mod(eu_number a, eu_number b) {
	eu_number c;
	long e1, e2, res;

	e1 = eunum_get_integer(a);
	e2 = eunum_get_integer(b);
	res = e1 % e2;

	if (res * e2 < 0)
		res += e2;

	eunum_integer(c) = res;

	return c;
}

eu_bool eunum_eq(eu_number a, eu_number b) {
	if (eunum_is_fixnum(a) && eunum_is_fixnum(b))
		return eunum_integer(a) == eunum_integer(b);
	else
		return eunum_get_real(a) == eunum_get_real(b);
}

eu_bool eunum_gt(eu_number a, eu_number b) {
	if (eunum_is_fixnum(a) && eunum_is_fixnum(b))
		return eunum_integer(a) > eunum_integer(b);
	else
		return eunum_get_real(a) > eunum_get_real(b);
}

eu_bool eunum_ge(eu_number a, eu_number b) {
	return !eunum_lt(a, b);
}

eu_bool eunum_lt(eu_number a, eu_number b) {
	if (eunum_is_fixnum(a) && eunum_is_fixnum(b))
		return eunum_integer(a) < eunum_integer(b);
	else
		return eunum_get_real(a) < eunum_get_real(b);
}

eu_bool eunum_le(eu_number a, eu_number b) {
	return !eunum_gt(a, b);
}