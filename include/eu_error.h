#ifndef __EUROPA_ERROR_H__
#define __EUROPA_ERROR_H__

#include "europa.h"
#include "eu_int.h"
#include "eu_commons.h"
#include "eu_object.h"

enum {
	EU_ERROR_NONE, /* it makes no sense, I know */
	EU_ERROR_READ,
};

/** error type definition */
typedef struct europa_error eu_error;

/** Error type structure */
struct europa_error {
	EU_OBJ_COMMON_HEADER;

	int flags; /*!< error flags. */
	eu_byte _msg; /*!< start of the errors, message. */
};

/* conversion macros */

#define _euerror_to_obj(s) cast(eu_gcobj*, s)
#define _euobj_to_error(o) cast(eu_error*, o)

#define _euvalue_to_error(v) _euobj_to_error((v)->value.object)
#define _euerror_to_value(v) { \
	.type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE, \
	.value = { .object = (s) }}

/* member access macros */

#define _euerror_message(s) (&((s)->_msg))

eu_error* euerror_new(europa* s, int flags, void* text);

void* euerror_message(eu_error* err);
eu_uinteger euerror_hash(eu_error* err);

#endif
