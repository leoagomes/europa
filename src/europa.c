#include "europa.h"

/** Initializes an Europa state.
 * 
 * @param s The state to initialize.
 * @return The result of the operation.
 */
eu_result eu_init(europa* s) {
	if (s == NULL)
		return EU_RESULT_NULL_ARGUMENT;

	return EU_RESULT_OK;
}
