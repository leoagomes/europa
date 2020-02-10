#ifndef __EUROPA_ERROR_H__
#define __EUROPA_ERROR_H__

#include "europa/europa.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"

enum {
	EU_ERROR_NONE, /* it makes no sense, I know */
	EU_ERROR_READ,
	EU_ERROR_WRITE,
};

/** error type definition */
typedef struct europa_error eu_error;

/** Error type structure */
struct europa_error {
	EU_OBJ_COMMON_HEADER;

	eu_error* nested; /*!< possibly nested error. */

	int flags; /*!< error flags. */
	eu_byte _msg; /*!< start of the errors, message. */
};

/* conversion macros */

#define _euerror_to_obj(s) cast(eu_object*, s)
#define _euobj_to_error(o) cast(eu_error*, o)

#define _euvalue_to_error(v) _euobj_to_error((v)->value.object)
#define _euerror_to_value(v) { \
	.type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE, \
	.value = { .object = (s) }}

/* member access macros */

#define _euerror_message(s) (&((s)->_msg))
#define _euerror_nested(s) (&((s)->nested))

eu_error* euerror_new(europa* s, int flags, void* text, eu_error* nested);

void* euerror_message(eu_error* err);
eu_uinteger euerror_hash(eu_error* err);

#endif
