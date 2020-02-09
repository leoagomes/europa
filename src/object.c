/** General object and value type procedures.
 *
 * @file object.c
 * @author Leonardo G.
 */
#include "europa/object.h"

#include "europa/number.h"
#include "europa/bytevector.h"
#include "europa/string.h"
#include "europa/symbol.h"
#include "europa/pair.h"
#include "europa/table.h"
#include "europa/port.h"
#include "europa/error.h"
#include "europa/vector.h"
#include "europa/character.h"

/* global "singleton" declarations */
eu_value _null = EU_VALUE_NULL;
eu_value _true = EU_VALUE_TRUE;
eu_value _false = EU_VALUE_FALSE;
eu_value _eof = EU_VALUE_EOF;

const char* eu_type_names[] = {
	"null", "boolean", "number", "character", "eof", "symbol", "string", "error",
	"pair", "vector", "bytevector", "table", "port", "closure", "continuation",
	"prototype", "state", "global", "c-pointer", "userdata", "something-invalid"
};

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
eu_bool euobj_is_null(eu_object* obj) {
	return _euobj_is_null(obj);
}

/** Checks if the object is of a given type.
 *
 * @param obj The object.
 * @return A boolean representing whether the object is of the type.
 */
eu_bool euobj_is_type(eu_object* obj, eu_byte type) {
	if (obj == NULL)
		return (type == EU_TYPE_NULL);
	return _euobj_is_type(obj, type);
}

/** Hashes a value.
 *
 * @param v The value to hash.
 * @return The hash.
 */
eu_uinteger euvalue_hash(eu_value* v) {
	if (v == NULL)
		return 0;

	switch(_euvalue_type(v)) {
	case EU_TYPE_NULL: return 0;
	case EU_TYPE_NUMBER: return eunum_hash(v);
	case EU_TYPE_BOOLEAN: return eubool_hash(v);
	case EU_TYPE_BYTEVECTOR: return eubvector_hash(_euvalue_to_bvector(v));
	case EU_TYPE_CHARACTER: return euchar_hash(v);
	case EU_TYPE_CPOINTER: return cast(eu_integer, v->value.p);
	case EU_TYPE_STRING: return eustring_hash(_euvalue_to_string(v));
	case EU_TYPE_SYMBOL: return eusymbol_hash(_euvalue_to_symbol(v));
	case EU_TYPE_PAIR: return eupair_hash(_euvalue_to_pair(v));
	case EU_TYPE_PORT: return euport_hash(_euvalue_to_port(v));
	case EU_TYPE_TABLE: return eutable_hash(_euvalue_to_table(v));
	case EU_TYPE_VECTOR: return euvector_hash(_euvalue_to_vector(v));
	case EU_TYPE_ERROR: return euerror_hash(_euvalue_to_error(v));
	case EU_TYPE_EOF:
	case EU_TYPE_CLOSURE:
	case EU_TYPE_USERDATA:
	default:
		return 0;
	}
}

/** Checks whether two values are `eqv?`.
 *
 * @param a The first value.
 * @param b The second value.
 * @param out Where to place the result.
 * @return Whether the operation was successful.
 */
eu_result euvalue_eqv(eu_value* a, eu_value* b, eu_value* out) {
	if (a == NULL || b == NULL || out == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* always false if objects are of different types */
	if (_euvalue_type(a) != _euvalue_type(b)) {
		_eu_makebool(out, EU_FALSE);
		return EU_RESULT_OK;
	}

	switch(_euvalue_type(a)) {
	case EU_TYPE_NULL:
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;
	case EU_TYPE_NUMBER: return eunum_eqv(a, b, out);
	case EU_TYPE_BOOLEAN: return eubool_eqv(a, b, out);
	case EU_TYPE_CHARACTER: return euchar_eqv(a, b, out);
	case EU_TYPE_SYMBOL: return eusymbol_eqv(a, b, out);
	case EU_TYPE_STRING: return eustring_equal(a, b, out);
	case EU_TYPE_CPOINTER:
	case EU_TYPE_BYTEVECTOR:
	case EU_TYPE_PAIR:
	case EU_TYPE_PORT:
	case EU_TYPE_TABLE:
	case EU_TYPE_VECTOR:
	case EU_TYPE_ERROR:
	case EU_TYPE_USERDATA:
		_eu_makebool(out, _euvalue_to_obj(a) == _euvalue_to_obj(b));
		return EU_RESULT_OK;

	case EU_TYPE_EOF: /* there is one single EOF object. */
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;

	case EU_TYPE_CLOSURE: // TODO: implement
	default:
		return EU_RESULT_OK;
	}

	return EU_RESULT_INVALID;
}

/** Checks whether two values are `eq?`.
 *
 * @param a The first value.
 * @param b The second value.
 * @param out Where to place the result.
 * @return Whether the operation was successful.
 */
eu_result euvalue_eq(eu_value* a, eu_value* b, eu_value* out) {
	if (a == NULL || b == NULL || out == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* two objects can only be eq? if they're the same type */
	if (_euvalue_type(a) != _euvalue_type(b)) {
		_eu_makebool(out, EU_FALSE);
		return EU_RESULT_OK;
	}

	switch(_euvalue_type(a)) {
	case EU_TYPE_NULL:
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;
	case EU_TYPE_NUMBER: return eunum_eqv(a, b, out);
	case EU_TYPE_BOOLEAN: return eubool_eqv(a, b, out);
	case EU_TYPE_CHARACTER: return euchar_eqv(a, b, out);
	case EU_TYPE_SYMBOL: return eusymbol_eqv(a, b, out);
	case EU_TYPE_STRING: return eustring_equal(a, b, out);
	case EU_TYPE_BYTEVECTOR:
	case EU_TYPE_CPOINTER:
	case EU_TYPE_PAIR:
	case EU_TYPE_PORT:
	case EU_TYPE_TABLE:
	case EU_TYPE_VECTOR:
	case EU_TYPE_ERROR:
	case EU_TYPE_USERDATA:
		_eu_makebool(out, _euvalue_to_obj(a) == _euvalue_to_obj(b));
		return EU_RESULT_OK;

	case EU_TYPE_EOF: /* there is one single EOF object. */
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;

	case EU_TYPE_CLOSURE: // TODO: implement
	default:
		return EU_RESULT_OK;
	}

	return EU_RESULT_OK;
}

/** Checks whether two values are `equal?`.
 *
 * @param a The first value.
 * @param b The second value.
 * @param out Where to place the result.
 * @return Whether the operation was successful.
 */
eu_result euvalue_equal(eu_value* a, eu_value* b, eu_value* out) {
	if (a == NULL || b == NULL || out == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	/* two objects can only be equal? if they're the same type */
	if (_euvalue_type(a) != _euvalue_type(b)) {
		_eu_makebool(out, EU_FALSE);
		return EU_RESULT_OK;
	}

	switch(_euvalue_type(a)) {
	case EU_TYPE_NULL:
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;
	case EU_TYPE_NUMBER: return eunum_eqv(a, b, out);
	case EU_TYPE_BOOLEAN: return eubool_eqv(a, b, out);
	case EU_TYPE_CHARACTER: return euchar_eqv(a, b, out);
	case EU_TYPE_SYMBOL: return eusymbol_eqv(a, b, out);
	case EU_TYPE_STRING: return eustring_equal(a, b, out);
	case EU_TYPE_BYTEVECTOR: // TODO: implement
	case EU_TYPE_PAIR:
	case EU_TYPE_PORT:
	case EU_TYPE_TABLE:
	case EU_TYPE_VECTOR:
	case EU_TYPE_ERROR:
	case EU_TYPE_CPOINTER:
		_eu_makebool(out, _euvalue_to_obj(a) == _euvalue_to_obj(b));
		return EU_RESULT_OK;

	case EU_TYPE_EOF: /* there is one single EOF object. */
		_eu_makebool(out, EU_TRUE);
		return EU_RESULT_OK;

	case EU_TYPE_CLOSURE: // TODO: implement
	case EU_TYPE_USERDATA:
	default:
		return EU_RESULT_OK;
	}

	return EU_RESULT_OK;
}
