#ifndef __EUROPA_SYMBOL_H__
#define __EUROPA_SYMBOL_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

typedef struct europa_symbol eu_symbol;

struct europa_symbol {
	EU_OBJ_COMMON_HEADER;

	void* symbol;
	eu_integer hash;
};



#endif /* __EUROPA_SYMBOL_H__ */