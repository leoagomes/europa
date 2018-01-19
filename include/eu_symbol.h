#ifndef __EUROPA_SYMBOL_H__
#define __EUROPA_SYMBOL_H__

#include "eu_commons.h"
#include "eu_int.h"

#include "eu_object.h"

#include "europa.h"

typedef struct eu_symbol eu_symbol;

struct eu_symbol {
	EU_COMMON_HEAD;
	unsigned long hash;

	eu_uint length;
	char str;
};

#define eusym_size(len) \
	(sizeof(eu_symbol) + (len) * sizeof(eu_byte))
#define eusym_get_str(sym) (&((sym)->str))

#define eu_symbol2gcobj(sym) cast(eu_gcobj*,(sym))
#define eu_gcobj2symbol(obj) cast(eu_symbol*,(obj))

eu_symbol* eusym_new(europa* s, void* og, eu_uint len);
void eusym_destroy(europa_gc* gc, eu_symbol* sym);

#endif /* __EUROPA_SYMBOL_H__ */