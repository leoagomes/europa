#ifndef __EUROPA_CHARACTER_H__
#define __EUROPA_CHARACTER_H__

#include "europa/europa.h"
#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

/* helper macros */
#define _eu_makechar(vptr, c) \
	do { \
		(vptr)->type = EU_TYPE_CHARACTER; \
		(vptr)->value.character = (int)c; \
	} while(0)

#define _euvalue_to_char(v) ((v)->value.character)

eu_uinteger euchar_hash(eu_value* v);
int euchar_eqv(eu_value* a, eu_value* b, eu_value* out);

#endif
