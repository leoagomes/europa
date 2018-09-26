#include "eu_table.h"

#include <stdint.h>
#include <string.h>

/* This code is heavily inspired in Lua's `ltable.c` table code.
 * 
 * The only code that is derived from Lua code is the adjust_length function
 * which comes from Lua's `luaO_ceillog2` and the twoto macro, Lua's code is
 * released under an MIT-like License. I am not entirely sure where that leaves
 * the code for functions like `adjust_length`. In any case, if you haven't
 * already, you should definitely check out Lua at http://www.lua.org. It is an
 * inspiration for this project and is straight out an amazing language that's
 * most definitely elegant and performs really well.
 * 
 * If anyone from the Lua team sees this and does not agree with the way licensing/
 * credit is handled, please do contact me.
 */

static eu_tnode _dummy = {
	.key = EU_VALUE_NULL,
	.value = EU_VALUE_NULL,
	.next = -1,
};

/* calculates 2^x */
#define twoto(x) (1 << (x))
/* calculates ceil(log2(l)) */
int ceil_log2(unsigned int length) {
	static const eu_byte log_2[256] = {/* log_2[i] = ceil(log2(i - 1)) */
		0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
		6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
		8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
	int l = 0;

	length--;
	while (length >= 256) {
		l += 8;
		length >>= 8;
	}
	return l + log_2[length];
}

/** Creates a new `nodes` array with a minimum specified length.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @param length The new minimum length.
 * @return The result of the operation.
 */
static eu_result set_nodes_length(europa* s, eu_table* t, size_t length) {
	size_t size;

	if (length == 0) {
		/* free the nodes array in case it wasn't the dummy already */
		if (t->nodes != &_dummy)
			eugc_free(_eu_gc(s), t->nodes);

		t->last_free = NULL;
		t->nodes = &_dummy;
		t->lsize = 0;
	} else {
		/* calculate the adjusted length */
		t->lsize = ceil_log2(length);
		size = twoto(t->lsize);

		t->nodes = cast(eu_tnode*,
			eugc_malloc(_eu_gc(s), sizeof(eu_tnode) * size));
		if (t->nodes == NULL) /* check for bad allocation */
			return EU_RESULT_BAD_ALLOC;

		/* all nodes are free */
		t->last_free = _eutable_node(t, size);
	}

	return EU_RESULT_OK;
}

eu_result eutable_resize(europa* s, eu_table* t, size_t new_length) {
	size_t old_llen, old_len, new_llen;
	eu_tnode* old_nodes;
	int i;

	/* check whether trying to shrink a table beyond the number of elements it
	 * has in it */
	if (_eutable_count(t) > new_length)
		return EU_RESULT_BAD_ARGUMENT;

	/* already check for the 0-length case */
	if (_eutable_nodes(t) == &_dummy && new_length == 0)
		return EU_RESULT_OK;
	else if (new_length == 0)
		return set_nodes_length(s, t, 0);

	/* adjusting lengths to fit powers of two */
	old_llen = t->lsize;
	old_len = _eutable_nodes(t) == &_dummy ? 0 : twoto(old_llen);
	new_llen = ceil_log2(new_length);
	new_length = twoto(new_llen);

	/* check whether table is already of the required length */
	if (new_llen == old_llen)
		return EU_RESULT_OK;

	/* save old node array */
	old_nodes = _eutable_nodes(t);
	/* create a new nodes array */
	if (set_nodes_length(s, t, new_length)) {
		/* if it fails, restore old nodes and return */
		_eutable_nodes(t) = old_nodes;
		return EU_RESULT_BAD_ALLOC;
	}

	/* insert old_nodes' elements into the new `nodes` */
	for (i = 0; i < old_len; i++) {
		if (!_euvalue_is_null(_eutnode_key(_eutable_node(t, i)))) {
			// TODO: implement
		}
	}

	/* free old nodes */
	eugc_free(_eu_gc(s), old_nodes);

	return EU_RESULT_OK;
}

/** Creates a new table capable of holding `length` elements.
 * 
 * @param s The Europa state.
 * @param length The amount of objects the table initially should be able to
 * hold.
 * @return The new table. NULL in case there was an error.
 */
eu_table* eutable_new(europa* s, size_t length) {
	eu_table *t;
	
	/* allocate space for the main table structure */
	t = cast(eu_table*, eugc_new_object(s, EU_TYPE_TABLE | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_table)));
	if (t == NULL)
		return NULL;

	/* initialize the node array with the specified length */
	if (set_nodes_length(s, t, length)) {
		/* error initializing nodes, return NULL */
		return NULL;
	}

	/* initialize other fields */
	t->metatable = NULL;

	return t;
}

/** Calculates a hash for the target table.
 * 
 * @param t The target table.
 * @return The hash.
 */
eu_integer eutable_hash(eu_table* t) {
	return cast(eu_integer, t);
}

/** Releases resources used by the table.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @return The result of the operation. (Always OK)
 */
eu_result eutable_destroy(europa* s, eu_table* t) {
	/* manually free the node array if applicable */
	if (t->nodes != &_dummy)
		eugc_free(_eu_gc(s), t->nodes);

	return EU_RESULT_OK;
}

/** Marks objects inside the table.
 * 
 * @param s The Europa state.
 * @param mark The marking procedure.
 * @param t The target table.
 * @return The result of the operation.
 */
eu_result eutable_mark(europa* s, eu_gcmark mark, eu_table* t) {
	int i;
	size_t len;
	eu_value* v;
	eu_tnode* n;

	len = twoto(t->lsize);
	for (i = 0; i < len; i++) {
		n = _eutable_node(t, i);
		v = _eutnode_key(n);

		if (!_euvalue_is_null(v)) {
			/* valid key try marking it */
			if (_euvalue_is_collectable(v)) {
				_eu_checkreturn(mark(s, _euvalue_to_obj(v)));
			}

			/* mark the associated value */
			v = _eutnode_value(n);
			if (!_euvalue_is_collectable(v)) {
				_eu_checkreturn(mark(s, _euvalue_to_obj(v)));
			}
		}
	}

	return EU_RESULT_OK;
}

/**
 * 
 */
eu_result eutable_get(europa* s, eu_table* t, eu_value* key) {

}

/**
 * 
 */
eu_result eutable_get_string(europa* s, eu_table* t, const char* str) {

}

/**
 * 
 */
eu_result eutable_get_symbol(europa* s, eu_table* t, const char* sym_text) {

}

/**
 * 
 */
eu_result eutable_create_key(europa* s, eu_table* t, eu_value* key) {

}