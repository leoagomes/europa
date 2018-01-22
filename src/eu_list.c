#include "eu_cell.h"

#include <stdarg.h>

eu_value eulist_make_list(europa* s, int count, ...) {
	va_list args;
	int i;
	eu_value list, *cell_slot;
	eu_cell *current_cell;

	va_start(args, count);

	/* if invalid argument count return the empty list*/
	if (count <= 0)
		return EU_VALUE_NULL;

	/* set slot for list start as returning value */
	cell_slot = &list;

	/* for each argument passed */
	for (i = 0; i < count; i++) {
		/* create a new cell (that will hold the argument) */
		current_cell = eucell_new(s, va_arg(args, eu_value), EU_VALUE_NULL);

		/* put the cell in the given value slot */
		cell_slot->type = EU_TYPE_OBJECT;
		cell_slot->value.object = eu_cell2gcobj(current_cell);

		/* update value slot to be the tail of the new cell */
		cell_slot = &(eucell_tail(current_cell));
	}

	va_end(args);

	/* return the head of the list */
	return list;
}

int eulist_length(europa* s, eu_value v) {
	int i = 0;
	eu_value slow, fast;

	slow = fast = v;
	while (1) {
		if (euvalue_is_null(fast))
			return i;
		if (!euvalue_is_cell(fast))
			return -2 - i;
		fast = cdr(fast);
		i++;

		if (euvalue_is_null(fast))
			return i;
		if (!euvalue_is_cell(fast))
			return -2 - i;
		fast = cdr(fast);
		i++;

		slow = cdr(slow);
		if (fast == slow)
			return -1;
	}
}

eu_value eulist_copy(europa* s, eu_value list, eu_cell** last_cell) {
	eu_value copy, *copy_slot;
	eu_cell *original, *current_cell;

	/* if copying the empty list, return the empty list */
	if (euvalue_is_null(list))
		return EU_TYPE_NULL;

	/* if what was given wasn't a list, return a error object */
	if (!eulist_is_list(s, list))
		return euerr_tovalue(euerr_new(s,
			"expected argument #1 of type 'list?'"));

	/* get original list's cell object */
	original = eu_value2cell(list);

	/* set the head of the copy list slot to the returning value */
	copy_slot = &copy;

	while (original != NULL) {
		/* create a new cell with the same car value as the original */
		current_cell = eucell_new(s, eucell_head(original), EU_VALUE_NULL);

		/* put it in its slot */
		copy_slot->type = EU_TYPE_OBJECT;
		copy_slot->value.object = eu_cell2gcobj(current_cell);

		/* update the slot to be the cdr of the new cell */
		copy_slot = &(eucell_tail(current_cell));

		/* check if original list's next cell is null (the empty list) */
		if (euvalue_is_null(ccdr(original)))
			break;

		/* check if original list's next cell is actually a cell */
		if (!euvalue_is_cell(ccdr(original)))
			return euerr_tovalue(euerr_new(s,
				"argument to list copy is not a proper list"));

		/* update the 'original' cell to be the original list's next cell */
		original = eu_value2cell(ccdr(original));
	}

	/* current_cell has the value of the last allocated cell 
	 * and so it has the value of the last cell */
	if (last_cell != NULL)
		(*last_cell) = current_cell;

	/* return the copy of the list */
	return copy;
}

eu_cell* eulist_last_cell(europa* s, eu_value list) {
	eu_cell *ccell, *lcell;

	/* if given the empty list of something that is clearly not a list
	 * return an invalid result */
	if (euvalue_is_null(list) || !euvalue_is_cell(list))
		return NULL;

	/* given that this is a cell, set the current cell to the first cell */
	ccell = eu_value2cell(list);

	/* now there are only the possibilities that the next cell is the empty
	 * list (null) cell (making this the last cell) or that it is not. */

	/* while the next cell is not the empty list */
	while (!euvalue_is_null(ccdr(ccell))) {
		/* if in the middle of the list something that is not a cell was found
		 * then the result should be invalid, as this is not a proper list */
		if (!euvalue_is_cell(ccdr(ccell)))
			return NULL;
		
		/* set the next cell as the current cell */
		ccell = eu_value2cell(cdr(ccell));
	}

	/* return the reference to the last cell not found to be null */
	return ccell;
}

void eulist_append_list_to_list(europa* s, eu_value target, eu_value val) {
	eu_cell* last;

	/* find the next cell of a given list and if it is a valid cell, set
	 * its cdr to the value given */
	last = eulist_last_cell(s, target);
	if (last != NULL)
		eucell_tail(last) = val;
}

/* helper defines */
#define __arity_check(s,args,fname,exp) \
	if ((args) == NULL)
		return euerr_tovalue(euerr_arity_mismatch((s),(fname),(exp),0));

/* language API */
eu_value euapi_list_is_list(europa* s, eu_cell* args) {
	__arity_check(s, args, "list?");
	return euval_from_boolean(eulist_length(s, ccar(args)) >= 0);
}

eu_value euapi_list_make_list(europa* s, eu_cell* args) {
	eu_value list;

	/* if no argument was given, return empty list */
	if (args == NULL)
		return EU_VALUE_NULL;

	/* since we already received the arguments inside a list,
	 * we can just encapsulate the received list into a value
	 * and return it. */
	list.type = EU_TYPE_OBJECT;
	list.value.object = eu_cell2gcobj(args);

	return list;
}

eu_value euapi_list_length(europa* s, eu_cell* args) {
	int result;

	__arity_check(s, args, "length", 1);

	result = eulist_length(s, ccar(args));
	
	/* if the result of a length on the list is invalid, then
	 * the given argument was not a proper list */
	if (result < 0)
		return euerr_tovalue(euerr_bad_value_type(s, car(args), EU_OBJTYPE_CELL));
	else
		return euvalue_from_intM(result);
}

eu_value euapi_list_append(europa* s, eu_cell* args) {
	eu_value result, *slot, cargvalue;
	eu_cell *ccell, *cargcell, *copy_last_cell;

	/* if no argument was given, return empty list */
	if (args == NULL)
		return EU_VALUE_NULL;

	/* set current argument cell to first argument */
	cargcell = args;
	/* set append slot to be the value to encapsulate head of list */
	slot = &result;

	/* while there are arguments still*/
	while (cargcell != NULL) {
		/* if there is another argument available but no slot to put it, then
		 * the current list is not a proper list */
		if (slot == NULL)
			return euerr_tovalue(euerr_new(s, "can't append to improper list"))

		/* get the current argument */
		cargvalue = eucell_head(cargcell);

		/* check whether current argument is a proper list */
		if (eulist_is_list(s, cargvalue)) {
			/* if it is, then create a copy of the list and append it 
			 * to our result list, by filling the slot to the copy's head */
			(*slot) = eulist_copy(s, cargvalue, &copy_last_cell);

			/* and update the slot to be the new last cell's tail */
			slot = &(eucell_tail(copy_last_cell));
		} else {
			/* if the current argument is not a proper list, then we just 
			 * fill the available slot with the argument and set the slot to
			 * null, because the list is not a proper list anymore */
			slot->type = cargvalue.type;
			slot->value = cargvalue.value;
			slot = NULL;
		}

		/* if there is no next argument, break */
		if (euvalue_is_null(cdr(cargcell)))
			break;

		/* if the next argument is not a cell, then there is a bug in the
		 * caller of this function */
		if (!euvalue_is_cell(cdr(cargcell)))
			return euerr_tovalue(euerr_new("malformed argument list"));

		/* set the current arg cell to the next cell in the argument list */
		cargcell = eu_value2cell(cdr(cargcell));
	}

	/* return the result list */
	return result;
}

eu_value euapi_list_reverse(europa* s, eu_cell* args) {
	__arity_check(s, args, "reverse", 1);


}

eu_value euapi_list_list_tail(europa* s, eu_cell* args);
eu_value euapi_list_list_ref(europa* s, eu_cell* args);
eu_value euapi_list_list_set(europa* s, eu_cell* args);
eu_value euapi_list_memq(europa* s, eu_cell* args);
eu_value euapi_list_memv(europa* s, eu_cell* args);
eu_value euapi_list_member(europa* s, eu_cell* args);
eu_value euapi_list_assq(europa* s, eu_cell* args);
eu_value euapi_list_assv(europa* s, eu_cell* args);
eu_value euapi_list_assoc(europa* s, eu_cell* args);
eu_value euapi_list_list_copy(europa* s, eu_cell* args);