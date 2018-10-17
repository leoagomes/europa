#include "eu_rt.h"

#include "eu_symbol.h"
#include "eu_error.h"

enum {
	OP_NOP,
	OP_REFER,
	OP_CONST,
	OP_CLOSE,
	OP_TEST,
	OP_JUMP,
	OP_ASSIGN,
};

/* warning: currently assumes 32-bit */
/* adapting the following values should be enough, though */
#define OPCMASK 0xFF
#define OPCSHIFT 24
#define VALMASK 0xFFFFFF
#define OFFBIAS (0xFFFFFF >> 1)

#define opc_part(op) ((op & OPCMASK) << OPCSHIFT)
#define val_part(v) (v & VALMASK)
#define offset_part(off) ((OFFBIAS) + off)

/* opcode generation helper macros */
#define IREFER(k) (opc_part(OP_REFER) | val_part(k))
#define ICONST(k) (opc_part(OP_CONST) | val_part(k))
#define IASSIGN(k) (opc_part(OP_ASSIGN) | val_part(k))
#define ICLOSE(subindex) (opc_part(OP_CLOSE) | val_part(subindex))
#define ITEST(off) (opc_part(OP_TEST) | offset_part(off))
#define IJUMP(off) (opc_part(OP_JUMP) | offset_part(off))

eu_result compile(europa* s, eu_proto* proto, eu_value* v);

/* this function's return value does not take into account the improper end of
 * the list:
 * list_length(s, (a b c)) = 3
 * list_length(s, (a b . c)) = 2
 * 
 * the number of elements may be considered (return + improper)
 * */
int list_length(europa* s, eu_value* v, int* improper) {
	int count;

	/* empty list: length = 0 */
	if (_euvalue_is_null(v))
		return 0;

	/* not even a list */
	if (!_euvalue_is_type(v, EU_TYPE_PAIR))
		return -1;

	/* count number of elements */
	count = 0;
	*improper = 0;
	while (!_euvalue_is_null(v)) {
		count++;
		v = _eupair_tail(_euvalue_to_pair(v));

		/* check if improper list */
		if (!_euvalue_is_type(v, EU_TYPE_PAIR)) {
			*improper = 1; /* mark improper */
			break;
		}
	}
	return count;
}

eu_result compile_application(europa* s, eu_proto* proto, eu_value* v) {
	eu_value *head, *tail;
	int length, improper, index, aux;
	eu_proto* subproto;
	eu_symbol* beginsymbol;
	eu_value beginsym, beginpair;

	head = _eupair_head(_euvalue_to_pair(v));
	tail = _eupair_tail(_euvalue_to_pair(v));

	/* check whether head is a symbol */
	if (euvalue_is_type(head, EU_TYPE_SYMBOL)) {
		/* in which case it may be a special construct */
		if (eusymbol_equal_cstr(head, "quote")) {
			/* make sure arity is correct */
			length = list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"quote can't be called in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 1) {
				_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad quote arity: expected 1 argument, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* add the quote's argument to the constant list */
			tail = _eupair_head(_euvalue_to_pair(tail));
			_eu_checkreturn(euproto_add_constant(s, proto, tail, &index));

			/* add the const instruction to the code */
			_eu_checkreturn(euproto_append_instruction(s, proto, ICONST(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "lambda")) {
			/* check arity */
			length = list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"lambda can't be used with an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length < 2) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"lambda expects at least <formals> and one expression for the <body>."));
				return EU_RESULT_ERROR;
			}

			/* compile the function's body */
			head = _eupair_head(_euvalue_to_pair(tail)); /* get the formals */
			tail = _eupair_tail(_euvalue_to_pair(tail)); /* get the list containing the body */

			/* initialize the begin cell */
			_eu_makesym(&beginsym, eusymbol_new(s, "begin"));
			_eu_makepair(&beginpair, eupair_new(s, &beginsym, tail));

			/* create a prototype from the formals (in head) and source (in v) */
			subproto = euproto_new(s, head, 0, v, 0, 0);
			/* compile the body (with the prepended "begin") */
			_eu_checkreturn(compile_application(s, subproto, tail));
			/* add the compiled prototype as a subprototype */
			_eu_checkreturn(euproto_add_subproto(s, proto, subproto, &index));
			/* add a close instruction */
			_eu_checkreturn(euproto_append_instruction(s, proto, ICLOSE(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "if")) {
			/* check arity */
			length = list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"if can't be used with improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 3) {
				_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad if arity: expected 3 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* compile the test argument */
			head = _eupair_head(_euvalue_to_pair(tail)); /* test */
			_eu_checkreturn(compile(s, proto, head));

			/* because the offset of the false branch is not yet known (no
			 * branches have been compiled), insert a placeholder offset and
			 * save the instruction's index in order to update it later */
			_eu_checkreturn(euproto_append_instruction(s, proto, ITEST(0)));
			index = proto->code_length - 1; /* get instruction index */

			/* compile true branch */
			tail = _eupair_tail(_euvalue_to_pair(tail)); /* (true . (false . '())) */
			head = _eupair_head(_euvalue_to_pair(tail)); /* true */
			_eu_checkreturn(compile(s, proto, head));

			/* add a jump instruction to skip the false branch, where to jump
			 * is still not defined, though, so we'll have to do the same as
			 * with the test instruction */
			length = proto->code_length; /* the index of the jump instruction */
			_eu_checkreturn(euproto_append_instruction(s, proto, ITEST(0)));

			/* compile false branch */
			tail = _eupair_tail(_euvalue_to_pair(tail)); /* (false . '()) */
			head = _eupair_head(_euvalue_to_pair(tail)); /* false */
			_eu_checkreturn(compile(s, proto, head));
			improper = proto->code_length; /* the index of the instruction after the false branch*/

			/* update the test instruction */
			proto->code[index] = ITEST(length + 1 - index);
			/* update the jump instruction */
			proto->code[length] = IJUMP(improper - length);

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "set!")) {
			/* check arity */
			length = list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"set! can't be used in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 2) {
				_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad set! arity: expected 2 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* check if variable name parameter is actually a symbol */
			head = _eupair_head(_euvalue_to_pair(tail));
			if (!_euvalue_is_type(head, EU_TYPE_SYMBOL)) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"bad set! syntax, expected first argument to be an identifier (symbol)."));
				return EU_RESULT_ERROR;
			}

			/* compile the value parameter */
			tail = _eupair_head(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(tail))));
			_eu_checkreturn(compile(s, proto, tail));

			/* add name symbol to the constant list */
			_eu_checkreturn(euproto_add_constant(s, proto, head, &index));
			/* append the assign instruction */
			_eu_checkreturn(euproto_append_instruction(s, proto, IASSIGN(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "call/cc")
			|| eusymbol_equal_cstr(head, "call-with-current-continuation")) {
			
		} else if (eusymbol_equal_cstr(head, "begin")) {
			
		}
	}

	return EU_RESULT_OK;
}

eu_result compile(europa* s, eu_proto* proto, eu_value* v) {
	int index;

	switch (_euvalue_type(v)) {
	case EU_TYPE_SYMBOL: /* symbol: variable reference */
		/* add symbol to the prototype's constants */
		_eu_checkreturn(euproto_add_constant(s, proto, v, &index));

		/* add refer instruction to the code */
		_eu_checkreturn(euproto_append_instruction(s, proto, IREFER(index)));
		break;
	case EU_TYPE_PAIR: /* function call */
		/* call function responsible for compiling function applications */
		_eu_checkreturn(compile_application(s, proto, v));
		break;
	default: /* constant value */
		/* add constant to the prototype */
		_eu_checkreturn(euproto_add_constant(s, proto, v, &index));

		/* add const instruction to the code */
		_eu_checkreturn(euproto_append_instruction(s, proto, ICONST(index)));
		break;
	}

	return EU_RESULT_OK;
}

/**
 * @brief Compiles a value into a chunk.
 * 
 * Compiles a value into a chunk: a closure that when evaluated produces
 * whatever v evaluates to.
 * 
 * @param s The Europa state.
 * @param v The value to be compiled.
 * @param chunk Where to place the resulting chunk.
 * @return The result of the operation.
 */
eu_result eucode_compile(europa* s, eu_value* v, eu_value* chunk) {
	eu_proto* top;
	eu_value argssym;

	/* create a symbol that represents the top level prototype's formals */
	_eu_makesym(&argssym, eusymbol_new(s, "**chunk-args**"));
	if (_euvalue_to_obj(&argssym) == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* create the top level prototype */
	top = euproto_new(s, &argssym, 0, v, 0, 0);
	if (top == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* top level compile call */

	/* create closure from top level prototype */

	/* return the closure */

	return EU_RESULT_OK;
}


