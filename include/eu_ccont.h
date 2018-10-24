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
		int __eudi = 0;\
		dcl\
	} while (0)

#define _eucc_dtag(tagname) if (__eudi++ == (s)->pc) { goto tagname; }




#endif