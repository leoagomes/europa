#include "eu_rt.h"

#include "eu_error.h"
#include "eu_symbol.h"
#include "eu_number.h"

#define OPCMASK 0xFF
#define OPCSHIFT 24
#define VALMASK 0xFFFFFF

#define opc_part(x) ((x >> OPCSHIFT) & OPCMASK)
#define val_part(x) (x & VALMASK)
#define off_part(x) (val_part(x) - (VALMASK >> 1))

/**
 * @brief Prepares a closure for applying; setting formals to their respective
 * values.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @param args The argument list value.
 * @return The result of the operation.
 */
eu_result prepare_for_apply(europa* s, eu_closure* cl, eu_value* args) {
	eu_proto* proto;
	eu_value *tv, *cv, *cf;

	/* a C closure should already be prepared */
	if (cl->cf != NULL)
		return EU_RESULT_OK;

	/* check if argument list is valid */
	if (!_euvalue_is_type(args, EU_TYPE_PAIR)) {
		_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
			"Closure application argument list isn't a list."));
		return EU_RESULT_ERROR;
	}

	proto = cl->proto;

	/* add proper elements */
	for (cv = args, cf = _euproto_formals(proto);
		!_euvalue_is_null(cf) && _euvalue_is_type(cf, EU_TYPE_PAIR);
		cv = _eupair_tail(_euvalue_to_pair(cv)),
		cf = _eupair_tail(_euvalue_to_pair(cf))) {

		/* check if there is an arity problem */
		if (_euvalue_is_null(cv)) {
			_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
				"Expected more arguments in closure application."));
			return EU_RESULT_ERROR;
		}

		/* check if argument list is still behaving properly */
		if (!_euvalue_is_type(cv, EU_TYPE_PAIR)) {
			/* if it is not, error */
			_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
				"Closure application arguments aren't a proper list."));
			return EU_RESULT_ERROR;
		}

		/* get environment slot */
		_eu_checkreturn(eutable_get(s, cl->env, _eupair_head(_euvalue_to_pair(cf)),
			&tv));
		if (tv == NULL)
			goto bad_closure_environment;

		/* place the value */
		*tv = *(_eupair_head(_euvalue_to_pair(cv)));
	}

	/* if cv isn't the null value, formals was either an improper list or a
	 * symbol that should take the argument list. either way, the symbol at 
	 * cf is the correct parameter symbol in the environment and cv is the
	 * correct value for it */
	if (!_euvalue_is_null(cf)) {
		/* get the slot in the environment */
		_eu_checkreturn(eutable_get(s, cl->env, cf, &tv));
		/* check for errors */
		if (tv == NULL)
			goto bad_closure_environment;

		/* place the argument list there */
		*tv = *args;
		return EU_RESULT_OK;
	}

	/* the closure's environment is properly set */
	return EU_RESULT_OK;

	bad_closure_environment:
	/* closure wasn't initialized properly */
	_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
		"Closure was not initialized properly when constructed."));
	return EU_RESULT_ERROR;
}

eu_result prepare_state(europa* s, eu_closure* cl) {

	/* set initial state */

	return EU_RESULT_OK;
}

eu_result check_val_in_constant(europa* s, int val, const char* inst) {
	if (s->ccl->proto->constantc <= val) {
		_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid constant index at %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result check_val_in_subprotos(europa* s, int val, const char* inst) {
	if (s->ccl->proto->subprotoc <= val) {
		_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid subproto index at %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result check_off_in_code(europa* s, int off, const char* inst) {
	if (s->ccl->proto->code + s->ccl->proto->code_length < s->pc + off ||
		s->ccl->proto->code > s->pc + off) {
		_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid jumping offset for %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result add_rib_value(europa* s, eu_value* v) {

}

/**
 * @brief Starts a vm execution loop.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @return The result of the operation.
 */
eu_result euvm_execute(europa* s, eu_closure* cl) {
	eu_result res;
	eu_instruction ir;
	eu_value* tv;
	eu_proto* proto, *p;
	eu_closure *c;
	eu_continuation* cont;
	eu_pair* pair;

	/* if c closure, increase tag */
	if (cl->cf) {
		s->tag += 1;
		cl->status = EU_CLSTATUS_RUNNING;
		res = cl->cf(s);
		cl->status = EU_CLSTATUS_FINISHED;
		s->tag -= 1;
		return EU_RESULT_OK;
	}

	/* europa closure, start vm loop */
	ir = *s->pc;
	while (opc_part(ir) != EU_OP_HALT) {
		proto = s->ccl->proto;
		cl = s->ccl;

		switch (opc_part(ir)) {
		case EU_OP_REFER:
			/* check if instruction value is in constant range */
			_eu_checkreturn(check_val_in_constant(s, val_part(ir), "REFER"));
			/* get the env's value for the symbol constant key */
			_eu_checkreturn(eutable_rget(s, cl->env, &(proto->constants[val_part(ir)]),
				&tv));
			/* check if could get reference */
			if (tv == NULL) {
				_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"Could not reference %s in environment.",
					_eusymbol_text(_euvalue_to_symbol(&(proto->constants[val_part(ir)])))));
				return EU_RESULT_ERROR;
			}
			/* put the value in the accumulator */
			s->acc = *tv;
			break;
		case EU_OP_CONST:
			/* check if instruction value is in constant range */
			_eu_checkreturn(check_val_in_constant(s, val_part(ir), "CONST"));
			/* move it to the accumulator */
			s->acc = proto->constants[val_part(ir)];
			break;
		case EU_OP_CLOSE:
			/* check if value is in subproto range */
			_eu_checkreturn(check_val_in_subprotos(s, val_part(ir), "CLOSE"));
			/* get the subproto */
			p = proto->subprotos[val_part(ir)];
			/* create a new closure from it and current environment */
			c = eucl_new(s, NULL, p, s->env);
			if (c == NULL) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create closure."));
				return EU_RESULT_BAD_ALLOC;
			}
			/* place it in the accumulator */
			_eu_makeclosure(_eu_acc(s), c);
			break;
		case EU_OP_TEST:
			/* check whether offset is in code range */
			_eu_checkreturn(check_off_in_code(s, off_part(ir), "TEST"));
			/* check accumulator */
			if (!_euvalue_is_type(_eu_acc(s), EU_TYPE_BOOLEAN) ||
				!_euvalue_to_bool(_eu_acc(s))) {
				/* if the acc isn't the true object, jump to offset */
				s->pc += off_part(ir);
				/* go to instruction fetch */
				goto fetch_next;
			}
			/* in case the accumulator is #t, just continue */
			break;
		case EU_OP_ASSIGN:
			/* check whether value is in constant range */
			_eu_checkreturn(check_val_in_constant(s, val_part(ir), "ASSIGN"));
			/* get the env's value for the symbol constant key */
			_eu_checkreturn(eutable_rget(s, cl->env, &(proto->constants[val_part(ir)]),
				&tv));
			/* check if could get reference */
			if (tv == NULL) {
				_eu_checkreturn(europa_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"Could not set %s in environment.",
					_eusymbol_text(_euvalue_to_symbol(&(proto->constants[val_part(ir)])))));
				return EU_RESULT_ERROR;
			}
			/* set the value slot to the value in the accumulator */
			*tv = s->acc;
			break;
		case EU_OP_CONTI:
			/* create a continuation from the current state */
			cont = eucont_new(s, s->tag, s->previous, s->env, &s->rib, s->ccl, s->pc);
			if (cont == NULL) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create continuation."));
				return EU_RESULT_ERROR;
			}
			/* place it in the accumulator */
			_eu_makeclosure(_eu_acc(s), cont);

			/* check whether to also add it to the argument list */
			if (val_part(ir)) {
				pair = eupair_new(s, _eu_acc(s), &_null);
				if (pair == NULL) {
					_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
						"Could not create cell to hold argument value."));
					return EU_RESULT_ERROR;
				}

				/* add value to rib */
				_eu_makepair(s->rib_lastpos, pair);
				/* update last position */
				s->rib_lastpos = _eupair_tail(pair);
			}
			break;
		case EU_OP_FRAME:
			/* check if return offset is valid */
			_eu_checkreturn(check_off_in_code(s, off_part(ir), "FRAME"));
			/* create a new continuation */
			cont = eucont_new(s, s->tag, s->previous, s->env, &s->rib, s->ccl,
				s->pc + off_part(ir));
			if (cont == NULL) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create continuation."));
				return EU_RESULT_ERROR;
			}
			/* link previous to this if applicable */
			if (s->previous) {
				s->previous->next = cont;
			}

			/* change current state values */
			s->previous = cont;
			s->rib = _null;
			s->rib_lastpos = &s->rib;
			break;
		case EU_OP_ARGUMENT:
			/* add the accumulator to the rib */
			pair = eupair_new(s, _eu_acc(s), &_null);
			if (pair == NULL) {
				_eu_checkreturn(europa_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create pair to hold argument."));
				return EU_RESULT_ERROR;
			}
			/* add it to the rib */
			_eu_makepair(s->rib_lastpos, pair);
			/* update last pos */
			s->rib_lastpos = _eupair_tail(pair);
			break;
		case EU_OP_APPLY: /* handle calling a value (can be closure, continuation or other object (like a table)) */
			break;
		default:
			break;
		}

		/* advance to next instruction */
		(s->pc)++;

		fetch_next: /* fetch next instruction */
		ir = *(s->pc);
	}

	return EU_RESULT_OK;
}

/**
 * @brief Initializes necessary fields in an Europa state.
 * 
 * @param s The target state.
 * @return Whether the operation was successful.
 */
eu_result euvm_initialize_state(europa* s) {
	s->acc = _null;
	s->env = s->global->env;
	s->ccl = NULL;
	s->previous = NULL;
	s->rib = _null;
	s->rib_lastpos = &s->rib;
	s->tag = 0;
}

/**
 * @brief Executes a closure.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @param args The target arguments.
 * @return The result of the operation.
 */
eu_result euvm_doclosure(europa* s, eu_closure* cl, eu_value* args) {
	/* prepare given arguments */
	_eu_checkreturn(prepare_for_apply(s, cl, args));

	/* initially set up the state */

	/* do the execution */
	_eu_checkreturn(euvm_execute(s, cl));
	return EU_RESULT_OK;
}
