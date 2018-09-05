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

	char _text;
};

/* conversion macros */

#define _euobj_to_symbol(o) (cast(eu_symbol*, o))
#define _eusymbol_to_obj(s) (cast(eu_gcobj*,s))

#define _euvalue_to_symbol(v) _euobj_to_symbol((v)->value.object)
#define _eusymbol_to_value(s) { \
	.type = EU_TYPE_SYMBOL, \
	.value = { .object = (s) }}

/* member access macros */
#define _eusymbol_text(sym) (&((sym)->_text))
#define _eusymbol_hash(sym) ((sym)->hash)

/* function declarations */

eu_symbol* eusymbol_new(europa* s, void* text);

void* eusymbol_text(eu_symbol* sym);
eu_integer eusymbol_hash(eu_symbol* sym);

/* library functions */


#endif /* __EUROPA_SYMBOL_H__ */
