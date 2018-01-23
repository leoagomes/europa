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

/* eu_gcobj* and eu_value type checks */
#define euobj_is_cell(o) ((o)->type == EU_OBJTYPE_CELL)
#define euvalue_is_cell(v) \
	((v).type == EU_TYPE_OBJECT && euobj_is_cell((v).value.object))

/* type conversions to/from eu_gcobj* */
#define eu_gcobj2cell(obj) cast(eu_cell*,(obj))
#define eu_cell2gcobj(cell) cast(eu_gcobj*,(cell))

/* type conversions to/from eu_value */
#define eu_value2cell(v) (eu_gcobj2cell((v).value.object))
#define eu_cell2value(c) cast(eu_value, { \
	.type = ((c) != NULL ? EU_TYPE_OBJECT : EU_TYPE_NULL), \
	.value = { .object = (c)}})

/* guaranteed access macros */
#define eucell_size() (sizeof(eu_cell))
#define eucell_head(cell) ((cell)->head)
#define eucell_tail(cell) ((cell)->tail)

/* getters and setters **not** guaranteed to be functions nor macros */
#define eucell_get_car(c) (eucell_head((c)))
#define eucell_get_cdr(c) (eucell_tail((c)))
#define eucell_set_car(c,v) (eucell_head(c) = (v))
#define eucell_set_cdr(c,v) (eucell_tail(c) = (v))

/* these are intended as shorthands */
#define ccar(c) (eucell_get_car(c))
#define ccdr(c) (eucell_get_cdr(c))
#define car(c) ((eu_gcobj2cell((c).value.object))->head)
#define cdr(c) ((eu_gcobj2cell((c).value.object))->tail)

/* cell related functions */
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