#ifndef __EUROPA_INTERNAL__
#define __EUROPA_INTERNAL__

#include "europa/types.h"

#define _europa_get_global(europa) ((europa)->global)
#define _europa_get_gc(europa) (&(_europa_get_global(europa)->gc))

struct europa;

struct europa_gc {
	europa_allocator allocator;
	void* allocator_userdata;
};

struct europa_global {
	struct europa* main_state;
	struct europa_gc gc;
};

struct europa_jump_list;

struct europa {
	struct europa_global* global;

	struct europa_jump_list* jump_list;
};

#define EUROPA_OBJECT_HEADER \
	unsigned long _flags; \
	struct europa_object *_previous, *_next;

struct europa_object {
	EUROPA_OBJECT_HEADER
};

#endif /* __EUROPA_INTERNAL_H__ */
