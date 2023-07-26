#ifndef __EUROPA_ERROR_H__
#define __EUROPA_ERROR_H__

#include "europa/europa.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"

#include "europa/types.h"

enum {
	EU_ERROR_NONE, /* it makes no sense, I know */
	EU_ERROR_READ,
	EU_ERROR_WRITE,
};

/* conversion macros */

#define _euerror_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_error(o) cast(struct europa_error*, o)

#define _euvalue_to_error(v) _euobj_to_error((v)->value.object)
#define _euerror_to_value(v) { \
	.type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE, \
	.value = { .object = (s) }}

/* member access macros */

#define _euerror_message(s) (&((s)->_msg))
#define _euerror_nested(s) (&((s)->nested))

struct europa_error* euerror_new(europa* s, int flags, void* text, struct europa_error* nested);

void* euerror_message(struct europa_error* err);
eu_uinteger euerror_hash(struct europa_error* err);

#endif
