#include "eu_rt.h"

#include "eu_symbol.h"
#include "eu_error.h"
#include "eu_util.h"


#define opc_part(op) ((op & OPCMASK) << OPCSHIFT)
#define val_part(v) (v & VALMASK)
#define offset_part(off) ((OFFBIAS) + off)

/* opcode generation helper macros */
#define IREFER(k) (opc_part(EU_OP_REFER) | val_part(k))
#define ICONST(k) (opc_part(EU_OP_CONST) | val_part(k))
#define IASSIGN(k) (opc_part(EU_OP_ASSIGN) | val_part(k))
#define ICLOSE(subindex) (opc_part(EU_OP_CLOSE) | val_part(subindex))
#define ITEST(off) (opc_part(EU_OP_TEST) | offset_part(off))
#define IJUMP(off) (opc_part(EU_OP_JUMP) | offset_part(off))
#define IARGUMENT() (opc_part(EU_OP_ARGUMENT) | val_part(0))
#define ICONTI(off) (opc_part(EU_OP_CONTI) | offset_part(off))
#define IAPPLY() (opc_part(EU_OP_APPLY) | val_part(0))
#define IRETURN() (opc_part(EU_OP_RETURN) | val_part(0))
#define IFRAME(return_to) (opc_part(EU_OP_FRAME) | offset_part(return_to))
#define IDEFINE(k) (opc_part(EU_OP_DEFINE) | val_part(k))

eu_result compile(europa* s, eu_proto* proto, eu_value* v, int is_tail);

eu_result check_formals(europa* s, eu_value* formals) {
	eu_value* v;

	/* invalid type for formals part */
	if (!_euvalue_is_type(formals, EU_TYPE_SYMBOL)
		&& !_euvalue_is_type(formals, EU_TYPE_PAIR)
		&& !_euvalue_is_type(formals, EU_TYPE_NULL)) {
		_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
			"Invalid type for lambda formals. Expected symbol or pair (list)."));
		return EU_RESULT_ERROR;
	}

	/* identifier for formals */
	if (_euvalue_is_type(formals, EU_TYPE_SYMBOL))
		return EU_RESULT_OK;

	/* list of identifiers, possibly improper */
	for(v = formals; _euvalue_is_type(v, EU_TYPE_PAIR);
		v = _eupair_tail(_euvalue_to_pair(v))) {
		/* check if pair's head (the name) is actually a symbol */
		if (!_euvalue_is_type(_eupair_head(_euvalue_to_pair(v)), EU_TYPE_SYMBOL))
			goto formal_not_symbol;
	}

	/* if v isn't null, the list was improper */
	if (!_euvalue_is_null(v) && !_euvalue_is_type(v, EU_TYPE_SYMBOL))
		goto formal_not_symbol;

	return EU_RESULT_OK;

	formal_not_symbol:
	_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
		"Invalid formal list parameter type. Expected formals to be symbols."));
	return EU_RESULT_ERROR;
}

eu_result compile_application(europa* s, eu_proto* proto, eu_value* v, int is_tail) {
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
		if (eusymbol_equal_cstr(head, "quote")) { /* (quote obj) */
			/* make sure arity is correct */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"quote can't be called in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 1) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad quote arity: expected 1 argument, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* add the quote's argument to the constant list */
			tail = _eupair_head(_euvalue_to_pair(tail));
			_eu_checkreturn(euproto_add_constant(s, proto, tail, &index));

			/* add the const instruction to the code */
			_eu_checkreturn(euproto_append_instruction(s, proto, ICONST(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "lambda")) { /* (lambda formals body...) */
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"lambda can't be used with an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length < 2) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"lambda expects at least <formals> and one expression for the <body>."));
				return EU_RESULT_ERROR;
			}

			/* compile the function's body */
			head = _eupair_head(_euvalue_to_pair(tail)); /* get the formals */
			tail = _eupair_tail(_euvalue_to_pair(tail)); /* get the list containing the body */

			/* check whether formals are valid */
			_eu_checkreturn(check_formals(s, head));

			/* initialize the begin cell */
			_eu_makesym(&beginsym, eusymbol_new(s, "begin"));
			_eu_makepair(&beginpair, eupair_new(s, &beginsym, tail));

			/* create a prototype from the formals (in head) and source (in v) */
			subproto = euproto_new(s, head, 0, v, 0, 0);
			/* compile the body (with the prepended "begin") */
			_eu_checkreturn(compile_application(s, subproto, &beginpair, 1));
			/* add a return instruction */
			_eu_checkreturn(euproto_append_instruction(s, subproto, IRETURN()));
			/* add the compiled prototype as a subprototype */
			_eu_checkreturn(euproto_add_subproto(s, proto, subproto, &index));
			/* add a close instruction */
			_eu_checkreturn(euproto_append_instruction(s, proto, ICLOSE(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "if")) { /* (if test then else) */
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"if can't be used with improper list."));
				return EU_RESULT_ERROR;
			}
			if (length > 3 || length <= 1) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad if arity: expected 2 or 3 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* compile the test argument */
			head = _eupair_head(_euvalue_to_pair(tail)); /* test */
			_eu_checkreturn(compile(s, proto, head, 0)); /* test is never in tail position */

			/* because the offset of the false branch is not yet known (no
			 * branches have been compiled), insert a placeholder offset and
			 * save the instruction's index in order to update it later */
			index = proto->code_length; /* get instruction index */
			_eu_checkreturn(euproto_append_instruction(s, proto, ITEST(0)));

			/* compile true branch */
			tail = _eupair_tail(_euvalue_to_pair(tail)); /* (true . (false . '())) */
			head = _eupair_head(_euvalue_to_pair(tail)); /* true */
			_eu_checkreturn(compile(s, proto, head, is_tail)); /* is in tail position if this is in tail position*/

			/* check if there is a false branch */
			if (!_euvalue_is_null(_eupair_tail(_euvalue_to_pair(tail)))) {
				/* add a jump instruction to skip the false branch, where to jump
				* is still not defined, though, so we'll have to do the same as
				* with the test instruction */
				length = proto->code_length; /* the index of the jump instruction */
				_eu_checkreturn(euproto_append_instruction(s, proto, IJUMP(0)));

				/* compile false branch */
				tail = _eupair_tail(_euvalue_to_pair(tail)); /* (false . '()) */
				head = _eupair_head(_euvalue_to_pair(tail)); /* false */
				_eu_checkreturn(compile(s, proto, head, is_tail)); /* is in tail position if this is in tail position */
				improper = proto->code_length; /* the index of the instruction after the false branch*/

				/* update the test instruction */
				proto->code[index] = ITEST(length + 1 - index);
				/* update the jump instruction */
				proto->code[length] = IJUMP(improper - length);
			} else {
				/* update the test statement to point to after the true branch */
				proto->code[index] = ITEST(proto->code_length - index);
			}

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "set!")) { /* (set! var value) */
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"set! can't be used in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 2) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad set! arity: expected 2 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* check if variable name parameter is actually a symbol */
			head = _eupair_head(_euvalue_to_pair(tail));
			if (!_euvalue_is_type(head, EU_TYPE_SYMBOL)) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"bad set! syntax, expected first argument to be an identifier (symbol)."));
				return EU_RESULT_ERROR;
			}

			/* compile the value parameter */
			tail = _eupair_head(_euvalue_to_pair(_eupair_tail(_euvalue_to_pair(tail))));
			_eu_checkreturn(compile(s, proto, tail, 0)); /* the set variable name is never in tail position */

			/* add name symbol to the constant list */
			_eu_checkreturn(euproto_add_constant(s, proto, head, &index));
			/* append the assign instruction */
			_eu_checkreturn(euproto_append_instruction(s, proto, IASSIGN(index)));

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "define")) { /* (define name thing) || (define (name args) body...) || (define (name . arglist) body...) */
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"define can't be used in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length < 2) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad define arity: expected at least 2 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* make head be name or (name args ...) or (name . arglist) */
			head = _eupair_head(_euvalue_to_pair(tail));
			/* cell of third parameter */
			tail = _eupair_tail(_euvalue_to_pair(tail));

			/* (define name value) */
			if (_euvalue_is_type(head, EU_TYPE_SYMBOL)) {

				/* compile the value parameter */
				/* the set variable name is never in tail position */
				_eu_checkreturn(compile(s, proto,
					_eupair_head(_euvalue_to_pair(tail)), 0));

				/* add name symbol to the constant list */
				_eu_checkreturn(euproto_add_constant(s, proto, head, &index));
				/* append the define instruction */
				_eu_checkreturn(euproto_append_instruction(s, proto, IDEFINE(index)));

				return EU_RESULT_OK;
			}

			/* (define (name args ...) body ...) also (name args . named) */
			if (_euvalue_is_type(head, EU_TYPE_PAIR)) {

				/* check whether formals are valid */
				_eu_checkreturn(check_formals(s, _eupair_tail(_euvalue_to_pair(head))));

				/* initialize the begin cell */
				_eu_makesym(&beginsym, eusymbol_new(s, "begin"));
				_eu_makepair(&beginpair, eupair_new(s, &beginsym, tail));

				/* create a prototype from the formals (in head's cdr) and source (in v) */
				subproto = euproto_new(s, _eupair_tail(_euvalue_to_pair(head)), 0, v, 0, 0);
				/* compile the body (with the prepended "begin") */
				_eu_checkreturn(compile_application(s, subproto, &beginpair, 1));
				/* add a return instruction */
				_eu_checkreturn(euproto_append_instruction(s, subproto, IRETURN()));
				/* add the compiled prototype as a subprototype */
				_eu_checkreturn(euproto_add_subproto(s, proto, subproto, &index));
				/* add a close instruction */
				_eu_checkreturn(euproto_append_instruction(s, proto, ICLOSE(index)));

				/* add name symbol to the constant list */
				_eu_checkreturn(euproto_add_constant(s, proto, _eupair_head(_euvalue_to_pair(head)), &index));
				/* append the define instruction */
				_eu_checkreturn(euproto_append_instruction(s, proto, IDEFINE(index)));

				return EU_RESULT_OK;
			}

			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
				"define's name is of invalid type %s. Expected symbol or list.",
				eu_type_name(_euvalue_type(head))));
			return EU_RESULT_ERROR;

		} else if (eusymbol_equal_cstr(head, "call/cc") /* (call/cc proc) */
			|| eusymbol_equal_cstr(head, "call-with-current-continuation")) {
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"call/cc can't be used in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length != 1) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"bad call/cc arity: expected 1 arguments, got %d.", length));
				return EU_RESULT_ERROR;
			}

			/* TODO: if anything bad happens, check the code below (everything, 
			 * even after the else if */

			/* create continuation instruction */
			improper = proto->code_length;
			_eu_checkreturn(euproto_append_instruction(s, proto, ICONTI(0)));

			/* in case this isn't a tail call, add the frame instruction */
			/* the reason why the frame is created _after_ the continuation is put
			 * in the accumulator is because if we created the continuation after
			 * the creation of the frame, restoring the continuation would restore
			 * also the created frame just below the stack, which breaks everything
			 * an example of code that would break if the frame was created before
			 * creating the continuation is:
			 * ((lambda ()
			 *     (define counter 0)
			 *     (define conti #f)
			 *     (call/cc (lambda (c) (set! conti c)))
			 * 
			 *     (display counter) (newline)
			 *     (set! counter (+ counter 1))
			 * 
			 *     (if (= counter 5)
			 *         #t
			 *         (conti))))
			 */
			if (!is_tail) {
				/* since we don't know yet where to return to, use a stub address */
				index = proto->code_length; /* save instruction's index */
				_eu_checkreturn(euproto_append_instruction(s, proto, IFRAME(0)));
			}

			/* add it to the argument rib */
			_eu_checkreturn(euproto_append_instruction(s, proto, IARGUMENT()));

			/* compile the argument (which shouldn't be considered to be in tail position) */
			_eu_checkreturn(compile(s, proto, _eupair_head(_euvalue_to_pair(tail)), 0));

			/* apply */
			_eu_checkreturn(euproto_append_instruction(s, proto, IAPPLY()));

			/* correct the offset from the stub frame instruction if needed */
			if (!is_tail) {
				proto->code[index] = IFRAME(proto->code_length - index);
			}

			/* correct CONTI instruction offset */
			proto->code[improper] = ICONTI(proto->code_length - improper);

			return EU_RESULT_OK;
		} else if (eusymbol_equal_cstr(head, "begin")) { /* (begin body...) */
			/* check arity */
			length = eutil_list_length(s, tail, &improper);
			if (length < 0 || improper) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"begin can't be used in an improper list."));
				return EU_RESULT_ERROR;
			}
			if (length == 0) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"begin needs at least one expression in its body."));
				return EU_RESULT_ERROR;
			}

			/* compile expressions in order */
			for (head = tail; !_euvalue_is_null(head);
				head = _eupair_tail(_euvalue_to_pair(head))) {
				_eu_checkreturn(compile(s, proto,
					_eupair_head(_euvalue_to_pair(head)),
					/* in case this is the last expression in the form it should be
					 a tail call if begin is in the tail */
					_euvalue_is_null(_eupair_tail(_euvalue_to_pair(head)))
					? is_tail : 0));
			}

			return EU_RESULT_OK;
		} /* it's a symbol, but not a special construct. treat as any value */
	}

	/* this is a normal function application, we need to evaluate the arguments
	 * since scheme does not specify an order and APPLY expects the function to
	 * be in the accumulator and all arguments correctly placed in the rib, we
	 * first evaluate the arguments in first to last order, then evaluate the
	 * procedure, then do the application */

	/* add a frame creation instruction if this is not in tail position */
	if (!is_tail) {
		index = proto->code_length;
		_eu_checkreturn(euproto_append_instruction(s, proto, IFRAME(0)));
	}

	head = _eupair_head(_euvalue_to_pair(v)); /* procedure */

	/* starts in the second pair and goes to the end */
	for (tail = _eupair_tail(_euvalue_to_pair(v));
		!_euvalue_is_null(tail);
		tail = _eupair_tail(_euvalue_to_pair(tail))) {

		/* compiles the argument (which shouldn't be considered to be at tail position) */
		_eu_checkreturn(compile(s, proto, _eupair_head(_euvalue_to_pair(tail)), 0));
		/* add the argument instruction */
		_eu_checkreturn(euproto_append_instruction(s, proto, IARGUMENT()));
	}

	/* compile the procedure (also not in tail position) */
	_eu_checkreturn(compile(s, proto, head, 0));
	/* apply */
	_eu_checkreturn(euproto_append_instruction(s, proto, IAPPLY()));

	/* correct the frame instruction's return address offset if not in tail position */
	if (!is_tail) {
		proto->code[index] = IFRAME(proto->code_length - index);
	}

	return EU_RESULT_OK;
}

eu_result compile(europa* s, eu_proto* proto, eu_value* v, int is_tail) {
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
		_eu_checkreturn(compile_application(s, proto, v, is_tail));
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
	eu_closure* cl;
	eu_value argssym;

	/* create the top level prototype */
	top = euproto_new(s, &_null, 0, v, 0, 0);
	if (top == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* top level compile call */
	_eu_checkreturn(compile(s, top, v, 1));
	/* add return instruction to prototype */
	_eu_checkreturn(euproto_append_instruction(s, top, IRETURN()));

	/* create closure from top level prototype */
	cl = eucl_new(s, NULL, top, s->global->env);
	if (cl == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* set it to run on passed environment */
	cl->own_env = EU_FALSE;

	/* return the closure */
	_eu_makeclosure(chunk, cl);

	return EU_RESULT_OK;
}
