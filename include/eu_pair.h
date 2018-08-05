#ifndef __EUROPA_PAIR_H__
#define __EUROPA_PAIR_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

#include "europa.h"

typedef struct europa_pair eu_pair;

struct europa_pair {
	EU_OBJ_COMMON_HEADER;
	eu_value head, tail;
};

/* eu_gcobj* and eu_value type checks */
/** check if object is a pair */
#define _euobj_is_pair(o) _euobj_is_type(o, EU_TYPE_PAIR)
/** check if value is a pair */
#define _euvalue_is_pair(v) _euvalue_is_type(v, EU_TYPE_PAIR)

/* type conversions to/from gcobj* */
/** converts a gcobj* to a pair* */
#define _euobj_to_pair(obj) cast(eu_pair*, (obj))
/** converts a pair* to a gcobj* */
#define _eupair_to_obj(pair) cast(eu_gcobj*, (pair))

/* type conversions to/from eu_value */
/** gets a pair* from a value* */
#define _euvalue_to_pair(v) cast(eu_pair*, (v)->value.gcobj)
/** returns a value initialization for a cell */
#define _eupair_to_value(p) { \
	.type = (p == NULL ? EU_TYPE_NULL : EU_TYPE_PAIR), \
	.value = { .object = (p) }}


/* cell related functions */
eu_pair* eupair_new(europa* s, eu_value* head, eu_value* tail);

eu_result eupair_mark(eu_gc* gc, eu_gcmark mark, eu_pair* pair);
eu_result eupair_destroy(eu_gc* gc, eu_pair* pair);

eu_integer eupair_hash(europa* s, eu_pair* pair);

/* the language API */

#endif /* __EUROPA_PAIR_H__ */