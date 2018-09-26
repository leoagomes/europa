#ifndef __EUROPA_TABLE_H__
#define __EUROPA_TABLE_H__

#include "europa.h"
#include "eu_int.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_gc.h"

typedef struct europa_table eu_table;
typedef struct europa_table_node eu_tnode;

struct europa_table_node {
	eu_value value;
	eu_value key;
	int next;
};

#define _eutnode_key(n) (&((n)->key))
#define _eutnode_value(n) (&((n)->value))
#define _eutnode_next(n) ((n)->next)

struct europa_table {
	EU_OBJ_COMMON_HEADER;
	eu_byte lsize;
	int count;
	struct europa_table_node *nodes, *last_free;
	struct europa_table* metatable;
};

#define _eutable_to_obj(s) cast(eu_gcobj*, s)
#define _euobj_to_table(o) cast(eu_table*, o)

#define _euvalue_to_table(v) _euobj_to_table((v)->value.object)
#define _eu_maketable(vptr, t) do {\
		(vptr)->type = EU_TYPE_TABLE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = (t);\
	} while (0)

#define _eutable_nodes(t) ((t)->nodes)
#define _eutable_count(t) ((t)->count)
#define _eutable_node(t, i) (&((t)->nodes[(i)]))

/* member access macros */

/* function declarations */

eu_table* eutable_new(europa* s, size_t count);
eu_result eutable_destroy(europa* s, eu_table* t);
eu_result eutable_mark(europa* s, eu_gcmark mark, eu_table* t);
eu_uinteger eutable_hash(eu_table* t);

#endif
