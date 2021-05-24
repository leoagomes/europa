#ifndef __EUROPA_GC__
#define __EUROPA_GC__

#include "europa/types.h"

struct europa_gc;
struct europa_object;

int europa_gc_init(
	struct europa_gc* gc,
	europa_allocator allocator,
	void* allocator_userdata
);

struct europa_object* europa_gc_new(struct europa*, unsigned long);

#endif /* __EUROPA_GC__ */
