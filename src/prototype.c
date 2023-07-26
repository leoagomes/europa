/** Function prototype routines.
 *
 * @file prototype.c
 * @author Leonardo G.
 */
#include "europa/rt.h"
#include "europa/number.h"

/** how much to grow the code buffer */
#define CODE_GROWTH_RATE 5
/** how much to grow the constants buffer */
#define CONSTANTS_GROWTH_RATE 5
/** how much to grow the subprotos array */
#define SUBPROTOS_GROWTH_RATE 2

int resize_constants(europa* s, struct europa_prototype* proto, int size) {
	proto->constants_size = size; /* new size */
	proto->constants = _eugc_realloc(_eu_gc(s), proto->constants,
		sizeof(struct europa_value) * size);

	/* check for errors */
	if (proto->constants == NULL && size > 0)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

int resize_subprotos(europa* s, struct europa_prototype* proto, int size) {
	proto->subprotos_size = size;
	proto->subprotos = _eugc_realloc(_eu_gc(s), proto->subprotos,
		sizeof(struct europa_prototype) * size);

	/* check for error */
	if (proto->subprotos == NULL && size > 0)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

int resize_code(europa* s, struct europa_prototype* proto, int size) {
	proto->code_size = size;
	proto->code = _eugc_realloc(_eu_gc(s), proto->code,
		sizeof(eu_instruction) * size);

	/* check for errors */
	if (proto->code == NULL && size > 0)
		return EU_RESULT_BAD_ALLOC;

	return EU_RESULT_OK;
}

#define checkreturnnull(exp) if (exp) return NULL

/**
 * @brief Creates a new prototype structure.
 *
 * @param s The Europa state.
 * @param formals The formal parameters to the function.
 * @param constants_size The initial size for the constant array.
 * @param source The value for the original source.
 * @param subprotos_size The initial size for the sub-prototype array.
 * @param code_size The initial size for the prototype's code.
 * @return The created prototype.
 */
struct europa_prototype* euproto_new(europa* s, struct europa_value* formals, int constants_size,
	struct europa_value* source, int subprotos_size, int code_size) {
	struct europa_prototype* proto;

	/* fail on invalid state */
	if (!s)
		return NULL;

	/* allocate a prototype */
	proto = _euobj_to_proto(eugc_new_object(s, EU_TYPE_PROTO |
		EU_TYPEFLAG_COLLECTABLE, sizeof(struct europa_prototype)));
	if (proto == NULL)
		return NULL;

	/* set fields */
	proto->formals = *formals;
	proto->constants = NULL;
	proto->constantc = 0;
	proto->source = *source;
	proto->subprotos = NULL;
	proto->subprotoc = 0;
	proto->code = NULL;
	proto->code_length = 0;

	/* initialize to passed sizes */
	checkreturnnull(resize_constants(s, proto, constants_size));
	checkreturnnull(resize_subprotos(s, proto, subprotos_size));
	checkreturnnull(resize_code(s, proto, code_size));

	return proto;
}


#define mark_if_collectable(vptr, mark, s) \
	if (_euvalue_is_collectable(vptr)) {\
		_eu_checkreturn(mark(s, _euvalue_to_obj(vptr)));\
	}

/**
 * @brief Marks a prototype's references.
 *
 * @param s The Europa state.
 * @param mark The marking function.
 * @param p The target prototype.
 * @return The result of the operation.
 */
int euproto_mark(europa* s, europa_gc_mark mark, struct europa_prototype* p) {
	int i;

	/* mark formals list/symbol */
	mark_if_collectable(&(p->formals), mark, s)

	/* mark constants */
	for (i = 0; i < p->constantc; i++) {
		mark_if_collectable(&(p->constants[i]), mark, s)
	}

	/* mark the source object */
	mark_if_collectable(&(p->source), mark, s)

	/* mark sub prototypes */
	for (i = 0; i < p->subprotoc; i++) {
		_eu_checkreturn(mark(s, _euproto_to_obj(p->subprotos[i])));
	}

	return EU_RESULT_OK;
}

/**
 * @brief Frees the associated to this prototype.
 *
 * Frees any non-gc'd resources this prototype uses, namely its code.
 *
 * @param s The Europa state.
 * @param p The target prototype.
 * @return The result of the operation.
 */
int euproto_destroy(europa* s, struct europa_prototype* p) {
	/* only the code buffer is not garbage collected, so free it */
	if (p->code) {
		_eugc_free(_eu_gc(s), p->code);
	}

	if (p->constants) {
		_eugc_free(_eu_gc(s), p->constants);
	}

	if (p->subprotos) {
		_eugc_free(_eu_gc(s), p->subprotos);
	}

	return EU_RESULT_OK;
}

/**
 * @brief Hashes the prototype.
 *
 * Currently the hashing function is based solely on the prototype's memory
 * address.
 *
 * @param p The target prototype.
 * @return Its hash.
 */
eu_integer euproto_hash(struct europa_prototype* p) {
	return cast(eu_integer, p);
}


/**
 * @brief Appends an instruction to the prototype's code.
 *
 * @param s The Europa state.
 * @param proto The target prototype.
 * @param inst The instruction to append.
 * @return The result of the operation.
 */
eu_integer euproto_append_instruction(europa* s, struct europa_prototype* proto, eu_instruction inst) {
	/* check whether instruction fits the array */
	if (++(proto->code_length) > proto->code_size) {
		/* grow it in case it doesn't */
		_eu_checkreturn(resize_code(s, proto, proto->code_size +
			CODE_GROWTH_RATE));
	}

	/* put the instruction in the buffer */
	proto->code[proto->code_length - 1] = inst;

	return EU_RESULT_OK;
}


/**
 * @brief Adds a constant to the buffer, returning its index.
 *
 * If the given value is found to be in the constant list already, then its
 * index is returned, but nothing is added (the list won't grow in size).
 *
 * @param s The Europa state.
 * @param proto The target prototype.
 * @param constant The constant value.
 * @param index Where to place the resulting index.
 * @return The result of the operation.
 */
eu_integer euproto_add_constant(europa* s, struct europa_prototype* proto, struct europa_value* constant,
	int* index) {
	int i;
	struct europa_value out;

	/* check if constant is in the constant list already */
	for (i = 0; i < proto->constantc; i++) {
		_eu_checkreturn(euvalue_equal(&(proto->constants[i]), constant, &out));
		if (_euvalue_is_type(&out, EU_TYPE_BOOLEAN) && _euvalue_to_bool(&out)) {
			/* value found, set index and return ok */
			*index = i;
			return EU_RESULT_OK;
		}
	}

	/* value not found, we need to add it to the list */

	/* check whether constant fits the array */
	if (++(proto->constantc) > proto->constants_size) {
		/* grow it in case it doesn't */
		_eu_checkreturn(resize_constants(s, proto, proto->constants_size +
			CONSTANTS_GROWTH_RATE));
	}

	/* put the constant in the array */
	proto->constants[proto->constantc - 1] = *constant;

	/* return the index */
	if (index) *index = proto->constantc - 1;

	return EU_RESULT_OK;
}

/**
 * @brief Adds a subprototype to the subprotos array. (Growing it if necessary.)
 *
 * @param s The Europa State.
 * @param proto The target prototype.
 * @param subproto The target sub-prototype.
 * @param index Where to place the resulting index.
 * @return The result of the operation.
 */
eu_integer euproto_add_subproto(europa* s, struct europa_prototype* proto, struct europa_prototype* subproto,
	int* index) {

	/* check whether subproto fits the array */
	if (++(proto->subprotoc) > proto->subprotos_size) {
		/* grow it in case it doesn't */
		_eu_checkreturn(resize_subprotos(s, proto, proto->subprotos_size +
			SUBPROTOS_GROWTH_RATE));
	}

	/* put the subprototype in the array */
	proto->subprotos[proto->subprotoc - 1] = subproto;

	/* return the index */
	if (index) *index = proto->subprotoc - 1;

	return EU_RESULT_OK;
}



