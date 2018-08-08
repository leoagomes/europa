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
	eu_value value; /*!< the first value for the vector. */
};

/* conversion macros */

#define _euobj_to_vector(o) cast(eu_vector*, o)
#define _euvector_to_obj(v) cast(eu_gcobj*, v)

#define _euvalue_to_vector(v) _euobj_to_vector(&((v)->value.object))
#define _euvector_to_value(v) { \
	.type = EU_TYPE_VECTOR,\
	.value = { .object = (v) }}

/* member access macros */

#define _euvector_length(v) ((v)->length)
#define _euvector_values(v) (&((v)->value))

/* function declarations */
eu_vector* euvector_new(europa* s, eu_integer length);

eu_integer euvector_length(eu_vector* vec);
eu_value* euvector_values(eu_vector* vec);

eu_result euvector_mark(eu_gc* gc, eu_gcmark mark, eu_vector* vec);
eu_result euvector_destroy(eu_vector* vec);

#endif /* __EUROPA_VECTOR_H__ */