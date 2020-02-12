/**
 * @file eu_object.h
 *
 * @brief Value and (garbage collected) objects header.
 * @author Leonardo G.
 */
#ifndef __EUROPA_OBJECT_H__
#define __EUROPA_OBJECT_H__

#include "europa/gc/header.h"
#include "europa/common.h"
#include "europa/int.h"

/* type definitions */
typedef struct europa_object eu_object;
typedef struct europa_value eu_value;

/* internal value representation. */
/** enum representing the possible types for tagged value */
enum eu_type {
	EU_TYPE_NULL = 0,
	EU_TYPE_BOOLEAN,
	EU_TYPE_NUMBER,
	EU_TYPE_CHARACTER,
	EU_TYPE_EOF,

	EU_TYPE_SYMBOL,
	EU_TYPE_STRING,
	EU_TYPE_ERROR,

	EU_TYPE_PAIR,
	EU_TYPE_VECTOR,
	EU_TYPE_BYTEVECTOR,
	EU_TYPE_TABLE,
	EU_TYPE_PORT,

	EU_TYPE_CLOSURE,
	EU_TYPE_CONTINUATION,
	EU_TYPE_PROTO, /* function prototype */

	EU_TYPE_STATE,
	EU_TYPE_GLOBAL,
	EU_TYPE_CPOINTER,
	EU_TYPE_USERDATA,
	EU_TYPE_LAST
};

extern const char* eu_type_names[];
#define eu_type_name(type) (eu_type_names[type > EU_TYPE_LAST ? EU_TYPE_LAST : type])

/** flag to signal if a type is garbage collected */
#define EU_TYPEFLAG_COLLECTABLE (1 << 7)
/** extra flag to be used by each type in the way it prefers. */
#define EU_TYPEFLAG_EXTRA (1 << 6)

/** helper mask to check for types */
#define EU_TYPEMASK (0xFF ^ (EU_TYPEFLAG_COLLECTABLE | EU_TYPEFLAG_EXTRA))

/** union of values for language types */
union eu_values {
	eu_object* object; /*!< garbage collected objects */

	eu_integer i; /*!< (fixnum) integer number value */
	eu_real r; /*!< (floating) real number value */

	int boolean; /*!< booleans */
	int character; /*!< characters */
	void* p; /*!< (unmanaged) c pointer */
};

/** internal value representation structure */
struct europa_value {
	union eu_values value; /*!< value itself */
	eu_byte type; /*!< the value's type */
};

/* garbage collected objects */
#define EU_OBJECT_HEADER \
	EU_OBJECT_GC_HEADER

/** Garbage Collected object abstraction. */
struct europa_object {
	EU_OBJECT_HEADER
};

/* Global object singletons declarations */
/** value struct initialization definition for the null object */
#define EU_VALUE_NULL \
	{.type = EU_TYPE_NULL, .value = {.object = NULL}}
/** effective null value singleton */
extern eu_value _null;
#define _eu_makenull(vptr) \
	do {\
		(vptr)->type = EU_TYPE_NULL;\
		(vptr)->value.object = NULL;\
	} while (0)

/** value struct initialization definition for the true object */
#define EU_VALUE_TRUE \
	{.value = {.boolean = EU_TRUE}, .type = EU_TYPE_BOOLEAN}
/** effective true value singleton */
extern eu_value _true;

/** value struct initialization definition for the true object */
#define EU_VALUE_FALSE \
	{.value = {.boolean = EU_FALSE}, .type = EU_TYPE_BOOLEAN}
/** effective false value singleton */
extern eu_value _false;

/** value struct initialization definition for the EOF object */
#define EU_VALUE_EOF \
	{.type = EU_TYPE_EOF}
/** effective eof singleton */
extern eu_value _eof;

#define _eu_makecpointer(vptr, ptr) \
	do {\
		(vptr)->type = EU_TYPE_CPOINTER;\
		(vptr)->value.p = (ptr);\
	} while (0)

#define _eu_makeeof(vptr) \
	do {\
		(vptr)->type = EU_TYPE_EOF;\
		(vptr)->value.i = 0;\
	} while (0)

/* function declarations */

/** gets the raw type of a value */
#define _euvalue_rtype(v) ((v)->type)
/** gets the object type */
#define _euvalue_type(v) (_euvalue_rtype(v) & EU_TYPEMASK)
/** checks if a value is null */
#define _euvalue_is_null(v) (_euvalue_type(v) == EU_TYPE_NULL)
/** checks if a value is of a given type */
#define _euvalue_is_type(v, t) (_euvalue_type(v) == (t))
/** checks if a value is collectable */
#define _euvalue_is_collectable(v) (((v)->type) & EU_TYPEFLAG_COLLECTABLE)
/** gets the object from a value */
#define _euvalue_to_obj(v) ((v)->value.object)

/** gets the object type */
#define _euobj_type(o) ((o)->_type & EU_TYPEMASK)
/** checks if an object is null */
#define _euobj_is_null(o) ((o) == NULL)
/** checks if an object is of a given type */
#define _euobj_is_type(o, t) (_euobj_type(o) == (t))

/* value functions */
eu_bool euvalue_is_null(eu_value* value);
eu_bool euvalue_is_type(eu_value* value, eu_byte type);
eu_bool euobj_is_null(eu_object* obj);
eu_bool euobj_is_type(eu_object* obj, eu_byte type);

eu_uinteger euvalue_hash(eu_value* v);
int euvalue_eqv(eu_value* a, eu_value* b, eu_value* out);
int euvalue_eq(eu_value* a, eu_value* b, eu_value* out);
int euvalue_equal(eu_value* a, eu_value* b, eu_value* out);

/* language side api */

#endif /* __EUROPA_OBJECT_H__ */
