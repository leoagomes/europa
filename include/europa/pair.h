#ifndef __EUROPA_PAIR_H__
#define __EUROPA_PAIR_H__

#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

#include "europa/europa.h"

typedef struct europa_pair eu_pair;

struct europa_pair {
	EU_OBJ_COMMON_HEADER;
	eu_value head; /*!< The first/head/car value. */
	eu_value tail; /*!< The second/tail/cdr value. */
};

/* eu_object* and eu_value type checks */
/** check if object is a pair */
#define _euobj_is_pair(o) _euobj_is_type(o, EU_TYPE_PAIR)
/** check if value is a pair */
#define _euvalue_is_pair(v) _euvalue_is_type(v, EU_TYPE_PAIR)

/* type conversions to/from gcobj* */
/** converts a gcobj* to a pair* */
#define _euobj_to_pair(obj) cast(eu_pair*, (obj))
/** converts a pair* to a gcobj* */
#define _eupair_to_obj(p) cast(eu_object*, (p))

/* type conversions to/from eu_value */
/** gets a pair* from a value* */
#define _euvalue_to_pair(v) cast(eu_pair*, (v)->value.object)
/** returns a value initialization for a cell */
#define _eupair_to_value(p) { \
	.type = (p == NULL ? EU_TYPE_NULL : EU_TYPE_PAIR), \
	.value = { .object = (p) }}

#define _eupair_head(p) (&(p->head))
#define _eupair_tail(p) (&(p->tail))

#define _eu_makepair(vptr, p) do {\
		eu_pair* __PAIR__ = (p); \
		(vptr)->type = (__PAIR__ == NULL) ? EU_TYPE_NULL : EU_TYPE_PAIR | \
			EU_TYPEFLAG_COLLECTABLE; \
		(vptr)->value.object = _eupair_to_obj(__PAIR__); \
	} while (0)

/* cell related functions */
eu_pair* eupair_new(europa* s, eu_value* head, eu_value* tail);

eu_result eupair_mark(europa* s, eu_gcmark mark, eu_pair* pair);
eu_result eupair_destroy(europa* s, eu_pair* pair);

eu_uinteger eupair_hash(eu_pair* pair);

/* list operating procedures */
int eulist_is_list(europa* s, eu_value* list);
eu_value* eulist_ref(europa* s, eu_pair* list, int k);
eu_value* eulist_tail(europa* s, eu_pair* list, int k);
int eulist_length(europa* s, eu_pair* list);
eu_result eulist_copy(europa* s, eu_value* list, eu_value* out);

/* the language API */
eu_result euapi_pairQ(europa* s);
eu_result euapi_cons(europa* s);
eu_result euapi_car(europa* s);
eu_result euapi_cdr(europa* s);
eu_result euapi_set_carB(europa* s);
eu_result euapi_set_cdrB(europa* s);
eu_result euapi_nullQ(europa* s);
eu_result euapi_list(europa* s);
eu_result euapi_make_list(europa* s);
eu_result euapi_list(europa* s);
eu_result euapi_length(europa* s);
eu_result euapi_append(europa* s);
eu_result euapi_reverse(europa* s);
eu_result euapi_list_tail(europa* s);
eu_result euapi_list_ref(europa* s);
eu_result euapi_list_setB(europa* s);
eu_result euapi_list_copy(europa* s);

eu_result euapi_register_pair(europa* s);

#endif /* __EUROPA_PAIR_H__ */
