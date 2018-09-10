#include "eu_vector.h"

/* Vectors:
 * 
 * Like symbols and strings, vectors store their "content" along the same memory
 * block their metadata (like their length) are stored. This makes it hard to
 * resize a vector, so vectors are not resizable, their elements, though, can
 * be set!.
 */

/** Creates allocates a new vector object.
 * 
 * @param s The Europa state.
 * @param length The number of values the vector can house.
 */
eu_vector* euvector_new(europa* s, eu_integer length) {
	eu_vector* vec;

	if(s == NULL || length < 0)
		return NULL;

	vec = _euobj_to_vector(eugc_new_object(_eu_get_gc(s), 
		EU_TYPE_VECTOR | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_vector) + (sizeof(eu_value) * (length - 1))));
	if (vec == NULL)
		return NULL;

	vec->length = length;

	return vec;
}

/** Returns the length of the vector.
 * 
 * @param vec The target vector object.
 * @return The length of the given vector.
 */
eu_integer euvector_length(eu_vector* vec) {
	if (vec == NULL)
		return -1;
	return _euvector_length(vec);
}

/** Returns the continuous memory buffer that holds the values.
 * 
 * @param vec The vector object.
 * @return The address of where values are stored.
 */
eu_value* euvector_values(eu_vector* vec) {
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
eu_result euvector_mark(eu_gc* gc, eu_gcmark mark, eu_vector* vec) {
	eu_result res;
	eu_integer i;
	eu_value* values;
	eu_gcobj* obj;

	if (vec == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* mark all objects in the vector if they are collectable by the gc */
	values = _euvector_values(vec);
	for (i = 0; i < _euvector_length(vec); i++) {
		if (_euvalue_is_collectable(&(values[i]))) {
			obj = _euvalue_to_obj(&(values[i]));
			if ((res = mark(gc, obj)))
				return res;
		}
	}

	return EU_RESULT_OK;
}

/** Destroys the vector object. Freeing resources.
 * 
 * @param vec The vector object.
 * @return The result of destroying the object.
 */
eu_result euvector_destroy(eu_vector* vec) {
	return EU_RESULT_OK;
}