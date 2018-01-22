#include "eu_gc.h"

#include "eu_string.h"
#include "eu_symbol.h"

#define eugc_malloc(gc,s) ((gc)->realloc((gc)->ud, NULL,(s)))
#define eugc_realloc(gc,ptr,s) ((gc)->realloc((gc)->ud, (ptr), (s))
#define eugc_free(gc,ptr) (gc)->free((gc)->ud, (ptr))

#define eugco_markwhite(obj) ((obj)->color = EUGC_COLOR_WHITE)
#define eugco_markgrey(obj) ((obj)->color = EUGC_COLOR_GREY)
#define eugco_markblack(obj) ((obj)->color = EUGC_COLOR_BLACK)

eu_gcobj* eugc_new_object(europa_gc* gc, eu_byte type, size_t size) {
	eu_gcobj* obj;

	obj = eugc_malloc(gc, size);
	if (!obj)
		return NULL;

	obj->next = gc->last_obj;
	gc->last_obj = obj;
	obj->color = EUGC_COLOR_WHITE;
	obj->type = type;

	return obj;
}

void eugc_mark(europa_gc* gc, eu_gcobj* obj) {
	switch (obj->type) {

	}
}

void eugc_sweep(europa_gc* gc) {
	int prev_was_white;
	eu_gcobj *last_black, *current, *aux;

	prev_was_white = 0;
	last_black = NULL;
	current = gc->last_obj;

	while (current != NULL) {
		switch (current->color) {
		case EUGC_COLOR_BLACK:
			if (prev_was_white) {
				last_black->next = current;
			}

			current->color = EUGC_COLOR_WHITE;
			last_black = current;
			current = current->next;
			break;

		case EUGC_COLOR_WHITE:
			aux = current->next;

			eugc_run_destructor(gc, current);
			eugc_free(gc, current);

			prev_was_white = 1;
			current = aux->next;
			break;

		case EUGC_COLOR_GREY:
			// this should never happen
			current = current->next;
			break;
		}
	}

	last_black->next = NULL;
	gc->last_obj = last_black;
}


void eugc_run_destructor(europa_gc* gc, eu_gcobj* obj) {
	switch(obj->type) {
		case EU_OBJTYPE_STRING:
			eustr_destroy(gc, eu_gcobj2string(obj));
			break;
		case EU_OBJTYPE_SYMBOL:
			eusym_destroy(gc, eu_gcobj2symbol(obj));
			break;
		case EU_OBJTYPE_CELL:
			eucell_destroy(gc, eu_gcobj2cell(obj));
		default:
			break;
	}
	gc->free(gc->ud, obj);
}
