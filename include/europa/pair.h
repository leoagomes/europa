#ifndef __EUROPA_PAIR_H__
#define __EUROPA_PAIR_H__

#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

#include "europa/europa.h"

#include "europa/types.h"

/* struct europa_object* and eu_value type checks */
/** check if object is a pair */
#define _euobj_is_pair(o) _euobj_is_type(o, EU_TYPE_PAIR)
/** check if value is a pair */
#define _euvalue_is_pair(v) _euvalue_is_type(v, EU_TYPE_PAIR)

/* type conversions to/from gcobj* */
/** converts a gcobj* to a pair* */
#define _euobj_to_pair(obj) cast(struct europa_pair*, (obj))
/** converts a pair* to a gcobj* */
#define _eupair_to_obj(p) cast(struct europa_object*, (p))

/* type conversions to/from eu_value */
/** gets a pair* from a value* */
#define _euvalue_to_pair(v) cast(struct europa_pair*, (v)->value.object)
/** returns a value initialization for a cell */
#define _eupair_to_value(p) { \
	.type = (p == NULL ? EU_TYPE_NULL : EU_TYPE_PAIR), \
	.value = { .object = (p) }}

#define _eupair_head(p) (&(p->head))
#define _eupair_tail(p) (&(p->tail))

#define _eu_makepair(vptr, p) do {\
		struct europa_pair* __PAIR__ = (p); \
		(vptr)->type = (__PAIR__ == NULL) ? EU_TYPE_NULL : EU_TYPE_PAIR | \
			EU_TYPEFLAG_COLLECTABLE; \
		(vptr)->value.object = _eupair_to_obj(__PAIR__); \
	} while (0)

/* cell related functions */
struct europa_pair* eupair_new(europa* s, struct europa_value* head, struct europa_value* tail);

int eupair_mark(europa* s, europa_gc_mark mark, struct europa_pair* pair);
int eupair_destroy(europa* s, struct europa_pair* pair);

eu_uinteger eupair_hash(struct europa_pair* pair);

/* list operating procedures */
int eulist_is_list(europa* s, struct europa_value* list);
struct europa_value* eulist_ref(europa* s, struct europa_pair* list, int k);
struct europa_value* eulist_tail(europa* s, struct europa_pair* list, int k);
int eulist_length(europa* s, struct europa_pair* list);
int eulist_copy(europa* s, struct europa_value* list, struct europa_value* out);

/* the language API */
int euapi_pairQ(europa* s);
int euapi_cons(europa* s);
int euapi_car(europa* s);
int euapi_cdr(europa* s);
int euapi_set_carB(europa* s);
int euapi_set_cdrB(europa* s);
int euapi_nullQ(europa* s);
int euapi_list(europa* s);
int euapi_make_list(europa* s);
int euapi_list(europa* s);
int euapi_length(europa* s);
int euapi_append(europa* s);
int euapi_reverse(europa* s);
int euapi_list_tail(europa* s);
int euapi_list_ref(europa* s);
int euapi_list_setB(europa* s);
int euapi_list_copy(europa* s);

int euapi_register_pair(europa* s);

#endif /* __EUROPA_PAIR_H__ */
