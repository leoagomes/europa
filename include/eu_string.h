#ifndef __EUROPA_STRING_H__
#define __EUROPA_STRING_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"
#include "europa.h"

typedef struct eu_string eu_string;

struct eu_string {
	EU_COMMON_HEAD;
	eu_uint length;

	char s;
};

#define eustr_size(len) \
	(sizeof(eu_string) + (len * sizeof(eu_byte)))
#define eustr_get_str(str) &((str)->s)

#define eu_string2gcobj(str) cast(eu_gcobj*,(str))
#define eu_gcobj2string(obj) cast(eu_string*,(obj))

eu_string* eustr_new(europa* s, void* og, eu_uint len);

int eustr_compare(eu_string* a, eu_string* b);

#endif /* __EUROPA_STRING_H__ */
