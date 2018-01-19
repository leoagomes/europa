#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "eu_object.h"
#include "eu_gc.h"

typedef struct europa europa;

struct europa {
	europa_gc* gc;
};

#define eu_get_gc(s) ((s)->gc)

#endif
