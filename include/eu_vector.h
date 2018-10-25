#ifndef __EUROPA_VECTOR_H__
#define __EUROPA_VECTOR_H__

#include "europa.h"

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

/** Vector structure type definition. */
typedef struct europa_vector eu_vector;

/** Vector object structure. */
struct europa_vector {
	EU_OBJ_COMMON_HEADER;

	eu_integer length; /*!< the length of the vector. */
	eu_value _value; /*!< the first value for the vector. */
};

/* conversion macros */

#define _euobj_to_vector(o) cast(eu_vector*, o)
#define _euvector_to_obj(v) cast(eu_object*, v)

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
eu_vector* euvector_new(europa* s, eu_value* data, eu_integer length);

eu_integer euvector_length(eu_vector* vec);
eu_value* euvector_values(eu_vector* vec);
eu_uinteger euvector_hash(eu_vector* vec);

eu_result euvector_mark(europa* s, eu_gcmark mark, eu_vector* vec);

#endif /* __EUROPA_VECTOR_H__ */
