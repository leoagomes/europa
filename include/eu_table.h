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
	/* because of an ugly hack below, value needs to be the first field
	 * (this forces &tn->value and tn to the same value and the conversion cast
	 * works)
	 */
	eu_value value;
	eu_value key;
	int next;
};

#define _eutnode_key(n) (&((n)->key))
#define _eutnode_value(n) (&((n)->value))
#define _eutnode_next(n) ((n)->next)
#define _eutnode_from_valueptr(vptr) cast(eu_tnode*, vptr)

/* calculates 2^x */
#define twoto(x) (1 << (x))

struct europa_table {
	EU_OBJ_COMMON_HEADER;
	eu_byte lsize; /*!< log2 of the table's size */
	int count; /*!< the number of elements in the table */
	struct europa_table_node *nodes, *last_free;

	struct europa_table* index; /*!< the table's index */
};

#define _eutable_to_obj(s) cast(eu_object*, s)
#define _euobj_to_table(o) cast(eu_table*, o)

#define _euvalue_to_table(v) _euobj_to_table((v)->value.object)
#define _eu_maketable(vptr, t) do {\
		(vptr)->type = EU_TYPE_TABLE | EU_TYPEFLAG_COLLECTABLE;\
		(vptr)->value.object = (t);\
	} while (0)

#define _eutable_nodes(t) ((t)->nodes)
#define _eutable_count(t) ((t)->count)
#define _eutable_node(t, i) (&((t)->nodes[(i)]))
#define _eutable_lsize(t) ((t)->lsize)
#define _eutable_last_free(t) ((t)->last_free)
#define _eutable_size(t) (_eutable_last_free(t) ? twoto(_eutable_lsize(t)) : 0)
#define _eutable_index(t) ((t)->index)

#define _eutable_set_index(t, i) (_eutable_index(t) = (i))

/* member access macros */

/* function declarations */

eu_table* eutable_new(europa* s, size_t count);
eu_result eutable_destroy(europa* s, eu_table* t);
eu_result eutable_mark(europa* s, eu_gcmark mark, eu_table* t);
eu_uinteger eutable_hash(eu_table* t);
eu_table* eutable_set_index(eu_table* t, eu_table* i);

eu_result eutable_create_key(europa* s, eu_table* t, eu_value* key,
	eu_value** val);
eu_result eutable_define_symbol(europa* s, eu_table* t, void* text, eu_value** val);
eu_result eutable_get(europa* s, eu_table* t, eu_value* key, eu_value** val);
eu_result eutable_get_string(europa* s, eu_table* t, const char* str,
	eu_value** val);
eu_result eutable_get_symbol(europa* s, eu_table* t, const char* sym_text,
	eu_value** val);

eu_result eutable_rget(europa* s, eu_table* t, eu_value* key, eu_value** val);
eu_result eutable_rget_string(europa* s, eu_table* t, const char* str,
	eu_value** val);
eu_result eutable_rget_symbol(europa* s, eu_table* t, const char* str,
	eu_value** val);

#endif
