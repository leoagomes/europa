/** Table structure operations.
 * 
 * @file table.c
 * @author Leonardo G.
 */
#include "eu_table.h"
#include "eu_object.h"
#include "eu_number.h"
#include "eu_string.h"
#include "eu_symbol.h"

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
	eu_tnode* node;

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

		/* fill the nodes with empty values */
		for (node = t->nodes; node != t->last_free; node++) {
			_eu_makenull(_eutnode_key(node));
			_eu_makenull(_eutnode_value(node));
			_eutnode_next(node) = -1;
		}
	}

	return EU_RESULT_OK;
}

eu_result eutable_resize(europa* s, eu_table* t, size_t new_length) {
	size_t old_llen, old_len, new_llen;
	eu_tnode* old_nodes;
	eu_value* v;
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
	if (new_llen == old_llen && _eutable_nodes(t) != &_dummy)
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
		/* check if key-value pair is valid */
		if (!_euvalue_is_null(_eutnode_key(&(old_nodes[i])))) {
			/* insert the key into the table */
			_eu_checkreturn(eutable_create_key(s, t,
				_eutnode_key(&(old_nodes[i])), &v));
			/* copy the value into the new slot */
			*v = *(_eutnode_value(&(old_nodes[i])));
		}
	}

	/* free old nodes */
	if (old_nodes != &_dummy)
		eugc_free(_eu_gc(s), old_nodes);

	return EU_RESULT_OK;
}

/** Finds a free position in the table.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @return The node. NULL if length is 0 or no free positions.
 */
eu_tnode* eutnode_free_position(europa* s, eu_table* t) {
	if (t->last_free) {
		while (t->last_free > t->nodes) {
			t->last_free--;
			if (_euvalue_is_null(_eutnode_key(t->last_free)))
				return t->last_free;
		}
	}
	return NULL;
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
	t->count = 0;
	return t;
}

/** Calculates a hash for the target table.
 * 
 * @param t The target table.
 * @return The hash.
 */
eu_uinteger eutable_hash(eu_table* t) {
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

/** Gets a pointer to the value associated to a given key.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @param key The desired key.
 * @return A pointer to the associated value. NULL if key is not found in the
 * table.
 */
eu_result eutable_get(europa* s, eu_table* t, eu_value* key, eu_value** val) {
	eu_uinteger vhash;
	int pos;
	eu_tnode* node;
	eu_value out;

	/* return error in case any of the arguments is invalid */
	if (!s || !t || !key)
		return EU_RESULT_NULL_ARGUMENT;

	/* table has no elements */
	if (_eutable_size(t) == 0) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	/* calculate the value's hash */
	vhash = euvalue_hash(key);
	/* find the position in the table */
	pos = vhash % _eutable_size(t);
	/* get the node */
	node = _eutable_node(t, pos);

	/* check whether the found node is empty */
	if (_euvalue_is_null(_eutnode_key(node))) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	do {
		/* check if colliding element and key are the same */
		_eu_checkreturn(euvalue_eqv(key, _eutnode_key(node), &out));

		/* in case they are, return the current node's value field */
		if (_euvalue_to_bool(&out)) {
			*val = _eutnode_value(node);
			return EU_RESULT_OK;
		}

		/* we haven't found the key; try the next colliding element if there
		 * are any */
		if (_eutnode_next(node) >= 0) {
			node = _eutable_node(t, _eutnode_next(node));
		}
	} while (_eutnode_next(node) >= 0);

	/* the key wasn't found in its collision chain, so object not found */
	*val = NULL;
	return EU_RESULT_OK;
}

/** Gets a pointer associated to the value of the string key.
 * 
 * This function treats a C string as an eu_string in hashing and collision
 * resolution.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @param str The string key.
 * @param val Where to place the value pointer.
 * @return Whether the operation was succesfull.
 */
eu_result eutable_get_string(europa* s, eu_table* t, const char* str,
	eu_value** val) {
	eu_uinteger vhash;
	int pos;
	eu_tnode* node;

	/* check parameters */
	if (!s || !t || !str || !val)
		return EU_RESULT_NULL_ARGUMENT;

	/* table has no elements */
	if (_eutable_size(t) == 0) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	/* calculate the string's hash */
	vhash = eustring_hash_cstr(str);
	/* find the position in the table */
	pos = vhash % _eutable_size(t);
	/* get the node */
	node = _eutable_node(t, pos);

	/* check whether the found node is empty */
	if (_euvalue_is_null(_eutnode_key(node))) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	do {
		/* check whether the colliding value is equal to str */
		if (_euvalue_is_type(_eutnode_key(node), EU_TYPE_STRING) &&
			eustring_equal_cstr(_eutnode_key(node), str)) {
			*val = _eutnode_value(node);
			return EU_RESULT_OK;
		}

		/* we haven't found the key; try the next colliding element if there
		 * are any */
		if (_eutnode_next(node) >= 0) {
			node = _eutable_node(t, _eutnode_next(node));
		}
	} while (_eutnode_next(node) >= 0);

	*val = NULL;
	return EU_RESULT_OK;
}

/** Gets a pointer to the value associated to a symbol with a given text.
 * 
 * Like `eutable_get_cstr`, this function treats the given symbol text as a
 * symbol object and searches for a matching key.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @param sym_text The string representing the target symbol's text.
 * @param val Where to place the resulting value pointer.
 * @return Whether the operation was successful.
 */
eu_result eutable_get_symbol(europa* s, eu_table* t, const char* sym_text,
	eu_value** val) {
	eu_uinteger vhash;
	int pos;
	eu_tnode* node;
	eu_value out;

	/* check parameters */
	if (!s || !t || !sym_text || !val)
		return EU_RESULT_NULL_ARGUMENT;

	/* table has no elements */
	if (_eutable_size(t) == 0) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	/* calculate the string's hash (as if it were a symbol) */
	vhash = eusymbol_hash_cstr(sym_text);
	/* find the position in the table */
	pos = vhash % _eutable_size(t);
	/* get the node */
	node = _eutable_node(t, pos);

	/* check whether the found node is empty */
	if (_euvalue_is_null(_eutnode_key(node))) {
		*val = NULL;
		return EU_RESULT_OK;
	}

	do {
		/* check whether the colliding value is equal to str */
		if (_euvalue_is_type(_eutnode_key(node), EU_TYPE_SYMBOL) &&
			eusymbol_equal_cstr(_eutnode_key(node), sym_text)) {
			*val = _eutnode_value(node);
			return EU_RESULT_OK;
		}

		/* we haven't found the key; try the next colliding element if there
		 * are any */
		if (_eutnode_next(node) >= 0) {
			node = _eutable_node(t, _eutnode_next(node));
		}
	} while (_eutnode_next(node) >= 0);

	*val = NULL;
	return EU_RESULT_OK;
}

/** Adds a key to the table and returns a pointer to the associated value.
 * 
 * @param s The Europa state.
 * @param t The target table.
 * @param key The key to be inserted.
 * @param val Where to place the value pointer.
 * @return Whether the operation was successful.
 */
eu_result eutable_create_key(europa* s, eu_table* t, eu_value* key, eu_value** val) {
	eu_uinteger vhash;
	int pos, i;
	eu_tnode *node, *cnode, *fnode;
	eu_value out;

	/* check parameters */
	if (!s || !t || !key || !val)
		return EU_RESULT_NULL_ARGUMENT;

	/* grow the table if it does not fit an extra element */
	if (_eutable_size(t) == _eutable_count(t)) {
		_eu_checkreturn(eutable_resize(s, t, _eutable_size(t) + 1));
	}

	/* calculate key's position */
	vhash = euvalue_hash(key);
	/* find the position in the table */
	pos = vhash % _eutable_size(t);
	/* get the node */
	node = _eutable_node(t, pos);

	/* main position isn't empty */
	if (!_euvalue_is_null(_eutnode_key(node)) || _eutable_last_free(t) == NULL) {
		/* find a free position in the table */
		fnode = eutnode_free_position(s, t);
		if (fnode == NULL) {
			/* considering we already growed the table, this is a strange error */
			return EU_RESULT_ERROR;
		}

		/* get the colliding node */
		cnode = _eutable_node(t, euvalue_hash(_eutnode_key(node)) % _eutable_size(t));

		if (cnode != node) { /* colliding node isn't in main position */
			/* find whatever node previously pointed to it */
			while (_eutable_nodes(t) + _eutnode_next(cnode) != node)
				cnode = _eutable_nodes(t) + _eutnode_next(cnode);
			/* update its 'next' field */
			_eutnode_next(cnode) = fnode - _eutable_nodes(t);
			/* place the colliding node in the free slot */
			*fnode = *node;

			/* mark the main position node as free */
			_eutnode_next(node) = -1;
			*_eutnode_key(node) = _null;
			/* update fnode to point to the insertion point */
			fnode = node;
		} else {
			/* in case the colliding key is in its main position, we need to add
			 * 'key' to the collision chain */

			/* set the `next` field of the new node to point to the "tail" of
			 * the collision chain (if you consider the node at its main position
			 * a 'head' of this list) */
			fnode->next = node->next;
			/* set the next of the "colision chain's head" to be the offset of
			 * the added node */
			node->next = fnode - _eutable_nodes(t);

			/* all that's left to do now is set the fnode's key to `key` and 
			 * return a pointer to its `value` field */
		}
	} else { /* main position is empty */
		/* set fnode to it */
		fnode = node;
		/* set its `next` to the "end" value*/
		_eutnode_next(fnode) = -1;
	}
	/* whenever we reach this point we're at three possible situations:
	 * a) the key's main position is empty.
	 *    In which case, `fnode` points to the correct position and has the
	 *    correct `next` value (-1).
	 * b) the key's main position wasn't empty, but the colliding key was **not**
	 *    in it's main position.
	 *    In this case, the colliding key is now at a new free position,
	 *    correctly set up, and `fnode` points to the key's main position and has
	 *    the correct `next` value (-1).
	 * c) the key's main position wasn't empty and the colliding key **was** in
	 *    its main position.
	 *    In which case, the colliding node already points to `fnode` and `fnode`
	 *    has the correct `next` value (the same as the colliding node had prior
	 *    to this key's insertion).
	 * 
	 * In any of the cases above, the only thing left to do is properly set `fnode`'s
	 * `key` and return the address of it's `value` field.
	 */

	_eutable_count(t) += 1; /* increase the number of elements in table */
	fnode->key = *key; /* set the free node's key */
	*val = _eutnode_value(fnode); /* return the address of it's value field */
	return EU_RESULT_OK; /* everything went fine */
}