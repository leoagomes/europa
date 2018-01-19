#ifndef __EUROPA_GC_H__
#define __EUROPA_GC_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

typedef void* (*eu_realloc)(void* ud, void* ptr, eu_uint amount);
typedef void (*eu_free)(void* ud, void* ptr);

typedef struct europa_gc europa_gc;

enum eugc_color {
	EUGC_COLOR_WHITE = 0,
	EUGC_COLOR_GREY,
	EUGC_COLOR_BLACK
};

struct europa_gc {
	void* ud;
	eu_realloc realloc;
	eu_free free;

	eu_gcobj *last_obj;
};

eu_gcobj* eugc_new_object(europa_gc* gc, eu_byte type, eu_uint size);

void eugc_mark(europa_gc* gc, eu_gcobj* root);
void eugc_sweep(europa_gc* gc);

#endif /* __EUROPA_GC_H__ */
