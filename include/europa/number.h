#ifndef __EUROPA_NUMBER_H__
#define __EUROPA_NUMBER_H__

#include "europa/eu.h"
#include "europa/object.h"
#include "europa/int.h"
#include "europa/commons.h"

#define EU_NUMBER_REAL EU_TYPEFLAG_EXTRA

/* number functions */

#define _eu_makeint(vptr, num) \
	do { \
		eu_integer __num = (num); \
		(vptr)->type = EU_TYPE_NUMBER; \
		(vptr)->value.i = __num; \
	} while(0)

#define _eu_makereal(vptr, num) \
	do { \
		eu_real __num = (num); \
		(vptr)->type = EU_TYPE_NUMBER | EU_NUMBER_REAL; \
		(vptr)->value.r = __num; \
	} while(0)

#define _euvalue_is_number(v) (_euvalue_type(v) == EU_TYPE_NUMBER)
#define _eunum_is_exact(v) ((_euvalue_rtype(v) & EU_NUMBER_REAL) == 0)
#define _eunum_i(v) ((v)->value.i)
#define _eunum_r(v) ((v)->value.r)
#define _eunum_to_real(v) (_eunum_is_exact(v) ? (cast(eu_real, _eunum_i(v))) : _eunum_r(v))
#define _eunum_to_int(v) (_eunum_is_exact(v) ? (_eunum_i(v)) : (cast(eu_integer, _eunum_r(v))))
#define _eunum_is_int(v) (_eunum_is_exact(v) ||\
	cast(eu_real, cast(eu_integer, _eunum_r(v))) == _eunum_r(v))
#define _eunum_is_zero(v) (_eunum_is_exact(v) ? (_eunum_i(v) == (eu_integer)0) :\
	(_eunum_r(v) == (eu_real)0))

eu_uinteger eunum_hash(eu_value* v);
eu_result eunum_eqv(eu_value* a, eu_value* b, eu_value* out);

int eunum_equal(europa* s, eu_value* a, eu_value* b);
int eunum_greater(europa* s, eu_value* a, eu_value* b);
int eunum_lesser(europa* s, eu_value* a, eu_value* b);

eu_result eunum_add(europa* s, eu_value* a, eu_value* b, eu_value* out);
eu_result eunum_subtract(europa* s, eu_value* a, eu_value* b, eu_value* out);
eu_result eunum_multiply(europa* s, eu_value* a, eu_value* b, eu_value* out);
eu_result eunum_divide(europa* s, eu_value* a, eu_value* b, eu_value* out);

eu_result eunum_invert(europa* s, eu_value* a, eu_value* out);
eu_result eunum_negate(europa* s, eu_value* a, eu_value* out);

/* boolean functions */

#define _eu_makebool(vptr, b) \
	do {\
		(vptr)->type = EU_TYPE_BOOLEAN;\
		(vptr)->value.boolean = (b);\
	} while(0)

#define _euvalue_to_bool(v) ((v)->value.boolean)
#define _eubool_is_false(v) (!((v)->value.boolean))
#define _eubool_is_true(v) ((v)->value.boolean)

eu_uinteger eubool_hash(eu_value* v);
eu_result eubool_eqv(eu_value* a, eu_value* b, eu_value* out);

/* library functions */

eu_result euapi_register_number(europa* s);

eu_result euapi_numberQ(europa* s);
eu_result euapi_complexQ(europa* s);
eu_result euapi_realQ(europa* s);
eu_result euapi_rationalQ(europa* s);
eu_result euapi_integerQ(europa* s);

eu_result euapi_exactQ(europa* s);
eu_result euapi_inexactQ(europa *s);

eu_result euapi_finiteQ(europa* s);
eu_result euapi_infiniteQ(europa* s);

eu_result euapi_E(europa* s);
eu_result euapi_L(europa* s);
eu_result euapi_G(europa* s);
eu_result euapi_LE(europa* s);
eu_result euapi_GE(europa* s);

eu_result euapi_zeroQ(europa* s);
eu_result euapi_positiveQ(europa* s);
eu_result euapi_negativeQ(europa* s);
eu_result euapi_oddQ(europa* s);
eu_result euapi_evenQ(europa* s);

eu_result euapi_min(europa* s);
eu_result euapi_max(europa* s);

eu_result euapi_P(europa* s);
eu_result euapi_S(europa* s);
eu_result euapi_M(europa* s);
eu_result euapi_D(europa* s);

eu_result euapi_abs(europa* s);

/* boolean */
eu_result euapi_not(europa* s);
eu_result euapi_booleanQ(europa* s);
eu_result euapi_booleanEQ(europa* s);

#endif
