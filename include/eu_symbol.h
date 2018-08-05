#ifndef __EUROPA_SYMBOL_H__
#define __EUROPA_SYMBOL_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

#include "europa.h"

/** type definition for the symbol type */
typedef struct europa_symbol eu_symbol;

/** symbol structure */
struct europa_symbol {
	EU_OBJ_COMMON_HEADER;

	eu_integer hash;
	void* text;

	eu_byte _text;
};

#define _euobj_to_symbol(o) (cast(eu_symbol*, o))
#define _eusymbol_to_obj(s) (cast(eu_object*,s))

#define _euvalue_to_symbol(v) euobj_to_symbol((v)->value.object)
#define _eusymbol_to_value(s) { \
	.type = EU_TYPE_SYMBOL, \
	.value = { .object = (s) }}

/* function declarations */

eu_symbol* eusymbol_new(europa* s, void* text);

/* library functions */


#endif /* __EUROPA_SYMBOL_H__ */