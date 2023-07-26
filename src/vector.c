/** Vector type operations.
 *
 * @file vector.c
 * @author Leonardo G.
 */
#include "europa/vector.h"

#include <string.h>

/* Vectors:
 *
 * Like symbols and strings, vectors store their "content" along the same memory
 * block their metadata (like their length) are stored. This makes it hard to
 * resize a vector, so vectors are not resizable, their elements, though, can
 * be set!.
 */

/** Creates a new vector object.
 *
 * @param s The Europa state.
 * @param length The number of values the vector can house.
 */
struct europa_vector* euvector_new(europa* s, struct europa_value* data, eu_integer length) {
	struct europa_vector* vec;

	if(s == NULL || length < 0)
		return NULL;

	vec = _euobj_to_vector(eugc_new_object(s,
		EU_TYPE_VECTOR | EU_TYPEFLAG_COLLECTABLE,
		sizeof(struct europa_vector) + (sizeof(struct europa_value) * (length - 1))));
	if (vec == NULL)
		return NULL;

	vec->length = length;

	if (data != NULL) {
		memcpy(_euvector_values(vec), data, length * sizeof(struct europa_value));
	}

	return vec;
}

/** Returns the length of the vector.
 *
 * @param vec The target vector object.
 * @return The length of the given vector.
 */
eu_integer euvector_length(struct europa_vector* vec) {
	if (vec == NULL)
		return -1;
	return _euvector_length(vec);
}

/** Returns the continuous memory buffer that holds the values.
 *
 * @param vec The vector object.
 * @return The address of where values are stored.
 */
struct europa_value* euvector_values(struct europa_vector* vec) {
	if (vec == NULL)
		return NULL;
	return _euvector_values(vec);
}

/** Marks all applicable objects in the vector as reachable for the GC.
 *
 * @param gc The garbage collector.
 * @param mark The gc marking function.
 * @param vec The target vector object.
 * @return The result of the marking process.
 */
int euvector_mark(europa* s, europa_gc_mark mark, struct europa_vector* vec) {
	int res;
	eu_integer i;
	struct europa_value* values;
	struct europa_object* obj;

	if (vec == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* mark all objects in the vector if they are collectable by the gc */
	values = _euvector_values(vec);
	for (i = 0; i < _euvector_length(vec); i++) {
		if (_euvalue_is_collectable(&(values[i]))) {
			obj = _euvalue_to_obj(&(values[i]));
			if ((res = mark(s, obj)))
				return res;
		}
	}

	return EU_RESULT_OK;
}

/** Calculates a hash for the vector.
 *
 * @param vec The target vector.
 * @return The hash.
 */
eu_uinteger euvector_hash(struct europa_vector* vec) {
	return cast(eu_integer, vec);
}
