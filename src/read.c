#include "europa.h"
#include "eu_port.h"
#include "eu_error.h"
#include "utf8.h"
#include <stdio.h>

/* This is where the first parser exists. Thread carefully!
 * 
 * This first iteration of the parser reads from ports a character at a time.
 * The goal is that each port implementation would have a parser of its own in
 * order to make optimizations.
 * 
 * This parser is written using only the concept of "port" instead of files or
 * memory buffers directly. This will result in a performance penalty. Even more
 * so because it uses the port's "public" API, so port will end up being
 * compared to NULL a thousand times, but the second I finish this parser, it
 * will also work for in memory strings so long as string-port implements the
 * interface correctly.
 * 
 */
#define AUX_BUFFER_SIZE 1024
#ifndef EOF
#define EOF -1
#endif

#define CHASH '#'
#define CLPAR '('
#define CRPAR ')'
#define CDQUOT '"'
#define CSQUOT '\''
#define CVLINE '|'
#define CSCOLON ';'

#define iseof(c) (c == EOF)
#define isverticalline(c) (c == CVLINE)
#define islineending(c) (c == '\n')
#define iswhitespace(c) (c == ' ' || c == '\t')
#define isitspace(c) (iswhitespace(c) || c == CSCOLON || c == CHASH)
#define islpar(c) (c == CLPAR)
#define isrpar(c) (c == CRPAR)
#define isdelimiter(c) (iswhitespace(c) || islineending(c) || islpar(c) || \
	isrpar(c) || c == CDQUOT || c == CSCOLON || iseof(c))
#define iswhitespace(c) (c == ' ' || c == '\t')

typedef struct parser parser;
struct parser {
	europa* s;
	eu_port* port;
	eu_error* error;
	int current, peek, line, col;
	char buf[AUX_BUFFER_SIZE];
};

/* some helper macros */
#define _checkreturn(res, cond) \
	if (res = cond) \
		return res

#define errorf(p, fmt, ...) \
	snprintf(p->buf, AUX_BUFFER_SIZE, "%d:%d: " fmt, p->line, p->col,\
		##__VA_ARGS__)

#define seterrorf(p, fmt, ...) \
	do {\
		errorf(p, fmt, ##__VA_ARGS__);\
		p->error = euerror_new(p->s, EU_ERROR_READ, p->buf);\
	} while(0)

#define seterror(p, fmt) \
	do {\
		snprintf(p->buf, AUX_BUFFER_SIZE, "%d:%d: " fmt, p->line, p->col);\
		p->error = euerror_new(p->s, EU_ERROR_READ, p->buf);\
	} while(0)

eu_result parser_init(parser* p, europa* s, eu_port* port) {
	int res;
	p->s = s;
	p->port = port;
	p->current = EOF;
	p->line = p->col = 0;
	p->error = NULL;
	return euport_peek_char(s, port, &(p->peek));
}

eu_result pconsume(parser* p) {
	p->col++;
	if (islineending(p->current)) {
		p->line++;
		p->col = 0;
	}
	return euport_read_char(p->s, p->port, &(p->current));
}

eu_result ppeek(parser* p) {
	return euport_peek_char(p->s, p->port, &(p->peek));
}

eu_result padvance(parser* p) {
	int res;
	if (res = pconsume(p))
		return res;
	return ppeek(p);
}

eu_result pmatch(parser* p, int c) {
	if (p->current != c) {
		seterrorf(p, "Expected character '%c' but got %d ('%c').",
			c, p->current, (char)p->current);
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result pmatchstring(parser* p, void* str) {
	void* next;
	int cp;
	eu_result res;

	next = utf8codepoint(str, &cp);
	while (cp && next) {
		if (res = pmatch(p, cp))
			break;
		if (res = padvance(p))
			break;
		next = utf8codepoint(next, &cp);
	}

	if (res) {
		seterrorf(p, "Could not match expected '%s'.", (char*)str);
		return res;
	}

	return EU_RESULT_OK;
}

eu_result pread_boolean(parser* p, eu_value* out) {
	eu_result res;

	_checkreturn(res, pmatch(p, CHASH));
	_checkreturn(res, padvance(p));

	switch (p->current) {
		case 't':
			if (!isdelimiter(p->peek)) { /* in case it wasnt exactly '#t' */
				res = pmatchstring(p, "true");
				if (res) /* in case it wasnt '#true' */
					return res;

				if (!isdelimiter(p->current)) { /* in case it wasn't exactly '#true' */
					seterror(p, "Invalid token provided when 'true' was expected.");
					return EU_RESULT_INVALID;
				}

				/* at this point we either returned an error or consumed all "true"
				 * characters. so we're already with p->current at the character just 
				 * after the 'e' */
			} else {
				/* consume the 't' character */
				padvance(p);
			}
			/* this code will only be reached if the input was either '#t' or 
			 * '#true' so we set the value to the true boolean and that's it */
			*out = _true;
			return EU_RESULT_OK;
		case 'f':
			if (!isdelimiter(p->peek)) { /* in case it wasnt exactly '#f' */
				res = pmatchstring(p, "false");
				if (res) /* in case it wasnt '#false' */
					return res;

				if (!isdelimiter(p->current)) { /* in case it wasn't exactly '#false' */
					seterror(p, "Invalid token provided when 'false' was expected.");
					return EU_RESULT_INVALID;
				}

				/* at this point we either returned an error or consumed all "false"
				 * characters. so we're already with p->current at the character just 
				 * after the 'e' */
			} else {
				/* consume the 't' character */
				padvance(p);
			}
			/* this code will only be reached if the input was either '#f' or 
			 * '#false' so we set the value to the true boolean and that's it */
			*out = _false;
			return EU_RESULT_OK;
			break;
		default:
			seterror(p, "Parser in invalid state.");
			return EU_RESULT_ERROR;
	}
}

eu_result pread_hash(parser* p, eu_value* out) {
	eu_result res;

	/* match the hash character */
	_checkreturn(res, pmatch(p, CHASH));

	switch (p->peek) {
		case 't': /* should be a boolean constant */
			return pread_boolean(p, out);
		case 'f':
			return pread_boolean(p, out);
		case '!':
			/* read directive */
			break;
		case 'b': case 'B':
		case 'o': case 'O':
		case 'd': case 'D':
		case 'x': case 'X':
			/* TODO: read number with radix */
			break;
		case '\\':
			/* TODO: read character literal */
			break;
		default:
			break;
	}

	return 	EU_RESULT_INVALID;
}

eu_result pskip_linecomment(parser* p) {
	eu_result res;

	if (res = pmatch(p, CSCOLON))
		return res;

	do {
		_checkreturn(res, padvance(p));
	} while (p->current != '\n' && p->current != '\r' && p->current != EOF);

	if (p->current == '\r' && p->peek == '\n') { /* check if line ending is \r\n */
		_checkreturn(res, padvance(p));
	}

	/* consume the final line ending character */
	_checkreturn(res, padvance(p));

	return EU_RESULT_OK;
}

/* skips a nested comment */
eu_result pskip_nestedcomment(parser* p) {
	eu_result res;
	int nestedcount = 0;

	_checkreturn(res, pmatch(p, CHASH)); /* make sure we're at a # */

	while (p->current != EOF) {
		if (p->current == CHASH && p->peek == CVLINE) {
			/* we found an opening '#|' */
			/* consume the # */
			_checkreturn(res, padvance(p));

			/* increment the nestedcount */
			nestedcount++;
		} else if (p->current == CVLINE && p->peek == CHASH) {
			/* we found a closing '|#' */
			/* consume the | */
			_checkreturn(res, padvance(p));

			/* decrement nested count */
			nestedcount--;

			/* check that there aren't more closings than openings */
			if (nestedcount < 0) {
				/* in which case consume the hash */
				_checkreturn(res, padvance(p));
				/* and break to return the error */
				break;
			} else if (nestedcount == 0) {
				/* in case we just closed the last comment */
				/* consume the # character and break */
				_checkreturn(res, padvance(p));
				break;
			}
		}
		_checkreturn(res, padvance);
	}

	if (nestedcount > 0) { /* more comments were started than ended */
		errorf(p, "%d more comments were opened than ended.", nestedcount);
		return EU_RESULT_ERROR;
	} else if (nestedcount < 0) { /* more comments were ended than started*/
		seterror(p, "More nested comments were closed than opened.");
		return EU_RESULT_ERROR;
	} else { /* comments were properly nested */
		return EU_RESULT_OK;
	}
}

/* skips datum comment */
eu_result pskip_datumcomment(parser* p) {
	eu_result res;
	eu_value out;

	_checkreturn(res, pmatch(p, CHASH));
	_checkreturn(res, padvance(p));
	_checkreturn(res, pmatch(p, CSCOLON));
	_checkreturn(res, padvance(p));

	/* skip any intertoken space */
	_checkreturn(res, pskip_itspace(p));

	/* read a datum that will be ignored */
	/* TODO: create a function that skips datum instead of just ignoring it */
	_checkreturn(res, parser_read(p, &out));

	return EU_RESULT_OK;
}

/* skips intertoken space */
eu_result pskip_itspace(parser* p) {
	eu_result res;

	while (isitspace(p->current)) {
		if (iswhitespace(p->current) || p->current == '\r' ||
			p->current == '\n') {

			/* in case it was a \r\n sequence, also consume the \n */
			if (p->current == '\r' && p->peek == '\n')
				if (res = padvance(p))
					return res;

			if (res = padvance(p))
				return res;
		}
		else if (p->current == CSCOLON) { /* single line comment, starting with ';' */
			_checkreturn(res, pskip_linecomment(p)); /* skip the comment and continue in the loop */
		}
		else if (p->current == CHASH) { /* might be a comment */
			if (p->peek == CVLINE) /* skip a nested comment */
				_checkreturn(res, pskip_nestedcomment(p));
			else if (p->peek == CSCOLON) /* skip a datum comment */
				_checkreturn(res, pskip_datumcomment(p));
			else /* it does not qualify for intertoken space */
				break;
		}
	}

	/* we already read any intertoken space available, so just return success */
	return EU_RESULT_OK;
}

/* this reads a <datum> */
eu_result parser_read(parser* p, eu_value* out) {
	eu_result res;

	if (isitspace(p->current)) {
		_checkreturn(res, pskip_itspace(p));
	} else if (p->current == CHASH) {
		_checkreturn(res, pread_hash(p, &out));
	}
}

eu_result euport_read(europa* s, eu_port* port, eu_value* out) {
	parser p;
	eu_result res;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	parser_init(&p, s, port);
	if (res = parser_read(&p, out)) {
		out->type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE;
		out->value.object = _euerror_to_obj(p.error);
		return res;
	}

	return EU_RESULT_OK;
}