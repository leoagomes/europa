#ifndef __EUROPA_STRING_H__
#define __EUROPA_STRING_H__

#include "europa.h"

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

/* TODO: attempt something akin to Lua's internalized and small strings */
/* also:
 * strings at the moment are just like symbols. is there a way to merge them? */

/** string type definition */
typedef struct europa_string eu_string;

/** String type structure. */
struct europa_string {
	EU_OBJ_COMMON_HEADER;

	eu_integer hash; /*!< the string hash. */
	eu_byte _text; /*!< the first byte of the string's text. */
};

/* conversion macros */

#define _eustring_to_obj(s) cast(eu_gcobj*, s)
#define _euobj_to_string(o) cast(eu_string*, o)

#define _euvalue_to_string(v) _euobj_to_string((v)->value.object)
#define _eustring_to_value(v) { \
	.type = EU_TYPE_STRING, \
	.value = { .object = (s) }}

/* member access macros */

#define _eustring_text(s) (&((s)->_text))
#define _eustring_hash(s) ((s)->hash)

/* function declarations */

eu_string* eustring_new(europa* gc, void* text);

void* eustring_text(eu_string* str);
eu_integer eustring_hash(eu_string* str);

#endif /* __EUROPA_STRING_H__ */