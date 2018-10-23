#include "eu_rt.h"

#include "eu_error.h"
#include "eu_symbol.h"
#include "eu_number.h"
#include "eu_util.h"

#define OPCMASK 0xFF
#define OPCSHIFT 24
#define VALMASK 0xFFFFFF

#define opc_part(x) ((x >> OPCSHIFT) & OPCMASK)
#define val_part(x) (x & VALMASK)
#define off_part(x) (val_part(x) - (VALMASK >> 1))

#define CALL_META_NAME "@@call"
#define ARGS_KEY_NAME "@@args"

/**
 * @brief Prepares the state's environment for running a closure, by extending
 * the environment in which it was defined and setting the state's environment to
 * it.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @param args The argument list value.
 * @return The result of the operation.
 */
eu_result prepare_environment(europa* s, eu_closure* cl, eu_value* args) {
	eu_proto* proto;
	eu_value *tv, *cv, *cf;
	eu_table* new_env;
	eu_value key;
	int length, improper, i;

	/* beacause C functions don't need to use the argument rib, we can leave
	 * C closure arguments in the rib when calling them, preventing one table
	 * creation */
	if (cl->cf) {
		/* place their creation environment in env */
		s->env = cl->env;
		/* place the arguments in rib */
		s->rib = *args;
		s->rib_lastpos = NULL;
		return EU_RESULT_OK;
	}

	/* europa closure */
	/* count number of parameters in environment */
	length = eutil_list_length(s, _euproto_formals(cl->proto), &improper);
	new_env = eutable_new(s, length + improper);
	if (new_env == NULL) {
		_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
			"Could not create new environment."));
		return EU_RESULT_ERROR;
	}
	/* set its index to point to the closure's environment */
	_eutable_set_index(new_env, cl->env);

	/* add proper elements */
	for (i = 0, cv = args, cf = _euproto_formals(cl->proto);
		!_euvalue_is_null(cf) && _euvalue_is_type(cf, EU_TYPE_PAIR);
		i++,
		cv = _eupair_tail(_euvalue_to_pair(cv)),
		cf = _eupair_tail(_euvalue_to_pair(cf))) {

		/* check if there is an arity problem */
		if (_euvalue_is_null(cv)) {
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
				"Expected %s%d arguments in closure application, got %d.",
				improper ? ">" : "", length, i));
			return EU_RESULT_ERROR;
		}

		/* check if argument list is still behaving properly */
		if (!_euvalue_is_type(cv, EU_TYPE_PAIR)) {
			/* if it is not, error */
			_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
				"Closure application arguments aren't a proper list."));
			return EU_RESULT_ERROR;
		}

		/* create formal key */
		_eu_checkreturn(eutable_create_key(s, new_env,
			_eupair_head(_euvalue_to_pair(cf)), &tv));
		if (tv == NULL) {
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
				"Could not add formal %s to the new environment.",
				_eusymbol_text(_euvalue_to_symbol(_eupair_head(_euvalue_to_pair(cf))))));
			return EU_RESULT_ERROR;
		}
		/* place the value */
		*tv = *(_eupair_head(_euvalue_to_pair(cv)));
	}

	/* if cv isn't the null value, formals was either an improper list or a
	 * symbol that should take the argument list. either way, the symbol at 
	 * cf is the correct parameter symbol in the environment and cv is the
	 * correct value for it */
	if (!_euvalue_is_null(cf)) {
		/* get the slot in the environment */
		_eu_checkreturn(eutable_create_key(s, new_env, cf, &tv));
		if (tv == NULL) {
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
				"Could not add formal %s to the new environment.",
				_eusymbol_text(_euvalue_to_symbol(cf))));
			return EU_RESULT_ERROR;
		}

		/* place the argument list there */
		*tv = *cv;
	}

	/* set the new environment, potentially losing the previous one */
	s->env = new_env;

	return EU_RESULT_OK;
}

eu_result prepare_state(europa* s, eu_closure* cl) {

	/* set initial state */

	return EU_RESULT_OK;
}

eu_result check_val_in_constant(europa* s, int val, const char* inst) {
	if (s->ccl->proto->constantc <= val) {
		_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid constant index at %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result check_val_in_subprotos(europa* s, int val, const char* inst) {
	if (s->ccl->proto->subprotoc <= val) {
		_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid subproto index at %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result check_off_in_code(europa* s, int off, const char* inst) {
	if (s->pc + off > s->ccl->proto->code_length) {
		_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"Invalid jumping offset for %s instruction.", inst));
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

void set_cc(europa* s, eu_continuation* cont) {
	if (cont == NULL) { /* nothing else to run */
		s->ccl = NULL;
		s->env = NULL;
		s->pc = 0;
		s->status = EU_SSTATUS_STOPPED;
		return;
	}

	s->ccl = cont->cl;
	s->previous = cont->previous;
	s->pc = cont->pc;
	s->env = cont->env;
	s->rib = cont->rib;
	s->rib_lastpos = cont->rib_lastpos;
}

/* WARNING: DOES NOT PREPARE THE ENVIRONMENT, USE prepare_environment FOR THAT */
void set_closure(europa* s, eu_closure* cl) {
	s->ccl = cl; /* set current closure */

	/* only clean up the rib if closure is not C closure, because arguments of a
	 * c closure are passed through it */
	if (cl->cf == NULL) {
		s->rib = _null;
		s->rib_lastpos = &(s->rib);
	}

	s->pc = 0;
}

/**
 * @brief Starts a vm execution loop.
 * 
 * This will make the vm continue executing from its current state. In order to
 * start running code, whatever it is that calls this function should've
 * properly set the state already.
 * 
 * @param s The Europa state.
 * @return The result of the operation.
 */
eu_result euvm_execute(europa* s) {
	eu_result res;
	eu_instruction ir;
	eu_value* tv;
	eu_proto* proto, *p;
	eu_closure *c;
	eu_continuation* cont;
	eu_pair* pair;
	eu_closure* cl;

	/* we need to do the execution loop
	 *
	 * because it makes no sense to try to run no code (s->ccl == NULL) or having
	 * no environment (s->env == NULL), we use that as a condition for stopping
	 * the fetch/decode/execute loop.
	 * 
	 * this stopping condition effectvely makes it so that whenever running code
	 * returns, but s->previous is NULL (meaning the top-level call returned),
	 * we stop runnning the F/D/E loop and return OK, leaving the result of the
	 * computation in the state's accumulator.
	 */
	while (s->ccl != NULL && s->env != NULL) {
		/* shorten some names */
		cl = s->ccl;

		/* the current thing to execute might be Europa code or C code */
		if (cl->cf) { /* we need to run a C procedure */
			/* call it, saving return value */
			res = (cl->cf)(s);

			/* The return value of C closure will be either:
			 * * OK - in which case the function properly returned and we need
			 *        to "discard" its frame
			 * * SOME ERROR VALUE - in which case an error occured and we should
			 *                      return it.
			 * * CONTINUE - in which case the closure decided to call back into
			 *              other code that is managed by the VM.
			 */
			if (res == EU_RESULT_OK) {
				/* the closure ran properly, return to previous frame */
				set_cc(s, s->previous);
				continue; /* restart the loop with the previous frame as state */

			} else if (res == EU_RESULT_CONTINUE) {
				/* the C closure called Europa code (or another C closure)
				 * This means that the current state is already set up to
				 * continue the computation, be it the continuation that was
				 * called or a closure. What we need to do, then, is just
				 * continue on with the loop.
				 */
				continue;
			} else {
				/* an error occured, we should return it */
				return res;
			}
		}

		/* at this point we must be trying to run some scheme code */

		/* first fetch the next instruction */
		vmfetch:
		if (s->pc > c->proto->code_length) { /* check if PC is in bounds */
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
				"Tried running code after code buffer."
				"(PC %d is inconsistent; code length is %d.)",
				s->pc, c->proto->code_length));
			return EU_RESULT_ERROR;
		}
		ir = c->proto->code[s->pc]; /* fetch instruction into IR */

		/* shorten some more names */
		proto = c->proto;

		switch (opc_part(ir)) {
		case EU_OP_NOP: break;
		case EU_OP_REFER:
			/* check if instruction value is in constant range */
			_eu_checkreturn(check_val_in_constant(s, val_part(ir), "REFER"));
			/* get the env's value for the symbol constant key */
			_eu_checkreturn(eutable_rget(s, cl->env,
				&(proto->constants[val_part(ir)]), &tv));
			/* check if could get reference */
			if (tv == NULL) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
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
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
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
				goto vmfetch;
			}
			/* in case the accumulator is #t, just continue */
			break;

		case EU_OP_JUMP:
			/* check whether offset is in boundaries */
			_eu_checkreturn(check_off_in_code(s, off_part(ir), "JUMP"));
			s->pc += off_part(ir);
			goto vmfetch;

		case EU_OP_ASSIGN:
			/* check whether value is in constant range */
			_eu_checkreturn(check_val_in_constant(s, val_part(ir), "ASSIGN"));
			/* get the env's value for the symbol constant key */
			_eu_checkreturn(eutable_rget(s, cl->env, &(proto->constants[val_part(ir)]),
				&tv));
			/* check if could get reference */
			if (tv == NULL) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"Could not set %s in environment.",
					_eusymbol_text(_euvalue_to_symbol(&(proto->constants[val_part(ir)])))));
				return EU_RESULT_ERROR;
			}
			/* set the value slot to the value in the accumulator */
			*tv = s->acc;
			break;

		case EU_OP_CONTI:
			/* create a continuation from the current state */
			cont = eucont_new(s, s->previous, s->env, &s->rib, s->rib_lastpos,
				s->ccl, s->pc);
			if (cont == NULL) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create continuation."));
				return EU_RESULT_ERROR;
			}
			/* place it in the accumulator */
			_eu_makeclosure(_eu_acc(s), cont);

			/* check whether to also add it to the argument list */
			if (val_part(ir)) {
				pair = eupair_new(s, _eu_acc(s), &_null);
				if (pair == NULL) {
					_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
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
			cont = eucont_new(s, s->previous, s->env, &s->rib, s->rib_lastpos,
				s->ccl, s->pc + off_part(ir));
			if (cont == NULL) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create continuation."));
				return EU_RESULT_ERROR;
			}
			/* link previous to this if applicable */

			/* change current state values */
			s->previous = cont;
			s->rib = _null;
			s->rib_lastpos = &s->rib;
			break;

		case EU_OP_ARGUMENT:
			/* add the accumulator to the rib */
			pair = eupair_new(s, _eu_acc(s), &_null);
			if (pair == NULL) {
				_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
					"Could not create pair to hold argument."));
				return EU_RESULT_ERROR;
			}
			/* add it to the rib */
			_eu_makepair(s->rib_lastpos, pair);
			/* update last pos */
			s->rib_lastpos = _eupair_tail(pair);
			break;

		case EU_OP_APPLY: /* handle calling a value (can be closure, continuation or a table) */
			/* TODO: add support for type indexes */

			/* fail if type isn't table, closure or continuation, for now */
			if (!_euvalue_is_type(_eu_acc(s), EU_TYPE_TABLE) &&
				!_euvalue_is_type(_eu_acc(s), EU_TYPE_CLOSURE) &&
				!_euvalue_is_type(_eu_acc(s), EU_TYPE_CONTINUATION)) {
				_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
					"Tried applying/calling something of invalid type %s.",
					eu_type_name(_euvalue_type(_eu_acc(s)))));
				return EU_RESULT_OK;
			}

			/* if the target value is a table, we need to check it for a '@@call'
			 * function */
			if (_euvalue_is_type(_eu_acc(s), EU_TYPE_TABLE)) {
				_eu_checkreturn(eutable_rget_symbol(s, _euvalue_to_table(_eu_acc(s)),
					CALL_META_NAME, &tv));
				/* check for correct type */
				if (tv == NULL || !_euvalue_is_type(tv, EU_TYPE_CLOSURE)) {
					_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
						"Could not call table. Invalid value for " CALL_META_NAME "."));
					return EU_RESULT_ERROR;
				}
				/* we have a proper closure at tv, prepend the table in acc to the
				 * rib */
				pair = eupair_new(s, _eu_acc(s), &s->rib);
				if (pair == NULL) {
					_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
						"Could not add called table to argument rib."));
					return EU_RESULT_ERROR;
				}
				_eu_makepair(&(s->rib), pair);
				/* finally, place what's in tv in the accumulator and continue
				 * with the APPLY instruction */
				s->acc = *tv;
			}

			/* at this point, acc is either a closure or a continuation */
			if (_euvalue_is_type(_eu_acc(s), EU_TYPE_CLOSURE)) {
				/* it is a closure, so run one */
				c = _euvalue_to_closure(_eu_acc(s));

				/* prepare the state's environment */
				_eu_checkreturn(prepare_environment(s, c, &(s->rib)));
				/* set current closure */
				set_closure(s, c);

				/* the current state is set up, so we can continue on with the
				 * F/D/E loop */
				continue;

			} else { /* it is a continuation */
				cont = _euvalue_to_cont(_eu_acc(s));

				/* we need to set the accumulator to the first argument */
				if (!_euvalue_is_pair(&(s->rib))) {
					s->acc = s->rib;
				} else {
					s->acc = *_eupair_head(_euvalue_to_pair(_eu_acc(s)));
				}

				/* place the continuation in the state */
				set_cc(s, cont);
				continue; /* start the loop again */
			}
			break;

		case EU_OP_RETURN:
			/* set the current frame to the previous, leaving acc untouched */
			set_cc(s, s->previous);
			continue;

		case EU_OP_HALT:
			/* make the current frame invalid, leaving acc untouched */
			set_cc(s, NULL);
			continue;

		default:
			break;
		}

		/* advance to next instruction */
		s->pc++;
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
	s->level = 0;

	return EU_RESULT_OK;
}

/**
 * @brief Executes a closure.
 * 
 * @param s The Europa state.
 * @param cl The target closure.
 * @param args The target arguments.
 * @param out Where to place the returned value.
 * @return The result of the operation.
 */
eu_result euvm_doclosure(europa* s, eu_closure* cl, eu_value* args, eu_value* out) {
	/* place the closure in the current continuation */
	_eu_checkreturn(prepare_environment(s, cl, args));
	set_closure(s, cl);

	/* do the execution */
	_eu_checkreturn(euvm_execute(s));

	if (out) /* return the value, if asked */
		*out = s->acc;

	return EU_RESULT_OK;
}
