#ifndef __EUROPA_STRING_H__
#define __EUROPA_STRING_H__

#include <string.h>

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

	eu_integer size; /*!< the string's length. */
	eu_integer hash; /*!< the string hash. */
	char _text; /*!< the first byte of the string's text. */
};

/* conversion macros */

#define _eustring_to_obj(s) cast(eu_object*, s)
#define _euobj_to_string(o) cast(eu_string*, o)

#define _euvalue_to_string(v) _euobj_to_string((v)->value.object)
#define _eustring_to_value(v) { \
	.type = EU_TYPE_STRING, \
	.value = { .object = (s) }}

#define _eu_makestring(vptr, s) do {\
		(vptr)->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eustring_to_obj(s);\
	} while (0)

/* member access macros */

#define _eustring_text(s) (&((s)->_text))
#define _eustring_hash(s) ((s)->hash)
#define _eustring_size(s) ((s)->size)

/* function declarations */

eu_string* eustring_new(europa* s, void* text);
eu_string* eustring_withsize(europa* s, size_t textsize);

void* eustring_text(eu_string* str);
eu_uinteger eustring_hash(eu_string* str);
eu_integer eustring_hash_cstr(const char* str);
eu_integer eustring_rehash(eu_string* str);
eu_integer eustring_size(eu_string* str);

eu_result eustring_equal(eu_value* a, eu_value* b, eu_value* out);
eu_integer eustring_equal_cstr(eu_value* vstr, const char* cstr);

#endif /* __EUROPA_STRING_H__ */
