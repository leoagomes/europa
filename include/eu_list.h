#ifndef __EUROPA_LIST_H__
#define __EUROPA_LIST_H__

#include "eu_cell.h"

/* internal api / helper functions */
#define eulist_is_list(s,v) (eulist_length((s), (v)) >= 0)

/* makes a list with eu_value arguments, returning it as an eu_value */
eu_value eulist_make_list(europa* s, ...);
int eulist_length(europa* s, eu_value v);
eu_value eulist_copy(europa* s, eu_value list, eu_cell** last_cell);
eu_cell* eulist_last_cell(europa* s, eu_value list);
void eulist_append_list_to_list(europa* s, eu_value target, eu_value val);
eu_value eulist_reverse(europa* s, eu_value original);
eu_value eulist_tail(europa* s, eu_value list, int k);
eu_value eulist_ref(europa* s, eu_value list, int k);

/* list procedures from the api (definition at eu_list.c) */
eu_value euapi_list_is_list(europa* s, eu_cell* args);
eu_value euapi_list_make_list(europa* s, eu_cell* args);
eu_value euapi_list_length(europa* s, eu_cell* args);
eu_value euapi_list_append(europa* s, eu_cell* args);
eu_value euapi_list_reverse(europa* s, eu_cell* args);
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

#endif