#ifndef __EUROPA_NUMBER_H__
#define __EUROPA_NUMBER_H__

#include "europa.h"
#include "eu_object.h"
#include "eu_int.h"
#include "eu_commons.h"

#define EU_NUMBER_REAL EU_TYPEFLAG_EXTRA

/* number functions */

#define _eu_makeint(vptr, num) \
	do { \
		(vptr)->type = EU_TYPE_NUMBER; \
		(vptr)->value.i = num; \
	} while(0)

#define _eu_makereal(vptr, num) \
	do { \
		(vptr)->type = EU_TYPE_NUMBER | EU_NUMBER_REAL; \
		(vptr)->value.r = num; \
	} while(0)

#define _euvalue_is_number(v) (_euvalue_type(v) == EU_TYPE_NUMBER)
#define _eunum_is_exact(v) ((_eu_rvalue(v) & EU_NUMBER_REAL) == 0)
#define _eunum_i(v) ((v)->value.i)
#define _eunum_r(v) ((v)->value.r)

eu_integer eunum_hash(eu_value* v);
eu_result eunum_eqv(eu_value* a, eu_value* b, eu_value* out);

/* boolean functions */

#define _eu_makebool(vptr, b) \
	do {\
		(vptr)->type = EU_TYPE_BOOLEAN;\
		(vptr)->value.boolean = (b);\
	} while(0)

#define _euvalue_to_bool(v) ((v)->value.boolean)

eu_integer eubool_hash(eu_value* v);
eu_result eubool_eqv(eu_value* a, eu_value* b, eu_value* out);

#endif
