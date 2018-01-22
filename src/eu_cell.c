#include "eu_cell.h"
#include "eu_error.h"


eu_cell* eucell_new(europa* s, eu_value head, eu_value tail) {
	eu_gcobj* obj;
	eu_cell* cell;

	obj = eugc_new_object(eu_get_gc(s), EU_OBJTYPE_CELL, eucell_size());
	if (obj == NULL)
		return NULL;

	cell = eu_gcobj2cell(obj);

	cell->head = head;
	cell->tail = tail;

	return cell;
}

void eucell_mark(europa_gc* gc, eu_cell* cell) {
	/* change color to grey */
	cell->color = EUGC_COLOR_GREY;

	/* TODO: remove recursions */
	if (cell->head.type == EU_TYPE_OBJECT)
		eugc_mark(gc, cell->head.value.object);
	if (cell->tail.type == EU_TYPE_OBJECT)
		eugc_mark(gc, cell->tail.value.object);
}

void eucell_destroy(europa_gc* gc, eu_cell* cell) {
	if (cell->head.type == EU_TYPE_OBJECT)
		eugc_run_destructor(gc, cell->head.value.object);

	if (cell->tail.type == EU_TYPE_OBJECT)
		eugc_run_destructor(gc, cell->tail.value.object);
}

/* the language API */
eu_value euapi_cell_car(europa* s, eu_cell* args) {
	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "car", 0));

	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head, EU_OBJTYPE_CELL));

	return car(ccar(args));
}

eu_value euapi_cell_cdr(europa* s, eu_cell* args) {
	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head, EU_OBJTYPE_CELL));
	
	return cdr(ccar(args));
}

eu_value euapi_cell_cons(europa* s, eu_cell* args) {
	eu_value v;
	v.type = EU_TYPE_OBJECT;
	v.value.object = eucell_make_pair(s, ccar(args), car(ccdr(args)));
	return v;
}

eu_value euapi_cell_is_pair(europa* s, eu_cell* args) {
	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "pair?", 0));

	return euval_from_boolean(euvalue_is_cell(ccar(args)));
}

eu_value euapi_cell_set_car(europa* s, eu_cell* args) {
	eu_value r;

	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-car!", 0));

	if (!euvalue_is_cell(ccar(args)))
		return euerr_tovalue(euerr_bad_value_type(s, args->head, EU_OBJTYPE_CELL));

	if (euvalue_is_null(ccdr(args)))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 1));

	eu_value2cell(ccar(args))->head = car(ccdr(args));

	r.type = EU_TYPE_NULL;
	return r;
}

eu_value euapi_cell_set_cdr(europa* s, eu_cell* args) {
	eu_value r;

	if (euobj_is_null(args))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 0));

	if (!euvalue_is_cell(ccar(args))))
		return euerr_tovalue(euerr_bad_value_type(s, args->head, EU_OBJTYPE_CELL));

	if (euvalue_is_null(ccdr(args)))
		return euerr_tovalue(euerr_bad_argument_count(s, "set-cdr!", 1));

	eu_value2cell(ccar(args))->tail = car(ccdr(args));

	r.type = EU_TYPE_NULL;
	return r;
}