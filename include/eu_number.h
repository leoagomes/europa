#ifndef __EUROPA_NUMBER_H__
#define __EUROPA_NUMBER_H__

#include "eu_int.h"
#include "eu_object.h"

/* TODO: refactor */

#define eunum_is_fixnum(num) ((num).is_fixnum)

#define eunum_get_real(num) \
	((num).is_fixnum ? (double)(num).value.integer : (num).value.real)
#define eunum_get_integer(num) \
	((num).is_fixnum ? (num).value.integer : (long)(num).value.real)

#define eu_value2integer(v) (eunum_get_integer(eu_value2num((v))))
#define eu_value2real(v) (eunum_get_real(eu_value2num((v))))

#define eunum_real(num) ((num).value.real)
#define eunum_integer(num) ((num).value.integer)

#define euvalue_from_numberM(num) \
	((eu_value){.type = EU_TYPE_NUMBER, .value = { .num = (num) }})

#define euvalue_from_intM(i) \
	((eu_value){ .type = EU_TYPE_NUMBER, .value = { \
			.num = { .is_fixnum = EU_BOOL_TRUE, .value = { .integer = (i)}}}}))

#define euvalue_from_doubleM(d) \
	((eu_value){ .type = EU_TYPE_NUMBER, .value = { \
			.num = { .is_fixnum = EU_BOOL_FALSE, .value = { .real = (d)}}}})

#define eu_number2value(n) \
	((eu_value){.type = EU_TYPE_NUMBER, .value {.num = (n)}})
#define eu_value2number(v) ((v).value.num)

#define euvalue_is_number(v) (v.type == EU_TYPE_NUMBER)


eu_value euvalue_from_number(eu_number num);
eu_value euvalue_from_int(int i);
eu_value euvalue_from_double(double d);

eu_number eunum_add(eu_number a, eu_number b);
eu_number eunum_sub(eu_number a, eu_number b);
eu_number eunum_mul(eu_number a, eu_number b);
eu_number eunum_div(eu_number a, eu_number b);
eu_number eunum_intdiv(eu_number a, eu_number b);
eu_number eunum_rem(eu_number a, eu_number b);
eu_number eunum_mod(eu_number a, eu_number b);
eu_bool eunum_eq(eu_number a, eu_number b);
eu_bool eunum_gt(eu_number a, eu_number b);
eu_bool eunum_ge(eu_number a, eu_number b);
eu_bool eunum_lt(eu_number a, eu_number b);
eu_bool eunum_le(eu_number a, eu_number b);

#endif /* __EUROPA_NUMBER_H__ */