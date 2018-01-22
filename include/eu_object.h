#ifndef __EUROPA_OBJECTS_H__
#define __EUROPA_OBJECTS_H__

#include "eu_commons.h"
#include "eu_int.h"

enum eu_type {
	EU_TYPE_NUMBER,
	EU_TYPE_BOOLEAN,
	EU_TYPE_OBJECT,
	EU_TYPE_NULL,
	EU_TYPE_ERROR,
	EU_TYPE_POINTER,
	EU_TYPE_CHARACTER,
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
typedef struct eu_number eu_number;

struct eu_gcobj {
	EU_COMMON_HEAD;
};

union eu_values {
	eu_gcobj* object;
	eu_number num;
	int boolean;
	int character;
};

struct eu_number {
	char is_fixnum;
	union {
		int integer;
		double real;
	} value;
};

struct eu_value {
	union eu_values value;
	eu_byte type;
};

/* Global object singletons declarations */
#define EU_VALUE_NULL ((eu_value){ .type = EU_TYPE_NULL , .value = { .object = NULL }})
#define EU_VALUE_TRUE \
	((eu_value){ .value = { .boolean = EU_BOOL_TRUE }, .type = EU_TYPE_BOOLEAN})
#define EU_VALUE_FALSE \
	((eu_value){ .value = { .boolean = EU_BOOL_FALSE }, .type = EU_TYPE_BOOLEAN})


#define euvalue_is_type(v,t) ((v).type == (t))
#define euvalue_is_objtype(v,t) \
	((v).type == EU_TYPE_OBJECT && (v).value.object->type == (t))

#define euvalue_is_null(v) ((v).type == EU_TYPE_NULL)
#define euobj_is_null(obj) ((obj) == NULL)

eu_value euval_from_boolean(int v);

/* language side api */
eu_value euapi_value_is_null(europa* s, eu_cell* args);

#endif /* __EUROPA_OBJECTS_H__ */
