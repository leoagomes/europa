#include "eu_symbol.h"

eu_symbol* eusym_new(europa* s, void* og, eu_uint len) {
	eu_gcobj* obj;
	eu_symbol* sym;

	obj = eugc_new_object(eu_get_gc(s), EU_TYPE_SYMBOL, eusym_size(len));
	if (obj == NULL)
		return NULL;

	sym = eu_gcobj2symbol(obj);
	sym->length = len;

	if (og)
		memcpy(eusym_get_str(sym), og, len);
	
	sym->hash = eutl_strb_hash(eusym_get_str(sym), len);

	return sym;
}

void eusym_destroy(europa_gc* gc, eu_symbol* s) {
}