#include "eu_rt.h"

eu_continuation* eucont_new(europa* s, eu_short tag, eu_continuation* previous,
	eu_table* env, eu_value* rib, eu_closure* cl, eu_instruction* pc) {
	eu_continuation* cont;

	cont = eugc_new_object(s, EU_TYPE_CONTINUATION | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_continuation));
	if (cont == NULL)
		return NULL;

	cont->tag = tag;
	cont->previous = previous;
	cont->next = NULL;
	cont->env = env;
	cont->cl = cl;
	cont->pc = pc;

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
	if (cont->next) {
		_eu_checkreturn(mark(s, _eucont_to_obj(cont->next)));
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
