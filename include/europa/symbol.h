#ifndef __EUROPA_SYMBOL_H__
#define __EUROPA_SYMBOL_H__

#include "europa/common.h"
#include "europa/int.h"
#include "europa/object.h"

#include "europa/europa.h"

#include "europa/types.h"

/* conversion macros */

#define _euobj_to_symbol(o) (cast(struct europa_symbol*, o))
#define _eusymbol_to_obj(s) (cast(struct europa_object*,s))

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

struct europa_symbol* eusymbol_new(europa* s, void* text);

void* eusymbol_text(struct europa_symbol* sym);
eu_uinteger eusymbol_hash(struct europa_symbol* sym);
eu_integer eusymbol_hash_cstr(const char* str);
int eusymbol_eqv(struct europa_value* a, struct europa_value* b, struct europa_value* out);
eu_bool eusymbol_equal_cstr(struct europa_value* vsym, const char* str);

/* library functions */
int euapi_register_symbol(europa* s);

int euapi_symbolQ(europa* s);
int euapi_symbolEQ(europa* s);
int euapi_symbol_to_string(europa* s);
int euapi_string_to_symbol(europa* s);

#endif /* __EUROPA_SYMBOL_H__ */
