#ifndef __EUROPA_STRING_H__
#define __EUROPA_STRING_H__

#include <string.h>

#include "europa/europa.h"

#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

#include "europa/types.h"

/* TODO: attempt something akin to Lua's internalized and small strings */
/* also:
 * strings at the moment are just like symbols. is there a way to merge them? */

#define _eustring_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_string(o) cast(struct europa_string*, o)

#define _euvalue_to_string(v) _euobj_to_string((v)->value.object)
#define _eustring_to_value(v) { \
	.type = EU_TYPE_STRING, \
	.value = { .object = (s) }}

#define _eu_makestring(vptr, s) do {\
		(vptr)->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eustring_to_obj(s);\
	} while (0)

/* member access macros */

#define _eustring_text(s) (&((s)->text))
#define _eustring_hash(s) ((s)->hash)
#define _eustring_size(s) ((s)->size)

/* function declarations */

struct europa_string* eustring_new(europa* s, void* text);
struct europa_string* eustring_withsize(europa* s, size_t textsize);

void* eustring_text(struct europa_string* str);
eu_uinteger eustring_hash(struct europa_string* str);
eu_integer eustring_hash_cstr(const char* str);
eu_integer eustring_rehash(struct europa_string* str);
eu_integer eustring_size(struct europa_string* str);

int eustring_equal(struct europa_value* a, struct europa_value* b, struct europa_value* out);
eu_integer eustring_equal_cstr(struct europa_value* vstr, const char* cstr);

/* library */
int euapi_stringQ(europa* s);
int euapi_make_string(europa* s);
int euapi_string(europa* s);
int euapi_string_length(europa* s);
int euapi_string_ref(europa*s);
int euapi_string_setB(europa* s);
int euapi_stringEQ(europa* s);
int euapi_string_ciEQ(europa* s);
int euapi_stringLQ(europa* s);
int euapi_string_ciLQ(europa* s);
int euapi_stringGQ(europa* s);
int euapi_string_ciGQ(europa* s);
int euapi_stringLEQ(europa* s);
int euapi_string_ciLEQ(europa* s);
int euapi_stringGEQ(europa* s);
int euapi_stirng_ciGEQ(europa* s);
int euapi_string_upcase(europa* s);
int euapi_string_downcase(europa* s);
int euapi_string_foldcase(europa* s);
int euapi_substring(europa* s);
int euapi_string_append(europa* s);
int euapi_string_to_list(europa* s);
int euapi_list_to_string(europa* s);
int euapi_string_copy(europa* s);
int euapi_string_copyB(europa* s);
int euapi_string_fillB(europa* s);

#endif /* __EUROPA_STRING_H__ */
