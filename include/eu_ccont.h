/** Helper macros and functions for dealing with continuations in C.
 * @file eu_ccont.h
 * @author Leonardo G.
 * 
 * This file defines functions and macros that help handling continuations and
 * its states when writing C closures for Europa that may have their continuations
 * captured.
 * 
 * The main idea behind continuation handling in C is 'tagging' C closures in a
 * way that allows resuming them in appropriate locations and having all needed
 * C closure state be kept in the continuation's environment instead of the C
 * stack.
 * 
 * If a C closure's state is kept inside the continuation's environment and
 * positions _after_ call/cc are safely tagged, then the C stack won't be a problem
 * to the interpreter.
 */
#ifndef __EUROPA_C_CONT_H__
#define __EUROPA_C_CONT_H__

#include "europa.h"
#include "eu_int.h"
#include "eu_rt.h"
#include "eu_commons.h"
#include "eu_object.h"
#include "eu_error.h"

/* 
 * I decided to try fixing the continuation problem by "flattening" the C stack.
 * 
 * Every time a C Closure (functions exposed to Europa code, but that are actually
 * written in C) wants to call back into Europa code, it actually returns a 
 * "CONTINUE" signal to the top level VM call, which properly does the call.
 * 
 * In order to continue a C Closure's execution past a function call -- or code
 * that could possibly capture the current continuation, for that matter -- it
 * needs to "tag" its body.
 * A "tag" is simply a label (as in goto) associated to a specific value for the
 * state's program counter. In the beginning of a C Closure function's body there
 * needs to be a dispatcher macro use with tag declarations. The code this macro
 * generates is basically a "if (s->pc == 0) goto tag_1;",
 * "if (s->pc == 1) goto tag_2;", etc.
 * Then, the tagging macros should be used whenever the C closure wants to call
 * into Europa code. This macro basically increments the current state's PC (s->pc++)
 * and creates a label with the tag's name.
 * 
 * Because of this possibility that the function will return "in the middle of its
 * execution", we can't guarantee that values in the C stack will be the same
 * before and after a tag. This means C Closures should transfer all data to the
 * state's environemnt prior to doing a function call that might fall in the cases
 * described above (where it would have to return "CONTINUE") and load its state
 * from the environment after every tag.
 * 
 * If you think that the impact of constantly doing hashtable referencing operations
 * is too big, you can make use of the fact that Europa never touches a C Closure's
 * environment, by essentially using the eu_table* to store some void* user data
 * of yours (that absolutely must be managed by the GC).
 * 
 * This means that C closures can't keep their state in the C stack, having to
 * store all state inside their 'env'. This is probably inefficient, but a solution
 * nonetheless.
 * 
 */

#define _eucc_dispatcher(s, dcl) do { \
		int __eudi = 1;\
		if (s->pc == 0) goto _euccdis_start;\
		dcl\
		return EU_RESULT_OK;\
	} while (0);\
	_euccdis_start:

#define _eucc_dtag(tagname) if (__eudi++ == (s)->pc) { goto tagname; }

#define _eucc_continue(s) return EU_RESULT_CONTINUE

#define _eucc_tag(s, tn, what) \
	((s)->pc)++;\
	what \
	tn: \

#define _eucc_setup_env(what) \
	do {\
		what\
	} while (0);


#define _eucc_argument(s, v, index) \
	do {\
		(v) = eulist_ref(s, _euvalue_to_pair(&((s)->rib)), (index));\
		if ((v) == NULL) {\
			eu_set_error_nf(s, EU_ERROR_NONE, (s)->err, 1024, \
				"could not get argument #%d into C local %s.", (index), #v);\
			return EU_RESULT_ERROR;\
		}\
	} while (0)

#define _eucc_argument_improper(s, v, index) \
	do {\
		(v) = eulist_ref(s, _euvalue_to_pair(&((s)->rib)), (index));\
	} while (0)

#define _eucc_argument_type(s, v, index, type) \
	do {\
		(v) = eulist_ref(s, _euvalue_to_pair(&((s)->rib)), (index));\
		if ((v) == NULL) {\
			eu_set_error_nf(s, EU_ERROR_NONE, (s)->err, 1024, \
				"could not get argument #%d into C local %s.", (index), #v);\
			return EU_RESULT_ERROR;\
		}\
		if (!_euvalue_is_type((v), (type))) {\
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024, \
				"Argument #%d of wrong type. Expected %s, got %s.", (index), \
				eu_type_name((type)), eu_type_name(_euvalue_type(v))));\
			return EU_RESULT_ERROR;\
		}\
	} while (0)

#define _eucc_valid_rib(s) \
	if (!_euvalue_is_pair(&(s)->rib)) {\
		_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL, \
			"Argument rib is not a list. Did you provide any arguments?"));\
		return EU_RESULT_ERROR;\
	}

#define _eucc_arity_proper(s, count) \
	do {\
		int __got;\
		if ((count) == 0 && !_euvalue_is_null(&((s)->rib))) {\
			_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL, \
				"No arguments expected for procedure, but some were provided."));\
			return EU_RESULT_ERROR;\
		}\
		if ((count) == 0 && !_euvalue_is_null(&((s)->rib))) {\
			_eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL, \
				"Procedure expected no arguments, but some were provided."));\
		}\
		_eucc_valid_rib(s)\
		if ((__got = eulist_length(s, _euvalue_to_pair(&(s)->rib))) != (count)) {\
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024, \
				"Bad arity: expected %d arguments, got %d.", (count), __got));\
			return EU_RESULT_ERROR;\
		}\
	} while (0)

#define _eucc_arity_improper(s, minimum) \
	do {\
		int __got;\
		_eucc_valid_rib(s)\
		if ((__got = eulist_length(s, _euvalue_to_pair(&(s)->rib))) < (minimum)) {\
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024, \
				"Bad arity: expected at least %d arguments, got %d.", (minimum),\
				__got));\
			return EU_RESULT_ERROR;\
		}\
	} while (0)

#define _eucc_arguments(s) (&((s)->rib))

#define _eucc_return(s) (_eu_acc(s))

#define _eucc_check_type(s, v, what, expected) \
	do {\
		if (!_euvalue_is_type(v, expected)) {\
			_eu_checkreturn(eu_set_error_nf(s, EU_ERROR_NONE, NULL, 1024, \
				"Bad type for " what ". Expected %s, got %s.", \
				eu_type_name(expected), eu_type_name(_euvalue_type(v))));\
			return EU_RESULT_ERROR;\
		}\
	} while (0)


eu_result eucc_frame(europa* s);
eu_result eucc_define_cclosure(europa* s, eu_table* t, eu_table* env, void* text, eu_cfunc cf);



#endif