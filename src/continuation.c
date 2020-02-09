#include "europa/rt.h"

eu_continuation* eucont_new(europa* s, eu_continuation* previous, eu_table* env,
	eu_value* rib, eu_value* rib_lastpos, eu_closure* cl, unsigned int pc) {
	eu_continuation* cont;

	cont = _euobj_to_cont(eugc_new_object(s, EU_TYPE_CONTINUATION |
		EU_TYPEFLAG_COLLECTABLE, sizeof(eu_continuation)));
	if (cont == NULL)
		return NULL;

	cont->previous = previous;
	cont->env = env;
	cont->cl = cl;
	cont->pc = pc;
	cont->rib = *rib;
	cont->rib_lastpos = rib_lastpos;

	return cont;
}

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

	/* mark environment and rib */
	_eu_checkreturn(mark(s, _eutable_to_obj(cont->env)));
	if (_euvalue_is_collectable(&cont->rib)) {
		_eu_checkreturn(mark(s, _euvalue_to_obj(&cont->rib)));
	}

	/* mark referenced closure */
	if (cont->cl) {
		_eu_checkreturn(mark(s, _euproto_to_obj(cont->cl)));
	}

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
