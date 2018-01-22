#ifndef __EUROPA_CELL_H__
#define __EUROPA_CELL_H__

#include "eu_commons.h"
#include "eu_int.h"
#include "eu_object.h"

#include "europa.h"

typedef struct eu_cell eu_cell;

struct eu_cell {
	EU_COMMON_HEAD;
	eu_value head, tail;
};

#define eu_gcobj2cell(obj) cast(eu_cell*,(obj))
#define eu_cell2gcobj(cell) cast(eu_gcobj*,(cell))

#define eu_value2cell(v) (eu_gcobj2cell((v).value.object))
#define eu_cell2value(c) cast(eu_value, { .type = EU_tYPE_OBJECT, \
	.value = { .object = (c)}})

#define eucell_size() (sizeof(eu_cell))
#define eucell_head(cell) ((cell)->head)
#define eucell_tail(cell) ((cell)->tail)

#define euobj_is_cell(o) ((o)->type == EU_OBJTYPE_CELL)
#define euvalue_is_cell(v) \
	((v).type == EU_TYPE_OBJECT && euobj_is_cell((v).value.object))

#define ccar(c) ((c)->head)
#define ccdr(c) ((c)->tail)

#define car(c) ((eu_gcobj2cell((c).value.object))->head)
#define cdr(c) ((eu_gcobj2cell((c).value.object))->tail)

eu_cell* eucell_new(europa* s, eu_value head, eu_value tail);
void eucell_mark(europa_gc* gc, eu_cell* cell);
void eucell_destroy(europa_gc* gc, eu_cell* cell);

/* the language API */
eu_value euapi_cell_car(europa* s, eu_cell* args);
eu_value euapi_cell_cdr(europa* s, eu_cell* args);
eu_value euapi_cell_cons(europa* s, eu_cell* args);
eu_value euapi_cell_pair(europa* s, eu_cell* args);
eu_value euapi_cell_set_car(europa* s, eu_cell* args);
eu_value euapi_cell_set_cdr(europa* s, eu_cell* args);


#endif