#ifndef __EUROPA_OBJECTS_H__
#define __EUROPA_OBJECTS_H__

#include "eu_commons.h"
#include "eu_int.h"

enum eu_type {
	EU_TYPE_OBJECT,
	EU_TYPE_NULL,
	EU_TYPE_POINTER,
	EU_TYPE_CFUNCTION
};

enum eu_objtype {
	EU_OBJTYPE_STRING,
	EU_OBJTYPE_SYMBOL,
	EU_OBJTYPE_CELL,
	EU_OBJTYPE_USERDATA
};

#define EU_COMMON_HEAD eu_gcobj* next; eu_byte type; eu_byte color

typedef struct eu_gcobj eu_gcobj;
typedef struct eu_value eu_value;

struct eu_gcobj {
	EU_COMMON_HEAD;
};

union eu_values {
	eu_gcobj* object;
	long integer;
	double real;
	int boolean;
};

struct eu_value {
	union eu_values value;
	eu_byte type;
};

#define eu_udata2gcobj(u) _cast(eu_gcobj*,s)
#define eu_table2gcobj(t) _cast(eu_gcobj*,s)

#define eu_gcobj2udata(o) _cast(eu_udata*,o)
#define eu_gcobj2table(o) _cast(eu_table*,o)

#endif /* __EUROPA_OBJECTS_H__ */
