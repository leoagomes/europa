#include "eu_frame.h"

eu_result euframe_mark(europa* s, eu_gcmark mark, eu_frame* fr) {
	_eu_checkreturn(mark(s, _eutable_to_obj(fr->env)));
	_eu_checkreturn(mark(s, _eupair_to_obj(fr->rib)));
	if (_euvalue_is_collectable(&(fr->next))) {
		_eu_checkreturn(mark(s, _euvalue_to_obj(&(fr->next))));
	}
	return EU_RESULT_OK;
}

eu_result euframe_destroy(europa* s, eu_frame* cl) {
	return EU_RESULT_OK;
}

eu_integer euframe_hash(eu_frame* fr) {
	return cast(eu_integer, fr);
}
