#include <ctype.h>
#include <stdio.h>

#include "europa.h"
#include "eu_port.h"
#include "eu_error.h"
#include "eu_number.h"
#include "eu_character.h"
#include "eu_util.h"

#include "utf8.h"

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
#define CPLUS '+'
#define CMINUS '-'
#define CDOT '.'
#define CBSLASH '\\'

#define iseof(c) ((c) == EOF)
#define isverticalline(c) ((c) == CVLINE)
#define islineending(c) ((c) == '\n')
#define iswhitespace(c) ((c) == ' ' || (c) == '\t')
#define isitspace(c) (iswhitespace(c) || (c) == CSCOLON || (c) == CHASH)
#define islpar(c) ((c) == CLPAR)
#define isrpar(c) ((c) == CRPAR)
#define isdelimiter(c) (iswhitespace(c) || islineending(c) || islpar(c) || \
	isrpar(c) || (c) == CDQUOT || (c) == CSCOLON || iseof(c) || (c) == '\0')
#define iswhitespace(c) ((c) == ' ' || (c) == '\t')

#define isexactness(c) ((c) == 'e' || (c) == 'E' || (c) == 'i' || (c) == 'I')
#define isradix(c) ((c) == 'b' || (c) == 'B' || (c) == 'o' || (c) == 'O' || \
	(c) == 'd' || (c) == 'D' || (c) == 'x' || (c) == 'X')
#define isbool(c) ((c) == 't' || (c) == 'T' || (c) == 'f' || (c) == 'F')
#define issign(c) ((c) == '-' || (c) == '+')

#define isbinarydigit(c) ((c) == '0' || c == '1')
#define isoctaldigit(c) ((c) >= '0' && (c) <= '7')
#define isdecimaldigit(c) ((c) >= '0' && c <= '9')
#define ishexdigit(c) (((c) >= '0' && (c) <= '9') || \
	((c) >= 'A' && (c) <= 'F') || \
	((c) >= 'a' && (c) <= 'f'))

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
	if ((res = cond)) \
		return res

#define errorf(p, fmt, ...) \
	snprintf(p->buf, AUX_BUFFER_SIZE, "%d:%d: " fmt, p->line, p->col,\
		__VA_ARGS__)

#define seterrorf(p, fmt, ...) \
	do {\
		errorf(p, fmt, __VA_ARGS__);\
		p->error = euerror_new(p->s, EU_ERROR_READ, p->buf);\
	} while(0)

#define seterror(p, fmt) \
	do {\
		snprintf(p->buf, AUX_BUFFER_SIZE, "%d:%d: " fmt, p->line, p->col);\
		p->error = euerror_new(p->s, EU_ERROR_READ, p->buf);\
	} while(0)

/* some needed prototypes */
eu_result parser_read(parser* p, eu_value* out);
eu_result pskip_itspace(parser* p);

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
	if ((res = pconsume(p)))
		return res;
	return ppeek(p);
}

eu_result pmatch(parser* p, int c) {
	if (p->current != c) {
		seterrorf(p, "Expected character '%c' but got %x ('%c').",
			c, p->current, (char)p->current);
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

eu_result pcasematch(parser* p, int c) {
	if (tolower(p->current) != tolower(c)) {
		seterrorf(p, "Expected (case-insensitive) '%c' but got %x ('%c').",
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
		if ((res = pmatch(p, cp)))
			break;
		if ((res = padvance(p)))
			break;
		next = utf8codepoint(next, &cp);
	}

	if (res) {
		seterrorf(p, "Could not match expected '%s'.", (char*)str);
		return res;
	}

	return EU_RESULT_OK;
}

eu_result pmatchstringcase(parser* p, void* str) {
	void* next;
	int cp;
	eu_result res;

	next = utf8codepoint(str, &cp);
	while (cp && next) {
		if ((res = pcasematch(p, cp)))
			break;
		if ((res = padvance(p)))
			break;
		next = utf8codepoint(next, &cp);
	}

	if (res) {
		seterrorf(p, "Could not match (ignoring case) expected '%s'.", (char*)str);
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
				res = pmatchstringcase(p, "true");
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
				res = pmatchstringcase(p, "false");
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

#define UNEXPECTED_DIGIT_IN_RADIX_STR "Unexpected digit %c for radix '%c' in number literal."
#define UNEXPECTED_CHAR_IN_NUMBER_STR "Unexpected character %c in number literal."
/* reads a <number> 
 *
 * WARNING: this implementation currently ignores explicitely exact real numbers,
 * integer numbers are exact unless #i is explicitely set and real (non-integer)
 * numbers are numbers are always inexact.
 */
eu_result pread_number(parser* p, eu_value* out) {
	eu_result res;
	int exactness = 'a', radix = 'd', sign = 1, value = 0, divideby = 0, aux;
	eu_integer ipart = 0;
	eu_real rpart = 0.0;

	/* test for explicit exactness/radix */
	if (p->current == CHASH) {
		if (isexactness(p->peek)) { /* exactnes radix */
			_checkreturn(res, padvance(p));
			exactness = tolower(p->current);

			/* check for radix */
			if (p->peek == CHASH) {
				_checkreturn(res, padvance(p));
				if (!isradix(p->peek)) {
					seterror(p, "Expected radix for number literal.");
					return EU_RESULT_ERROR;
				}
				_checkreturn(res, padvance(p)); /* consume # */
				radix = tolower(p->current);
			}
		} else if (isradix(p->peek)) { /* radix exactness */
			_checkreturn(res, padvance(p));
			radix = tolower(p->current);

			/* check for exactness */
			if (p->peek == CHASH) {
				_checkreturn(res, padvance(p));
				if (!isexactness(p->peek)) {
					seterror(p, "Expected exactness for number literal.");
					return EU_RESULT_ERROR;
				}
				_checkreturn(res,  padvance(p)); /* consume # */
				exactness = tolower(p->current);
			}
		} else { /* this is not a number token, it does not start with a prefix */
			seterrorf(p, "Unexpected character %c parsing a number (<prefix>).",
				p->peek);
			return EU_RESULT_ERROR;
		}
		/* consume final exactness/radix character */
		_checkreturn(res, padvance(p));
	}

	/* read optinal sign */
	if (issign(p->current)) {
		sign = p->current == '+' ? 1 : -1;
		_checkreturn(res, padvance(p));
	}

	/* convert radix to meaningful value */
	switch (radix) {
		case 'd': radix = 10; break;
		case 'b': radix = 2; break;
		case 'o': radix = 8; break;
		case 'x': radix = 16; break;
		default:
			seterrorf(p, "Invalid number radix '%c'.", radix);
			return EU_RESULT_ERROR;
	}

	/* read number */
	do {
		if (p->current == CDOT) {
			if (divideby != 0) { /* in case a dot appeared already */
				seterrorf(p, UNEXPECTED_CHAR_IN_NUMBER_STR, p->current);
				return EU_RESULT_ERROR;
			}
			/* TODO: maybe report when '.' appears in an explicitely exact
			 * number */

			_checkreturn(res, padvance(p)); /* consume the dot */

			/* place whatever was recognized as integer part so far into the 
			 * real part and reset the integer part */
			rpart = (eu_real)ipart;
			ipart = 0;

			/* set divideby to one so it will behave properly when multiplied 
			 * by radix */
			divideby = 1;

			continue; /* restart the loop */
		}

		/* assert digit is valid for current radix */
		if ((radix == 2 && !isbinarydigit(p->current)) ||
			(radix == 8 && !isoctaldigit(p->current)) ||
			(radix == 10 && !isdecimaldigit(p->current)) ||
			(radix == 16 && !ishexdigit(p->current))) {
			seterrorf(p, UNEXPECTED_DIGIT_IN_RADIX_STR, p->current, radix);
			return EU_RESULT_ERROR;
		}

		/* since most of the radices accept numbers in the 0-9 range, we
		 * can pre-calculate the value in a "radix-independent" way */
		value = p->current - '0';

		/* if the digit was in the A-F range, we need to correct 'value' */
		aux = tolower(p->current);
		if (aux >= 'a' && aux <= 'f')
			value = (aux - 'a') + 10;

		/* update integer part */
		ipart = (ipart * radix) + value;

		/* if no '.' appeared, then divideby will be 0 and the following line
		 * will result in divideby being zero still. 
		 * if a '.' appeared, then divideby will be the value we need to divide
		 * ipart by and sum to rpart to represent the final real number. */
		divideby *= radix;

		/* advance a character */
		_checkreturn(res, padvance(p));
	} while (!isdelimiter(p->current));

	if (divideby) { /* in case the resulting number is real */
		rpart = (eu_real)sign * (rpart + ((eu_real)ipart / (eu_real)divideby));
		_eu_makereal(out, rpart);
		return EU_RESULT_OK;
	}

	if (exactness == 'i') { /* in case the number was explicitely inexact int */
		_eu_makereal(out, (eu_real)(sign * ipart));
		return EU_RESULT_OK;
	}

	/* in case the resulting number was an exact integer */
	_eu_makeint(out, sign * ipart);
	return EU_RESULT_OK;
}

#define _checkchar(buf, out, str, c) \
	if (utf8casecmp(buf, str) == 0) {\
		_eu_makechar(out, c);\
		return EU_RESULT_OK;\
	}
#define CHAR_BUF_SIZE 128

eu_result pread_character(parser* p, eu_value* out) {
	eu_result res;
	int c, v, pos;
	char buf[CHAR_BUF_SIZE];
	void *next;
	size_t size;

	_checkreturn(res, pmatch(p, CHASH));
	_checkreturn(res, padvance(p));
	_checkreturn(res, pmatch(p, CBSLASH));
	_checkreturn(res, padvance(p));

	if (isdelimiter(p->peek)) { /* the #\<char> case */
		_eu_makechar(out, p->current);
		_checkreturn(res, padvance(p));
		return EU_RESULT_OK;
	} else if (p->current == 'x' || p->current == 'X') { /* the #\x<hexval> case */
		_checkreturn(res, padvance(p)); /* consume 'x' */

		c = 0;
		/* read the hex value */
		while (!isdelimiter(p->current)) {
			/* check for invalid digits */
			if (!ishexdigit(p->current)) {
				seterrorf(p,
					"Invalid character '%c' in unicode hex character literal.",
					(char)p->current);
				return EU_RESULT_ERROR;
			}

			/* calculate current character value from 0 to 15 */
			v = tolower(p->current);
			if (v <= 'a' && v >= 'f')
				v = v - 'a' + 10;
			else
				v = v - '0';

			c = (c << 4) | v;

			/* advance to the next character */
			_checkreturn(res, padvance(p));
		}

		/* conver the character to UTF-8 */
		c = unicodetoutf8(c);

		_eu_makechar(out, c);
		return EU_RESULT_OK;
	} else { /* the #\<character name> case */
		/* read the character name into the aux buffer */
		size = CHAR_BUF_SIZE;
		next = buf;
		while (next != NULL && !isdelimiter(p->current)) {
			next = utf8catcodepoint(next, p->current, size);
			size -= utf8codepointsize(p->current);
			_checkreturn(res, padvance(p));
		}

		/* check if the char name was too big */
		if (next == NULL) {
			seterror(p, "Literal character name too big (and probably invalid).");
			return EU_RESULT_ERROR;
		}
		cast(eu_byte*, next)[0] = '\0';

		/* check for names */
		_checkchar(buf, out, "alarm", '\n');
		_checkchar(buf, out, "backspace", '\n');
		_checkchar(buf, out, "delete", '\n');
		_checkchar(buf, out, "escape", '\n');
		_checkchar(buf, out, "newline", '\n');
		_checkchar(buf, out, "return", '\n');
		_checkchar(buf, out, "space", '\n');
		_checkchar(buf, out, "tab", '\n');

		/* in case it didn't match any valid character */
		seterrorf(p, "Unknown character literal name '%s'.", buf);
		return EU_RESULT_ERROR;
	}
}

eu_result pread_hash(parser* p, eu_value* out) {
	eu_result res;

	/* match the hash character */
	_checkreturn(res, pmatch(p, CHASH));

	if (isbool(p->peek))
		return pread_boolean(p, out);
	else if (isradix(p->peek) || isexactness(p->peek))
		return pread_number(p, out);
	else if (p->peek == CBSLASH)
		return pread_character(p, out);

	seterror(p, "Expected a boolean, number, ..., but nothing matched.");
	return EU_RESULT_ERROR;
}

eu_result pskip_linecomment(parser* p) {
	eu_result res;

	if ((res = pmatch(p, CSCOLON)))
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
		_checkreturn(res, padvance(p));
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
				_checkreturn(res, padvance(p));

			_checkreturn(res, padvance(p));
		}
		else if (p->current == CSCOLON) { /* single line comment, starting with ';' */
			_checkreturn(res, pskip_linecomment(p)); /* skip the comment and continue in the loop */
		}
		else if (p->current == CHASH) { /* might be a comment */
			if (p->peek == CVLINE) { /* skip a nested comment */
				_checkreturn(res, pskip_nestedcomment(p));
			}
			else if (p->peek == CSCOLON) { /* skip a datum comment */
				_checkreturn(res, pskip_datumcomment(p));
			}
			else { /* it does not qualify for intertoken space */
				break;
			}
		}
	}

	/* we already read any intertoken space available, so just return success */
	return EU_RESULT_OK;
}

/* this reads a <datum> */
eu_result parser_read(parser* p, eu_value* out) {
	eu_result res;

	/* check and skip inter token spaces (aka <atmosphere>) */
	if (isitspace(p->current)) {
		_checkreturn(res, pskip_itspace(p));
	}

	/* check terminals that start with # */
	if (p->current == CHASH) {
		return pread_hash(p, out);
	} else if (isdecimaldigit(p->current) ||
		(issign(p->current) && isdecimaldigit(p->peek))) {
		return pread_number(p, out);
	}

	return EU_RESULT_OK;
}

eu_result euport_read(europa* s, eu_port* port, eu_value* out) {
	parser p;
	eu_result res;

	if (!s || !port || !out)
		return EU_RESULT_NULL_ARGUMENT;

	parser_init(&p, s, port);
	_checkreturn(res, padvance(&p));
	if ((res = parser_read(&p, out))) {
		out->type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE;
		out->value.object = _euerror_to_obj(p.error);
		return res;
	}

	return EU_RESULT_OK;
}
