/** Character-related subroutines.
 *
 * @file character.c
 * @author Leonardo G.
 */
#include "europa/object.h"
#include "europa/character.h"
#include "europa/number.h"

/** Calculates a hash for a character.
 *
 * @param v The character value.
 * @return The hash.
 */
eu_uinteger euchar_hash(eu_value* v) {
	return v->value.character * 5; /* completely arbitrary. not even sure is a good idea */
}

/** Checks whether two characters are `eqv?`.
 *
 * @param a The first character.
 * @param b The other character.
 * @param out Where to place the result.
 * @return The result code.
 */
int euchar_eqv(eu_value* a, eu_value* b, eu_value* out) {
	_eu_makebool(out, _euvalue_to_char(a) == _euvalue_to_char(b));
	return EU_RESULT_OK;
}
