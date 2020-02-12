#ifndef __EUROPA_NUMBER_H__
#define __EUROPA_NUMBER_H__

#include "europa/europa.h"
#include "europa/object.h"
#include "europa/int.h"
#include "europa/common.h"

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
int eunum_eqv(eu_value* a, eu_value* b, eu_value* out);

int eunum_equal(europa* s, eu_value* a, eu_value* b);
int eunum_greater(europa* s, eu_value* a, eu_value* b);
int eunum_lesser(europa* s, eu_value* a, eu_value* b);

int eunum_add(europa* s, eu_value* a, eu_value* b, eu_value* out);
int eunum_subtract(europa* s, eu_value* a, eu_value* b, eu_value* out);
int eunum_multiply(europa* s, eu_value* a, eu_value* b, eu_value* out);
int eunum_divide(europa* s, eu_value* a, eu_value* b, eu_value* out);

int eunum_invert(europa* s, eu_value* a, eu_value* out);
int eunum_negate(europa* s, eu_value* a, eu_value* out);

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
int eubool_eqv(eu_value* a, eu_value* b, eu_value* out);

/* library functions */

int euapi_register_number(europa* s);

int euapi_numberQ(europa* s);
int euapi_complexQ(europa* s);
int euapi_realQ(europa* s);
int euapi_rationalQ(europa* s);
int euapi_integerQ(europa* s);

int euapi_exactQ(europa* s);
int euapi_inexactQ(europa *s);

int euapi_finiteQ(europa* s);
int euapi_infiniteQ(europa* s);

int euapi_E(europa* s);
int euapi_L(europa* s);
int euapi_G(europa* s);
int euapi_LE(europa* s);
int euapi_GE(europa* s);

int euapi_zeroQ(europa* s);
int euapi_positiveQ(europa* s);
int euapi_negativeQ(europa* s);
int euapi_oddQ(europa* s);
int euapi_evenQ(europa* s);

int euapi_min(europa* s);
int euapi_max(europa* s);

int euapi_P(europa* s);
int euapi_S(europa* s);
int euapi_M(europa* s);
int euapi_D(europa* s);

int euapi_abs(europa* s);

/* boolean */
int euapi_not(europa* s);
int euapi_booleanQ(europa* s);
int euapi_booleanEQ(europa* s);

#endif
