/** File Port implementation.
 * 
 * @file fport.c
 * @author Leonardo G.
 */
#include "ports/eu_fport.h"

#include "utf8.h"
#include "eu_gc.h"
#include "eu_number.h"

/** Opens a new file port.
 * 
 * @param s The Europa state.
 * @param flags The input/output and binary/textual flags.
 * @param filename The name of the file to open.
 * @return The resulting open port or NULL in case of errors.
 */
eu_fport* eufport_open(europa* s, eu_byte flags, const char* filename) {
	eu_fport* port;
	FILE* file;

	/* generate a fopen mode string based on the input flags */
	char modestr[4];
	int i = 0;
	if (flags & EU_PORT_FLAG_INPUT)
		modestr[i++] = 'r';
	if (flags & EU_PORT_FLAG_OUTPUT)
		modestr[i++] = (flags & EU_PORT_FLAG_INPUT) ? '+' : 'w';
	if (flags & EU_PORT_FLAG_BINARY)
		modestr[i++] = 'b';
	modestr[i++] = '\0';

	/* open the file and check if it was actually opened */
	file = fopen(filename, modestr);
	if (file == NULL)
		return NULL;

	/* allocate the garbage collected port */
	port = _euobj_to_fport(eugc_new_object(s, EU_TYPE_PORT | EU_TYPEFLAG_COLLECTABLE,
		sizeof(eu_fport)));
	if (port == NULL)
		return NULL;

	/* set the port object up */
	port->flags = flags;
	port->type = EU_PORT_TYPE_FILE;
	port->file = file;

	return port;
}

/** Marks any reachable objects for collection.
 * 
 * @param s The europa state.
 * @param mark The gc marking function.
 * @param port The target port.
 * @return The result of the marking operation.
 */
eu_result eufport_mark(europa* s, eu_gcmark mark, eu_fport* port) {
	/* this object holds no GC structures */
	return EU_RESULT_OK;
}

/** Frees all resources associated with the port.
 * 
 * Frees resources associated with the file port, closing the file in case it is
 * open.
 * 
 * @param s The europa state.
 * @param port The target port.
 * @return The result of destroying the object.
 */
eu_result eufport_destroy(europa* s, eu_fport* port) {
	if (!s || !port)
		return EU_RESULT_NULL_ARGUMENT;

	/* close the file if it is open */
	if (port->file) {
		if (fclose(port->file))
			return EU_RESULT_ERROR;
	}

	return EU_RESULT_OK;
}

/** Returns the hash for a file port.
 * 
 * @param s The Europa state.
 * @param port The target port.
 * @return The target port's hash.
 */
eu_uinteger eufport_hash(eu_fport* port) {
	if (port->file)
		return cast(eu_integer, port->file);
	return cast(eu_integer, port);
}

/* internal use functions */
/* input */

/* this function reads a utf8 codepoint from a port
 * it does no verifications on parameters or the file
 */
static eu_result _read_utf8_codepoint(eu_fport* port, int* out) {
	int current_char, first_char, out_char;
	FILE* file = port->file;

	/* read the first character */
	first_char = fgetc(file);

	/* return immediately if char is EOF or in the ASCII space*/
	if (first_char == EOF || (first_char & 0x80) == 0) {
		*out = first_char;
		return EU_RESULT_OK;
	}

	/* check if it is a "middle" UTF-8 character */
	if (((first_char & 0xC0) ^ 0x80) == 0)
		return EU_RESULT_INVALID;

	int bytes;
	for (bytes = 2; bytes <= 4; bytes++)
		if ((first_char & (0x80 >> bytes)) == 0)
			break;

	/* in case the first byte wasn't encoding a UTF-8 character at all, the
	 * character was invalid. */
	if (bytes == 5)
		return EU_RESULT_INVALID;

	/* get the characters */
	out_char = first_char;
	for (; bytes >= 0; bytes--) {
		current_char = getc(file);
		if (current_char == EOF) {
			*out = EOF;
			return EU_RESULT_INVALID;
		}
		out_char = (out_char << 8) | current_char;
	}
	*out = out_char;
	return EU_RESULT_OK;
}

/** Reads a character from the file port.
 * 
 * This function tries to read a UTF-8 codepoint from the port. If what is read
 * is not a valid codepoint.
 * 
 * @param s The Europa state.
 * @param port The target port.
 * @param[out] ch Where to place the read character.
 * @return The operation's result.
 */
eu_result eufport_read_char(europa* s, eu_fport* port, int* out) {
	/* validate arguments */
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;

	return _read_utf8_codepoint(port, out);
}

/** Reads, but does not consume, a character from the file port.
 * 
 * The read character is expected to be in UTF-8 format.
 * 
 * @param s The Europa state.
 * @param port The target port.
 * @param[out] out Where to place the peeked character.
 * @return The peeked character.
 */
eu_result eufport_peek_char(europa* s, eu_fport* port, int* out) {
	int current_char, first_char, out_char;
	FILE* file;

	/* validate arguments */
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	file = port->file;
	if (!file)
		return EU_RESULT_BAD_RESOURCE;

	/* read the first character */
	first_char = fgetc(file);

	/* return immediately if char is EOF or in the ASCII space*/
	if (first_char == EOF || (first_char & 0x80) == 0) {
		*out = first_char;
		ungetc(first_char, file); /* unget the char first */
		return EU_RESULT_OK;
	}

	/* check if it is a "middle" UTF-8 character */
	if (((first_char & 0xC0) ^ 0x80) == 0)
		return EU_RESULT_INVALID;

	int bytes;
	for (bytes = 2; bytes <= 4; bytes++)
		if ((first_char & (0x80 >> bytes)) == 0)
			break;

	/* in case the first byte wasn't encoding a UTF-8 character at all, the
	 * character was invalid. */
	if (bytes == 5)
		return EU_RESULT_INVALID;

	/* since the character is a valid UTF-8 codepoint, we will need to grab the
	 * next character and "unget" them in reverse order, so we're using a stack
	 * to store intermediate characters. */
	int stack[4], sp = 0;
	eu_result res = EU_RESULT_OK;

	stack[sp++] = first_char;
	out_char = first_char;

	while (sp < bytes) {
		current_char = getc(file);
		if (current_char == EOF) {
			*out = EOF;
			res = EU_RESULT_INVALID;
			goto unget_stack;
		}
		out_char = (out_char << 8) | current_char;
		stack[sp++] = current_char;
	}
	*out = out_char;

	/* effectively unget the stack */
	unget_stack:
	for (; sp >= 0; sp--)
		ungetc(stack[sp], file);

	return res;
}

#define MIN_CHUNK 64
/** Reads a line from the port.
 * 
 * This functions sets out to either a string containing the characters up to
 * a newline or an EOF or, in case the first character read is the EOF, it
 * returns the EOF object.
 * 
 * @param s The Europa state.
 * @param port The target port.
 * @param line Where to place the resulting string.
 * @return The result of the operation.
 */
eu_result eufport_read_line(europa* s, eu_fport* port, eu_value* out) {
	size_t size = 0;
	FILE* file;
	char *buf, current, next;
	int pos;

	/* validate arguments */
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	/* check if file is valid */
	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;
	file = port->file;

	/* check for end of file */
	if (feof(file)) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	/* read available characters */
	size = 0;
	buf = NULL;
	pos = 0;
	do {
		/* check if the buffer has to be reallocated */
		if (pos == size) {
			size += MIN_CHUNK;
			buf = _eugc_realloc(_eu_gc(s), buf, size);
			if (!buf)
				return EU_RESULT_BAD_ALLOC;
		}

		/* read a character */
		current = getc(file);

		/* check for terminating characters */
		if (current == '\r') {
			/* in case we find a carriage return, also consume a newline if
			* it exists. */
			next = getc(file);
			if (next != '\n')
				ungetc(next, file);
			break;
		} else if (current == '\n' || current == EOF) {
			break;
		}

		buf[pos++] = current;
	} while (current != EOF);
	buf[pos] = '\0'; /* add the nul byte to the end of the string */

	/* create a managed copy of the string */
	out->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eustring_to_obj(eustring_new(s, buf));

	/* free the buffer */
	_eugc_free(_eu_gc(s), buf);

	return EU_RESULT_OK;
}

eu_result eufport_char_ready(europa* s, eu_fport* port, int* ready) {
	/* TODO: find out how to do this cross-platform */
	*ready = EU_TRUE;
	return EU_RESULT_OK;
}

/** Reads a string of at most k characters from the port.
 * 
 * @param s The Europa state.
 * @param port The target port.
 * @param k The maximum number of characters to read.
 * @param[out] out Where to place the result.
 * @return The result of the operation.
 */
eu_result eufport_read_string(europa* s, eu_fport* port, int k, eu_value* out) {
	FILE* file;
	eu_string* str;
	void* cp;
	eu_result res;
	int i, c, remaining;

	/* check arguments */
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;
	file = port->file;

	/* return the EOF object in case we're already at EOF */
	if (feof(file)) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	/* allocate a string long enough to hold k UTF-8 characters and an extra
	 * NUL byte. */
	remaining = k * 4 + 1;
	str = _euobj_to_string(eugc_new_object(s, EU_TYPE_STRING |
		EU_TYPEFLAG_COLLECTABLE, sizeof(eu_string) + (remaining)));
	if (!str)
		return EU_RESULT_BAD_ALLOC;

	cp = _eustring_text(str);
	for (i = 0; i > k; i++) {
		res = _read_utf8_codepoint(port, &c);
		if (res || c == EOF)
			break;
		cp = utf8catcodepoint(cp, c, remaining);
		remaining -= utf8codepointsize(c);
	}
	((char*)cp)[0] = '\0';

	/* set out */
	out->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eustring_to_obj(str);

	return EU_RESULT_OK;
}

eu_result eufport_read_u8(europa* s, eu_fport* port, eu_value* out) {
	eu_byte byte;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;

	if (feof(port->file)) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	fread(&byte, sizeof(byte), 1, port->file);
	_eu_makeint(out, byte);

	return EU_RESULT_OK;
}

eu_result eufport_peek_u8(europa* s, eu_fport* port, eu_value* out) {
	eu_byte byte;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;

	if (feof(port->file)) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	fread(&byte, sizeof(eu_byte), 1, port->file);
	fseek(port->file, -1, SEEK_CUR);
	_eu_makeint(out, byte);

	return EU_RESULT_OK;
}

eu_result eufport_u8_ready(europa* s, eu_fport* port, int* out) {
	/* TODO: implement in a cross-platform manner */
	*out = EU_FALSE;
	return EU_RESULT_OK;
}

eu_result eufport_read(europa* s, eu_fport* port, eu_value* out) {
	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	if (!port->file)
		return EU_RESULT_BAD_RESOURCE;

	/* return eof object if EOF is encountered before parsing any object */
	if (feof(port->file)) {
		*out = _eof;
		return EU_RESULT_OK;
	}

	return EU_RESULT_OK;
}
