#include "eu_object.h"

#include "eu_error.h"

/* global "singleton" declarations */
eu_value _null = EU_VALUE_NULL;
eu_value _true = EU_VALUE_TRUE;
eu_value _false = EU_VALUE_FALSE;

/** Checks whether a value is of a given type.
 * 
 * @param value The value structure pointer.
 * @param type The type to check.
 * @return A boolean representing if the value is of the type.
 */
eu_bool euvalue_is_type(eu_value* value, eu_byte type) {
	if (value == NULL)
		return EU_FALSE;
	return _euvalue_is_type(value, type);
}

/** Checks whether the a given value is null.
 * 
 * @param value The value structure pointer.
 * @return A boolean representing if the value is null.
 */
eu_bool euvalue_is_null(eu_value* value) {
	/* TODO: this needs discussion:
	 * is a null value pointer similar to the null value? */
	if (value == NULL)
		return EU_FALSE;
	return _euvalue_is_null(value);
}

/** Checks whether the given object is null.
 * 
 * @param object The object.
 * @return A boolean representing if the object is null.
 */
eu_bool euobj_is_null(eu_gcobj* obj) {
	return _euobj_is_null(obj);
}

/** Checks if the object is of a given type.
 * 
 * @param obj The object.
 * @return A boolean representing whether the object is of the type.
 */
eu_bool euobj_is_type(eu_gcobj* obj, eu_byte type) {
	if (obj == NULL)
		return (type == EU_TYPE_NULL);
	return _euobj_is_type(obj, type);
}
