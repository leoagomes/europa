#ifndef __EUROPA_TABLE_H__
#define __EUROPA_TABLE_H__

#include "europa/europa.h"
#include "europa/int.h"
#include "europa/common.h"
#include "europa/object.h"
#include "europa/gc.h"

#include "europa/types.h"

#define _eutnode_key(n) (&((n)->key))
#define _eutnode_value(n) (&((n)->value))
#define _eutnode_next(n) ((n)->next)
#define _eutnode_from_valueptr(vptr) cast(eu_tnode*, vptr)

/* calculates 2^x */
#define twoto(x) (1 << (x))


#define _eutable_to_obj(s) cast(struct europa_object*, s)
#define _euobj_to_table(o) cast(struct europa_table*, o)

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

struct europa_table* eutable_new(europa* s, size_t count);
int eutable_destroy(europa* s, struct europa_table* t);
int eutable_mark(europa* s, europa_gc_mark mark, struct europa_table* t);
eu_uinteger eutable_hash(struct europa_table* t);
struct europa_table* eutable_set_index(struct europa_table* t, struct europa_table* i);

int eutable_create_key(europa* s, struct europa_table* t, struct europa_value* key,
	struct europa_value** val);
int eutable_define_symbol(europa* s, struct europa_table* t, void* text, struct europa_value** val);
int eutable_get(europa* s, struct europa_table* t, struct europa_value* key, struct europa_value** val);
int eutable_get_string(europa* s, struct europa_table* t, const char* str,
	struct europa_value** val);
int eutable_get_symbol(europa* s, struct europa_table* t, const char* sym_text,
	struct europa_value** val);

int eutable_rget(europa* s, struct europa_table* t, struct europa_value* key, struct europa_value** val);
int eutable_rget_string(europa* s, struct europa_table* t, const char* str,
	struct europa_value** val);
int eutable_rget_symbol(europa* s, struct europa_table* t, const char* str,
	struct europa_value** val);

#endif
