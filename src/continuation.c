#include "eu_rt.h"

/**
 * @brief Marks continuation references.
 * 
 * @param s The Europa state.
 * @param mark The marking function.
 * @param cont The target continuation.
 * @return The result of the operation.
 */
eu_result eucont_mark(europa* s, eu_gcmark mark, eu_continuation* cont) {
	/* mark linked continuations */
	if (cont->previous) {
		_eu_checkreturn(mark(s, _eucont_to_obj(cont->previous)));
	}
	if (cont->next) {
		_eu_checkreturn(mark(s, _eucont_to_obj(cont->next)));
	}

	/* mark environment and rib */
	_eu_checkreturn(mark(s, _eutable_to_obj(cont->env)));
	_eu_checkreturn(mark(s, _eupair_to_obj(cont->rib)));

	/* mark referenced prototype */
	_eu_checkreturn(mark(s, _euproto_to_obj(cont->proto)));

	return EU_RESULT_OK;
}

/**
 * @brief (Useless) Releases resources associated to the continuation.
 * 
 * @param s The Europa state.
 * @param cont The target continuation.
 * @return The result of the operation.
 */
eu_result eucont_destroy(europa* s, eu_continuation* cont) {
	return EU_RESULT_OK;
}

/**
 * @brief Hashes a continuation.
 * 
 * Currently the hash is based only on the continuation's heap address.
 * 
 * @param cont The target continuation.
 * @return The continuation's hash.
 */
eu_integer eucont_hash(eu_continuation* cont) {
	return cast(eu_integer, cont);
}
