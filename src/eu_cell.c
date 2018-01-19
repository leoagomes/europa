#include "eu_cell.h"

eu_cell* eucell_new(europa* s, eu_value head, eu_value tail) {
	eu_gcobj* obj;
	eu_cell* cell;

	obj = eugc_new_object(eu_get_gc(s), EU_TYPE_CELL, eucell_size());
	if (obj == NULL)
		return NULL;

	cell = eu_gcobj2cell(obj);

	cell->head = head;
	cell->tail = tail;

	return cell;
}

void eucell_destroy(europa_gc* gc, eu_cell* cell) {
	if (cell->head.type == EU_TYPE_OBJECT)
		eugc_run_destructor(gc, cell->head.value.object);

	if (cell->tail.type == EU_TYPE_OBJECT)
		eugc_run_destructor(gc, cell->tail.value.object);
}