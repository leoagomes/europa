#include "eu_string.h"

#include <string.h>
#include "utf8.h"

eu_string* eustr_new(europa* s, void* og, eu_uint len) {
	eu_string* str;
	eu_gcobj* obj;

	obj = eugc_new_object(eu_get_gc(s), EU_TYPE_STRING, eustr_size(len + 1));
	if (obj == NULL)
		return NULL;

	str = eu_gcobj2string(obj);
	str->length = len;

	if (og)
		memcpy(eustr_get_str(str), og, len);

	return str;
}

void eustr_destroy(europa_gc* gc, eu_string* str) {
	return;
}