#ifndef __EUROPA_VECTOR_H__
#define __EUROPA_VECTOR_H__

#include "europa/europa.h"

#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

#include "europa/types.h"

#define _euobj_to_vector(o) cast(struct europa_value*, o)
#define _euvector_to_obj(v) cast(struct europa_object*, v)

#define _euvalue_to_vector(v) _euobj_to_vector((v)->value.object)
#define _euvector_to_value(v) { \
	.type = EU_TYPE_VECTOR,\
	.value = { .object = (v) }}

#define _eu_makevector(vptr, v) do {\
		(vptr)->type = EU_TYPE_VECTOR | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _euvector_to_obj(v);\
	} while (0)

/* member access macros */

#define _euvector_length(v) ((v)->length)
#define _euvector_values(v) (&((v)->_value))
#define _euvector_ref(v, i) (_euvector_values(v) + (i))
#define _euvector_set(v, i, value) (*_euvector_ref(v, i) = (value))

/* function declarations */
struct europa_value* euvector_new(europa* s, struct europa_value* data, eu_integer length);

eu_integer euvector_length(struct europa_value* vec);
struct europa_value* euvector_values(struct europa_value* vec);
eu_uinteger euvector_hash(struct europa_value* vec);

int euvector_mark(europa* s, europa_gc_mark mark, struct europa_value* vec);

#endif /* __EUROPA_VECTOR_H__ */
