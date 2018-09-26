/** Memory Port implementation.
 * 
 * @file mport.c
 * @author Leonardo G.
 */
#include "ports/eu_mport.h"
#include "eu_number.h"
#include "utf8.h"

#include <string.h>

eu_mport* eumport_from_str(europa* s, eu_byte flags, void* str) {
	eu_mport* port;
	size_t size;

	if (!s || !str)
		return NULL;

	size = utf8size(str);
	port = _euobj_to_mport(eugc_new_object(s, EU_TYPEFLAG_COLLECTABLE |
		EU_TYPE_PORT, sizeof(eu_mport)));
	if (port == NULL)
		return NULL;

	port->type = EU_PORT_TYPE_MEMORY;
	port->flags = flags;
	port->size = size;
	port->mem = (eu_byte*)eugc_malloc(_eu_gc(s), size);
	if (port->mem == NULL)
		return NULL;

	port->next = port->mem;
	port->rpos = port->wpos = 0;

	memcpy(port->mem, str, size);

	return port;
}

eu_result eumport_mark(europa* s, eu_gcmark mark, eu_mport* port) {
	return EU_RESULT_OK;
}

eu_result eumport_destroy(europa* s, eu_mport* port) {
	if (!s || !port)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->mem)
		eugc_free(_eu_gc(s), port->mem);

	return EU_RESULT_OK;
}

eu_uinteger eumport_hash(eu_mport* port) {
	return cast(eu_integer, port);
}

/* internal use functions */
/* input */

eu_result eumport_read_char(europa* s, eu_mport* port, int* out) {
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->next == NULL || port->rpos >= port->size || *(port->next) == '\0') {
		*out = EOF;
		return EU_RESULT_OK;
	}

	port->next = utf8codepoint(port->next, out);
	port->rpos += utf8codepointsize(*out);
	return EU_RESULT_OK;
}

eu_result eumport_peek_char(europa* s, eu_mport* port, int* out) {
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->next == NULL || port->rpos >= port->size || *(port->next) == '\0') {
		*out = EOF;
		return EU_RESULT_OK;
	}

	utf8codepoint(port->next, out);
	return EU_RESULT_OK;
}

eu_result eumport_read_line(europa* s, eu_mport* port, eu_value* out) {
	int cp = 0, peek = 0;
	void* next = NULL;
	void* pnext = NULL;
	size_t linesize = 0;
	unsigned int nulpos = 0;
	eu_string* str;
	eu_result res;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->next == NULL || port->rpos >= port->size) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	next = port->next;
	utf8codepoint(next, &cp);
	while (next && cp) {
		/* incrementing line size here makes sure we have space for the nul byte*/
		linesize += utf8codepointsize(cp);
		if (cp == '\r') {
			if (next) {
				pnext = utf8codepoint(next, &peek);
				if (peek == '\n') {
					next = pnext;
					cp = peek;
				}
			}
			break;
		}
		else if (cp == '\n') {
			break;
		}
		next = utf8codepoint(next, &cp); /* grab next character */
	}

	if (next == NULL || cp == 0) { /* data terminated before end of line */
		nulpos = linesize;
		linesize += 1; /* add the space for the nul byte */
	}

	str = eustring_withsize(s, linesize);
	if (str == NULL)
		return EU_RESULT_BAD_ALLOC;

	memcpy(_eustring_text(str), port->next, linesize);
	cast(char*,&(str->_text))[linesize - 1] = '\0';

	if ((res = eustring_rehash(str)))
		return res;

	out->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eustring_to_obj(str);

	return EU_RESULT_OK;
}

eu_result eumport_char_ready(europa* s, eu_mport* port, int* ready) {
	if (!s || !port || !ready)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->next || port->rpos >= port->size)
		*ready = EU_FALSE;
	else
		*ready = EU_TRUE;
	return EU_RESULT_OK;
}

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

eu_result eumport_read_string(europa* s, eu_mport* port, int k, eu_value* out) {
	eu_byte* next;
	eu_string* str;
	eu_result res;
	size_t size;
	int i, cp;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (k < 0)
		return EU_RESULT_BAD_ARGUMENT;

	next = cast(eu_byte*, port->next);
	if (next == NULL || port->rpos >= port->size) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	size = 0;
	cp = 1;
	for (i = 0; i < k && next != NULL && cp != 0; i++) {
		next = utf8codepoint(next, &cp);
		size += utf8codepointsize(cp);
	}

	if (next == NULL || i == k) { /* last read character wasnt the end */
		size += 1; /* count the final nul byte */
	}

	str = eustring_withsize(s, size);
	if (str == NULL)
		return EU_RESULT_BAD_ALLOC;

	memcpy(_eustring_text(str), port->next, size - 1);
	cast(char*, _eustring_text(str))[size - 1] = '\0';

	if ((res = eustring_rehash(str)))
		return res;

	out->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eustring_to_obj(str);

	return EU_RESULT_OK;
}

eu_result eumport_read_u8(europa* s, eu_mport* port, eu_value* out) {
	eu_byte* next;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->next == NULL || port->rpos >= port->size) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	next = cast(eu_byte*, port->next);
	_eu_makeint(out, (eu_integer)*next++);
	port->next = next;
	port->rpos++;

	return EU_RESULT_OK;
}

eu_result eumport_peek_u8(europa* s, eu_mport* port, eu_value* out) {
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (port->next == NULL || port->rpos >= port->size) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	_eu_makeint(out, (eu_integer)*(cast(eu_byte*, port->next)));
	return EU_RESULT_OK;
}

eu_result eumport_u8_ready(europa* s, eu_mport* port, int* ready) {
	if (!s || !port || !ready)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->next || port->rpos >= port->size)
		*ready = EU_FALSE;
	else
		*ready = EU_TRUE;
	return EU_RESULT_OK;
}
