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

/* In order for continuations to work with C closures, the current computation
 * state for the C closure needs to be tracked. This is done thorugh 'tagging'.
 * 
 * A tag is basically a label inside the C closure which, through macros, are
 * associated to an incrementing numeric value.
 * A C closure should first declare its tags inside a dispatcher macro. This will
 * create, effectively, a sort of jump table, where the dispatcher will jump, based
 * on the 'tag' value in the continuation.
 * The closure should then have "tags" wherever 
 */

#endif