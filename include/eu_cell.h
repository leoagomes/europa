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

#define eucell_size() (sizeof(eu_cell))
#define eucell_get_head(cell) ((cell)->head)
#define eucell_get_tail(cell) ((cell)->tail)

eu_cell* eucell_new(europa* s, eu_value head, eu_value tail);

void eucell_destroy(europa_gc* gc, eu_cell* cell);

#endif