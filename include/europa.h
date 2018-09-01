#ifndef __EUROPA_H__
#define __EUROPA_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_gc.h"
#include "eu_object.h"

typedef struct europa europa;

struct europa {
	/* gc lives in structure */
	eu_gc gc;
};

#define _eu_get_gc(s) (&((s)->gc))

eu_result eu_init(europa* s);

#endif /* __EUROPA_H__ */
