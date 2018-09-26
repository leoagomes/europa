/** Character-related subroutines.
 * 
 * @file character.c
 * @author Leonardo G.
 */
#include "eu_object.h"
#include "eu_character.h"
#include "eu_number.h"

/** Calculates a hash for a character.
 * 
 * @param v The character value.
 * @return The hash.
 */
eu_integer euchar_hash(eu_value* v) {
	return v->value.character * 5; /* completely arbitrary. not even sure is a good idea */
}

/** Checks whether two characters are `eqv?`.
 * 
 * @param a The first character.
 * @param b The other character.
 * @param out Where to place the result.
 * @return The result code.
 */
eu_result euchar_eqv(eu_value* a, eu_value* b, eu_value* out) {
	_eu_makebool(out, _euchar_char(a) == _euchar_char(b));
	return EU_RESULT_OK;
}