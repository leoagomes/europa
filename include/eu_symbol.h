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
	eu_integer hash; /*!< the symbol's hash */
	char _text; /*!< the first character of its text */
};

/* conversion macros */

#define _euobj_to_symbol(o) (cast(eu_symbol*, o))
#define _eusymbol_to_obj(s) (cast(eu_object*,s))

#define _euvalue_to_symbol(v) _euobj_to_symbol((v)->value.object)
#define _eusymbol_to_value(s) { \
	.type = EU_TYPE_SYMBOL, \
	.value = { .object = (s) }}

#define _eu_makesym(vptr, sym) do {\
		(vptr)->type = EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = _eusymbol_to_obj(sym);\
	} while (0)

/* member access macros */
#define _eusymbol_text(sym) (&((sym)->_text))
#define _eusymbol_hash(sym) ((sym)->hash)

/* function declarations */

eu_symbol* eusymbol_new(europa* s, void* text);

void* eusymbol_text(eu_symbol* sym);
eu_uinteger eusymbol_hash(eu_symbol* sym);
eu_integer eusymbol_hash_cstr(const char* str);
eu_result eusymbol_eqv(eu_value* a, eu_value* b, eu_value* out);
eu_bool eusymbol_equal_cstr(eu_value* vsym, const char* str);

/* library functions */
eu_result euapi_register_symbol(europa* s);

eu_result euapi_symbolQ(europa* s);
eu_result euapi_symbolEQ(europa* s);
eu_result euapi_symbol_to_string(europa* s);
eu_result euapi_string_to_symbol(europa* s);

#endif /* __EUROPA_SYMBOL_H__ */
