#include "europa/gc.h"
#include "europa/internal.h"

/* helper macros */
#define _malloc(gc, size) \
	((gc)->allocator((gc)->allocator_userdata, NULL, size))
#define _free(gc, ptr) \
	((gc)->allocator((gc)->allocator_userdata, (ptr), 0))
#define _realloc(gc, ptr, size) \
	((gc)->allocator((gc)->allocator_userdata, (ptr), (size)))

/* forward declarations */

int europa_gc_init(
	struct europa_gc* gc,
	europa_allocator allocator,
	void* allocator_userdata
) {
	gc->allocator = allocator;
	gc->allocator_userdata = allocator_userdata;

	return 0;
}

struct europa_object* europa_gc_new(struct europa* europa, unsigned long size) {
	struct europa_object* object;

	object = _malloc(_europa_get_gc(europa), size);
	if (object == NULL) {
		europa_throw();
	}
}
