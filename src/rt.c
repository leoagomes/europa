/* beware the runtime code */
#include "eu_rt.h"

#include <setjmp.h>

#include "eu_ccont.h"
#include "eu_number.h"

struct europa_jmplist {
	struct europa_jmplist* previous;
	jmp_buf buf;
	eu_result res;
};

eu_result eurt_runcprotected(europa* s, eu_pfunc f, void* ud) {
	struct europa_jmplist jmp;

	/* jump buffer list setup */
	jmp.previous = s->error_jmp;
	jmp.res = EU_RESULT_OK;
	s->error_jmp = &jmp;

	/* protected call */
	if (setjmp(jmp.buf) == 0) {
		_eu_checkreturn((f)(s, ud));
	}

	/* restore old jump list */
	s->error_jmp = jmp.previous;

	return jmp.res;
}

eu_result eurt_evaluate(europa* s, eu_value* v, eu_value* out) {
	eu_value chunk;

	/* compile the value */
	_eu_checkreturn(eucode_compile(s, v, &chunk));

	/* run it in the vm */
	_eu_checkreturn(euvm_apply(s, &chunk, &_null, out));

	return EU_RESULT_OK;
}


eu_result euapi_register_controls(europa* s) {
	eu_table* env;

	env = s->env;

	_eu_checkreturn(eucc_define_cclosure(s, env, env, "procedure?", euapi_procedureQ));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "apply", euapi_apply));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "map", euapi_map));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "for-each", euapi_for_each));
	_eu_checkreturn(eucc_define_cclosure(s, env, env, "disassemble", euapi_disassemble));

	return EU_RESULT_OK;
}

eu_result euapi_procedureQ(europa* s) {
	eu_value* obj;

	_eucc_arity_proper(s, 1);
	_eucc_argument(s, obj, 0);

	_eu_makebool(_eucc_return(s), _euvalue_is_type(obj, EU_TYPE_CLOSURE) ||
		_euvalue_is_type(obj, EU_TYPE_CONTINUATION));
	return EU_RESULT_OK;
}

eu_result euapi_apply(europa* s) {
	eu_value *proc, *current;
	eu_value args, *slot;
	eu_pair* pair;

	_eucc_arity_improper(s, 2); /* check arity */
	_eucc_argument(s, proc, 0); /* get first argument */

	/* get cell of second argument */
	current = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));

	/* initialize the argument list */
	args = _null;
	slot = &args;

	/* iterate through arguments up until the last argument */
	while (!_euvalue_is_null(_eupair_tail(_euvalue_to_pair(current)))) {
		/* create a pair to hold current argument */
		pair = eupair_new(s, _eupair_head(_euvalue_to_pair(current)), &_null);
		if (pair == NULL)
			return EU_RESULT_BAD_ALLOC;

		/* add it to the argument list */
		_eu_makepair(slot, pair);
		slot = _eupair_tail(pair);

		/* advance to the next argument cell */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	/* make sure the last argument is a list */
	current = _eupair_head(_euvalue_to_pair(current));
	_eucc_check_type(s, current, "last argument", EU_TYPE_PAIR);

	if (!eulist_is_list(s, current)) {
		eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024,
			"last argument to map isn't a list.");
		return EU_RESULT_ERROR;
	}

	/* append it to the current argument */
	_eu_checkreturn(eulist_copy(s, current, slot));

	/* since apply can be a tail call, we just need to prepare for the call */
	return euvm_apply(s, proc, &args, _eucc_return(s));
}

eu_result euapi_map(europa* s) {
	eu_table* env;
	eu_value *proc, *current;
	eu_value args, *slot, *list, *rslot;
	eu_pair* pair;

	env = _eu_env(s);

	/* check arity */
	_eucc_arity_improper(s, 2);

	_eucc_dispatcher(s,
		/* first action is setting up environment */
		_eucc_dtag(environment_setup)

		/* at this point, the environment has been properly set up, so we can */
		rslot = cast(eu_value*, env->nodes[1].value.value.p);

		/* continue dispatcher */
		_eucc_dtag(apply_procedure)
	)

	/* the following code should be ran only once for configuring the environment */
	_eucc_tag(s, environment_setup,
		env = eutable_new(s, 3); /* create the environment */
		_eutable_set_index(env, s->env);

		if (env == NULL) return EU_RESULT_BAD_ALLOC;
		/* setup result rib value */
		_eutable_count(env) = 3;
		/* create field for returned list */
		_eu_makeint(&(env->nodes[0].key), 0);
		_eu_makenull(&(env->nodes[0].value));
		/* create field for current slot to place pair */
		_eu_makeint(&(env->nodes[1].key), 1);
		_eu_makecpointer(&(env->nodes[1].value), &(env->nodes[0].value));
		/* create field to place whether this is the last call */
		_eu_makeint(&(env->nodes[2].key), 2);
		_eu_makebool(&(env->nodes[2].value), EU_FALSE);

		s->env = env;

		/* this will return from the function and restart it at after environment_setup */
		_eucc_continue(s);
	);

	/* get the procedure argument */
	_eucc_argument(s, proc, 0);

	/* get the first list argument's node */
	current = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));

	/* initialize argument list */
	args = _null;
	slot = &args;

	/* iterate through passed lists */
	while (!_euvalue_is_null(current)) {
		list = _eupair_head(_euvalue_to_pair(current)); /* current list's head */

		/* create a pair for the current application argument */
		pair = eupair_new(s, _eupair_head(_euvalue_to_pair(list)), &_null);
		if (pair == NULL)
			return EU_RESULT_BAD_ALLOC;
		_eu_makepair(slot, pair);
		slot = _eupair_tail(pair);

		/* because the values on rib will be kept in this continuation and
		 * the argument list is ours, we can keep our state in it.
		 */
		*list = *_eupair_tail(_euvalue_to_pair(list));

		/* check whether this should be the last call to map */
		if (_euvalue_is_null(list)) {
			/* in which case set the value in the environment to signal that it is */
			_eu_makebool(&(env->nodes[2].value), EU_TRUE);
		}

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	/* apply the procedure to the arguments */
	_eucc_tag(s, apply_procedure,
		_eu_checkreturn(eucc_frame(s)); /* create a frame with current call info */
		_eu_checkreturn(euvm_apply(s, proc, &args, NULL)); /* call proc passing arguments */
	);

	/* at this point, the accumulator is the value returned from the procedure,
	 * so we create a new pair with its value in the head */
	pair = eupair_new(s, _eu_acc(s), &_null);
	if (!pair)
		return EU_RESULT_BAD_ALLOC;

	/* place the pair in the slot */
	_eu_makepair(rslot, pair);
	rslot = _eupair_tail(pair);

	/* return the resulting list's first pair if the end was reached */
	if (_eubool_is_true(&(env->nodes[2].value))) {
		/* set return to list */
		*_eucc_return(s) = env->nodes[0].value;
		return EU_RESULT_OK;
	}

	/* if this wasn't the last call to procedure, we need to do another call */
	/* update the value for rslot in env */
	env->nodes[1].value.value.p = rslot;
	/* set the current continuation's PC to the value for after the environment
	 * setup */
	s->pc = 1; /* TODO: careful when modifying this function */
	/* signal a continuation to the VM */
	return EU_RESULT_CONTINUE;
}

eu_result euapi_for_each(europa* s) {
	eu_table* env;
	eu_value *proc, *current;
	eu_value args, *slot, *list, *rslot;
	eu_pair* pair;
	int reached_end = 0;

	env = _eu_env(s);

	/* check arity */
	_eucc_arity_improper(s, 2);

	/* get the procedure argument */
	_eucc_argument(s, proc, 0);

	/* get the first list argument's node */
	current = _eupair_tail(_euvalue_to_pair(_eucc_arguments(s)));

	/* initialize argument list */
	args = _null;
	slot = &args;

	/* iterate through passed lists */
	while (!_euvalue_is_null(current)) {
		list = _eupair_head(_euvalue_to_pair(current)); /* current list's head */

		/* create a pair for the current application argument */
		pair = eupair_new(s, _eupair_head(_euvalue_to_pair(list)), &_null);
		if (pair == NULL)
			return EU_RESULT_BAD_ALLOC;
		_eu_makepair(slot, pair);
		slot = _eupair_tail(pair);

		/* because the values on rib will be kept in this continuation and
		 * the argument list is ours, we can keep our state in it.
		 */
		*list = *_eupair_tail(_euvalue_to_pair(list));

		/* check whether this should be the last call to map */
		if (_euvalue_is_null(list)) {
			reached_end = EU_TRUE;
		}

		/* go to next argument */
		current = _eupair_tail(_euvalue_to_pair(current));
	}

	/* if this isn't the last iteration, we will need to continue the for-each
	 * loop, so we need to create a frame for the current function activation
	 * record in order to tell the VM to resume this function after proc has
	 * returned.
	 * 
	 * In case this is the last iteration, meaning at least one of the passed
	 * lists has been entirely consumed, we can treat the call to proc as a
	 * tail call, where we don't need to create a new frame for the call.
	 * 
	 * The "stopping" condition for this function is actually reaching the end
	 * of at least one list, because that will not insert a frame and the VM
	 * will never call back our function. While none of the lists are over, a
	 * frame will be added, resulting on this function being called after the
	 * application of proc
	 */
	if (!reached_end) {
		_eu_checkreturn(eucc_frame(s));
	}

	/* lastly, apply the procedure. */
	return euvm_apply(s, proc, &args, NULL);
}
