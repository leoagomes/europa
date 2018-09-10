#include <ctype.h>
#include <stdio.h>

#include "europa.h"
#include "eu_port.h"
#include "eu_error.h"
#include "eu_number.h"
#include "eu_character.h"
#include "eu_util.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_bytevector.h"

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
#define CLSQB '['
#define CRSQB ']'
#define CLCRLB '{'
#define CRCRLB '}'
#define CDQUOT '"'
#define CSQUOT '\''
#define CVLINE '|'
#define CSCOLON ';'
#define CPLUS '+'
#define CMINUS '-'
#define CDOT '.'
#define CBSLASH '\\'
#define CEXCL '!'
#define CDOLLAR '$'
#define CPERCENT '%'
#define CAMP '&'
#define CASTERISK '*'
#define CFSLASH '/'
#define CCOLON ':'
#define CLESS '<'
#define CGREAT '>'
#define CEQUALS '='
#define CQUEST '?'
#define CAT '@'
#define CHAT '^'
#define CUNDERSC '_'
#define CTILDE '~'

#define iseof(c) ((c) == EOF)
#define isverticalline(c) ((c) == CVLINE)
#define islineending(c) ((c) == '\n')
#define iswhitespace(c) ((c) == ' ' || (c) == '\t')
#define isitspace(c) (iswhitespace(c) || (c) == CSCOLON || (c) == CHASH)
#define islpar(c) ((c) == CLPAR || (c) == CLSQB || (c) == CLCRLB)
#define isrpar(c) ((c) == CRPAR || (c) == CRSQB || (c) == CRCRLB)
#define isdelimiter(c) (iswhitespace(c) || islineending(c) || islpar(c) || \
	isrpar(c) || (c) == CDQUOT || (c) == CSCOLON || iseof(c) || (c) == '\0')

#define isexactness(c) ((c) == 'e' || (c) == 'E' || (c) == 'i' || (c) == 'I')
#define isradix(c) ((c) == 'b' || (c) == 'B' || (c) == 'o' || (c) == 'O' || \
	(c) == 'd' || (c) == 'D' || (c) == 'x' || (c) == 'X')
#define isbool(c) ((c) == 't' || (c) == 'T' || (c) == 'f' || (c) == 'F')
#define issign(c) ((c) == '-' || (c) == '+')
#define isdot(c) ((c) == CDOT)

#define isbinarydigit(c) ((c) == '0' || c == '1')
#define isoctaldigit(c) ((c) >= '0' && (c) <= '7')
#define isdecimaldigit(c) ((c) >= '0' && c <= '9')
#define ishexdigit(c) (((c) >= '0' && (c) <= '9') || \
	((c) >= 'A' && (c) <= 'F') || \
	((c) >= 'a' && (c) <= 'f'))

#define isspecialinitial(c) ((c) == CEXCL || (c) == CDOLLAR || (c) == CPERCENT ||\
	(c) == CAMP || (c) == CASTERISK || (c) == CFSLASH || (c) == CCOLON ||\
	(c) == CLESS || (c) == CEQUALS || (c) == CGREAT || (c) == CQUEST ||\
	(c) == CAT || (c) == CHAT || (c) == CUNDERSC || (c) == CTILDE)
#define isletter(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define isinitial(c) (isspecialinitial(c) || isletter(c))
#define isvline(c) ((c) == CVLINE)
#define isexplicitsign(c) ((c) == CPLUS || (c) == CMINUS)
#define ispeculiar(c) (isexplicitsign(c) || (c) == CDOT)
#define isidentifier(c) (isinitial(c) || isvline(c) || ispeculiar(c))
#define isspecialsubsequent(c) (isexplicitsign(c) || (c) == CDOT || (c) == CAT)
#define issubsequent(c) (isinitial(c) || isdecimaldigit(c) ||\
	isspecialsubsequent(c))
#define isdotsubsequent(c) (issubsequent(c) || isdot(c))
#define issignsubsequent(c) (isinitial(c) || isexplicitsign(c) || (c) == CAT)

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
eu_result pread_datum(parser* p, eu_value* out);
eu_result pskip_itspace(parser* p);


/* Because strings and identifiers can be arbitrarily long, the auxilary buffer
 * (p->buf) may be too small to hold them. Because of that, we take the approach
 * of using the auxilary buffer until it is full, then copying the data into the
 * heap, reallocating memory in chunks of a predetermined size whenever this
 * heap store is filled with string data.
 * 
 * To make this switch between p->buf and a heap allocated buffer transparent,
 * we use the following functions.
 */
eu_result gbuf_init(parser* p, void** buf, void** next, size_t* size) {
	/* set all variables to the auxilary buffer initially. */
	*buf = p->buf;
	*next = *buf;
	*size = AUX_BUFFER_SIZE;

	/* zero-out the buffer to ensure that there will be a final NUL byte. */
	memset(p->buf, 0, AUX_BUFFER_SIZE);

	return EU_RESULT_OK;
}

/* this is how much the buffer grows every time we need more memory for the
 * string/symbol.
 */
#define GBUF_GROWTH_RATE 128

eu_result gbuf_append(parser* p, void** buf, void** next, size_t* size,
	size_t* remaining, int c) {
	char *cbuf, *cnext;

	/* append the character */
	*next = utf8catcodepoint(*next, c, *remaining);

	/* check whether there was space for the character in the buffer */
	if (*next == NULL) {
		/* in case there wasn't, we need to either allocate a new heap buffer
		 * or grow the heap buffer. */
		if (*buf == p->buf) {
			/* we need to allocate a new buffer on the heap a bit larger than
			 * the auxilary buffer. */
			cbuf = eugc_malloc(_eu_get_gc(p->s), *size + GBUF_GROWTH_RATE);
			if (cbuf == NULL)
				return EU_RESULT_BAD_ALLOC;

			/* copy the contents of the aux buffer into the newly allocated 
			 * buffer */
			memcpy(cbuf, p->buf, AUX_BUFFER_SIZE);
		} else {
			/* the buffer currently lives in the heap, so we just need to grow it */
			cbuf = eugc_realloc(_eu_get_gc(p->s), *buf, *size + GBUF_GROWTH_RATE);
			if (cbuf == NULL)
				return EU_RESULT_BAD_ALLOC;
		}

		/* update the remainig space in the buffer and its size */
		*size += GBUF_GROWTH_RATE;
		*remaining += GBUF_GROWTH_RATE;

		/* update the effective values */
		*buf = cbuf;
		cnext = cbuf + (*size - *remaining);
		*next = cnext;

		/* try appending the character again */
		*next = utf8catcodepoint(*next, c, *remaining);
		if (*next == NULL)
			return EU_RESULT_ERROR;
	}
	*remaining -= utf8codepointsize(c);
	/* put a nul byte just after the last appended byte */
	cast(eu_byte*, *next)[0] = '\0';
	return EU_RESULT_OK;
}

eu_result gbuf_append_byte(parser* p, void** buf, void** next, size_t* size,
	size_t* remaining, eu_byte c) {
	eu_byte *bbuf, *bnext;

	bbuf = *buf;
	bnext = *next;

	if (remaining == 0) {
		if (*buf == p->buf) {
			/* we need to allocate a new buffer on the heap a bit larger than
			 * the auxilary buffer. */
			bbuf = eugc_malloc(_eu_get_gc(p->s), *size + GBUF_GROWTH_RATE);
			if (bbuf == NULL)
				return EU_RESULT_BAD_ALLOC;

			/* copy the contents of the aux buffer into the newly allocated 
			 * buffer */
			memcpy(bbuf, p->buf, AUX_BUFFER_SIZE);
		} else {
			/* the buffer currently lives in the heap, so we just need to grow it */
			bbuf = eugc_realloc(_eu_get_gc(p->s), *buf, *size + GBUF_GROWTH_RATE);
			if (bbuf == NULL)
				return EU_RESULT_BAD_ALLOC;
		}

		/* update the effective values */
		*buf = bbuf;
		bnext = bbuf + *size;
		*next = bnext;

		/* update the remainig space in the buffer and its size */
		*size += GBUF_GROWTH_RATE;
		*remaining += GBUF_GROWTH_RATE;
	}

	*bnext = c;
	bnext++;
	(*remaining) -= 1;

	return EU_RESULT_OK;
}

eu_result gbuf_terminate(parser* p, void** buf) {
	if (p->buf != *buf) {
		eugc_free(_eu_get_gc(p->s), *buf);
	}
	*buf = NULL;
	return EU_RESULT_OK;
}

/* initializes a parser structure */
eu_result parser_init(parser* p, europa* s, eu_port* port) {
	int res;
	p->s = s;
	p->port = port;
	p->current = EOF;
	p->line = p->col = 0;
	p->error = NULL;
	return euport_peek_char(s, port, &(p->peek));
}

/* consumes a character from the parser port */
eu_result pconsume(parser* p) {
	p->col++;
	if (islineending(p->current)) {
		p->line++;
		p->col = 0;
	}
	return euport_read_char(p->s, p->port, &(p->current));
}

/* peeks a character from the parser port, placing it into p->peek */
eu_result ppeek(parser* p) {
	return euport_peek_char(p->s, p->port, &(p->peek));
}

/* consumes the current character, making p->current = p->peek and populating
 * p->peek with the next peek character.
 * 
 * basically reads a character and peeks the other in a sequence */
eu_result padvance(parser* p) {
	int res;
	if ((res = pconsume(p)))
		return res;
	return ppeek(p);
}

/* asserts that the current character (p->current) matches a given character,
 * returning an error in case it does not match. */
eu_result pmatch(parser* p, int c) {
	if (p->current != c) {
		seterrorf(p, "Expected character '%c' but got %x ('%c').",
			c, p->current, (char)p->current);
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

/* asserts that the current character (p->current) matches a given character,
 * ignoring case, returning an error if they do not match. */
eu_result pcasematch(parser* p, int c) {
	if (tolower(p->current) != tolower(c)) {
		seterrorf(p, "Expected (case-insensitive) '%c' but got %x ('%c').",
			c, p->current, (char)p->current);
		return EU_RESULT_ERROR;
	}
	return EU_RESULT_OK;
}

/* applies the match procedure over all characters in a string, advancing the
 * port along with the string. */
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

/* applies the casematch procedure over all characters in a string, just like
 * matchstring. */
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

/* reads a <boolean> in either #t or #true (or #f and #false) formats, returning
 * an error if the sequence does not form a valid boolean. */
eu_result pread_boolean(parser* p, eu_value* out) {
	eu_result res;

	_checkreturn(res, pmatch(p, CHASH));
	_checkreturn(res, padvance(p));

	switch (p->current) {
		case 't':
			if (!isdelimiter(p->peek)) { /* in case it wasnt exactly '#t' */
				_checkreturn(res, pmatchstringcase(p, "true"));

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
				_checkreturn(res, pmatchstringcase(p, "false"));

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
 * <number> is defined in a bnf-like language:
 * <number> := <num 2> | <num 8> | <num 10> | <num 16>
 * <num R> := <prefix R> <real R>
 * <prefix R> := <radix R> <exactness>
 *             | <exactness> <radix R>
 * <real R> := <sign> <ureal R>
 * <ureal R> := <uinteger R> | <decimal R>
 * <uinteger R> := <digit R>+
 * <decimal R> := <uinteger R>
 *              | . <digit R>+
 *              | <digit R>+ . <digit R>*
 * 
 * 
 * available radices are #b #o #d #x for binary (2), octal (8), decimal (10) and
 * hexadecimal (16) respectively. note that the decimal radix is optional.
 * digits go according to the bases, so valid binary digits are 0 and 1, valid
 * octal digits are 0-7, decimal 0-9 and hex 0-9 and A-F (case-insensitive)
 * 
 * TODO: add support for <suffix>
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
	if (utf8cmp(buf, str) == 0) {\
		_eu_makechar(out, c);\
		return EU_RESULT_OK;\
	}
#define CHAR_BUF_SIZE 128

/* reads a character literal
 * <character>s are either '#\<any character>', '#\<character name>' or
 * '#\x<unicode hex value>'.
 * 
 * only R7RS-required character names are currently implemented.
 */
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

eu_result pread_bytevector(parser* p, eu_value* out) {
	eu_result res;
	eu_integer vsize;
	eu_value temp;
	void *buf, *next;
	size_t size, remaining;
	eu_bvector* vec;

	/* match the '#u8(' */
	_checkreturn(res, pmatch(p, 'u'));
	_checkreturn(res, padvance(p));
	_checkreturn(res, pmatch(p, '8'));
	_checkreturn(res, padvance(p));
	_checkreturn(res, pmatch(p, CLPAR));
	_checkreturn(res, padvance(p));

	/* initialize auxilary buffer */
	_checkreturn(res, gbuf_init(p, &buf, &next, &size));
	remaining = size;

	/* read bytevector elements */
	while (p->current != CRPAR && !iseof(p->current)) {
		/* skip any intertoken space */
		_checkreturn(res, pskip_itspace(p));

		/* assert that the next token is a number */
		if (!(isdecimaldigit(p->current) ||
			(issign(p->current) && isdecimaldigit(p->peek)) ||
			(isdot(p->current) && isdecimaldigit(p->peek)))) {
			seterrorf(p, "Expected a number literal but got character sequence "
				"'%c%c'.", (char)p->current, (char)p->peek);
			return EU_RESULT_ERROR;
		}

		/* read a number */
		_checkreturn(res, pread_number(p, &temp));

		/* assert the number is a valid byte (exact and in [0,256[) */
		if (temp.type & EU_NUMBER_REAL ||
			temp.value.i >= 256 || temp.value.i < 0) {
			seterror(p, "Bytevector values need to be an integer between 0 and "
				"255.");
			return EU_RESULT_ERROR;
		}

		/* append the byte to the buffer */
		_checkreturn(res, gbuf_append_byte(p, &buf, &next, &size, &remaining,
			temp.value.i));
	}

	/* turn the buf into a bytevector */
	vec = eubvector_new(p->s, size - remaining, cast(eu_byte*, buf));
	if (vec == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* set the return value */
	_eu_makebvector(out, vec);

	/* terminate the buffer */
	_checkreturn(res, gbuf_terminate(p, &buf));

	return EU_RESULT_OK;
}

/* reads a token that begins with a '#', those can be booleans (#t), numbers
 * (#e#x1F), characters, vectors (#(a b c)), bytevectors (#u8(a b c)) */
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
	else if (p->peek == 'u')
		return pread_bytevector(p, out);
	else if (p->peek == CLPAR)
		return pread_vector(p, out);

	seterror(p, "Expected a boolean, number, ..., but nothing matched.");
	return EU_RESULT_ERROR;
}

/* skips a single-line comment */
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
	_checkreturn(res, pread_datum(p, &out));

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

/* reads the inline (in-string?) escaped hex character value, something akin to
 * \xABC */
eu_result pread_escaped_hex_char(parser* p, int* out) {
	int c = 0, v;
	eu_result res;

	/* consume the x */
	_checkreturn(res, padvance(p));

	/* read characters up to a semicolon */
	while (p->current != CSCOLON) {
		/* check for invalid digits */
		if (!ishexdigit(p->current)) {
			seterrorf(p,
				"Invalid character '%c' in unicode inline hex character.",
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

	/* consume the semicolon */
	*out = unicodetoutf8(c);
	return EU_RESULT_OK;
}

/* skips an (inlined/instring) escaped whitespace-newline-whitespace sequence */
eu_result pskip_escaped_whitespace(parser* p) {
	eu_result res;

	/* skip whitespace */
	while (iswhitespace(p->current)) {
		_checkreturn(res, padvance(p));
	}

	/* make sure that there is a line ending */
	if (!islineending(p->current)) {
		seterrorf(p, "Expected a line ending after \\, but got %c.",
			p->current);
		return EU_RESULT_ERROR;
	}
	/* in case the newline was a \r\n consume the \r */
	if (p->current == '\r' && p->peek == '\n') {
		padvance(p);
	}
	_checkreturn(res, padvance(p));

	/* skip additional whitespace */
	while (iswhitespace(p->current)) {
		_checkreturn(res, padvance(p));
	}

	return EU_RESULT_OK;
}

/* reads an inline (in-string) escaped character */
eu_result pread_escaped_char(parser* p, int* out) {
	eu_result res;
	int c, v;

	/* consume the \ */
	_checkreturn(res, padvance(p));
	c = tolower(p->current);

	switch (c) {
		/* special characters */
		case 'a': c = '\a'; break;
		case 'b': c = '\b'; break;
		case 't': c = '\t'; break;
		case 'n': c = '\n'; break;
		case 'r': c = '\r'; break;
		case '"': c = '"'; break;
		case '\\': c = '\\'; break;
		case '|': c = '|'; break;

		/* inline hex escape */
		case 'x':
			return pread_escaped_hex_char(p, out);

		/* invalid */
		default:
			seterrorf(p, "Invalid escape character %c.", p->current);
			return EU_RESULT_OK;
	}

	*out = c;
	return EU_RESULT_OK;
}

/* reads a string literal from the port */
eu_result pread_string(parser* p, eu_value* out) {
	eu_result res;
	int c, v, should_advance;
	void *buf, *next;
	size_t size, remaining;
	eu_string* str;

	/* make sure the first character is a " */
	_checkreturn(res, pmatch(p, CDQUOT));
	_checkreturn(res, padvance(p)); /* consume it */

	/* initialize buffer pointers */
	_checkreturn(res, gbuf_init(p, &buf, &next, &size));
	remaining = size;

	/* read until end */
	while (p->current != CDQUOT && p->current != EOF) {
		/* handle special chars */
		if (p->current == CBSLASH) {
			if (iswhitespace(p->peek) || islineending(p->peek)) {
				_checkreturn(res, pskip_escaped_whitespace(p));
				/* start a new iteration of the loop after the spaces */
				continue;
			} else {
				_checkreturn(res, pread_escaped_char(p, &c));
			}
		} else { /* any other character */
			c = p->current;
		}

		/* append the character to the buffer */
		_checkreturn(res, gbuf_append(p, &buf, &next, &size, &remaining, c));

		/* advance to the next character in the string */
		_checkreturn(res, padvance(p));
	}

	/* if the string ended abruptly, we need to return an error */
	if (p->current == EOF) {
		seterror(p, "String literal ended abruptly.");
		return EU_RESULT_ERROR;
	}

	/* the string is valid, create the object */
	str = eustring_new(p->s, buf);
	if (str == NULL)
		return EU_RESULT_BAD_ALLOC;

	out->type = EU_TYPE_STRING | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eustring_to_obj(str);

	/* terminate the aux buffer, releasing memory if applicable */
	gbuf_terminate(p, &buf);

	return EU_RESULT_OK;
}

/* reads a
 * <symbol> := <vertical line> <symbol element>* <vertical line>
 */
eu_result pread_vline_symbol(parser* p, eu_value* out) {
	eu_result res;
	void *next, *buf;
	size_t size, remaining;
	eu_symbol* sym;
	int c;

	_checkreturn(res, pmatch(p, CVLINE));
	_checkreturn(res, padvance(p));

	_checkreturn(res, gbuf_init(p, &buf, &next, &size));
	remaining = size;

	while (!isvline(p->current) && !iseof(p->current)) {
		/* handle special chars */
		if (p->current == CBSLASH) {
			_checkreturn(res, pread_escaped_char(p, &c));
		} else { /* any other character */
			c = p->current;
		}

		/* append the character to the auxilary buffer */
		_checkreturn(res, gbuf_append(p, &buf, &next, &size, &remaining, c));

		/* advance to the next character */
		_checkreturn(res, padvance(p));
	}

	/* create the symbol object */
	sym = eusymbol_new(p->s, buf);
	if (sym == NULL)
		return EU_RESULT_BAD_ALLOC;

	/* set the output value */
	out->type = EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eusymbol_to_obj(sym);

	return EU_RESULT_OK;
}

eu_result pread_insub_symbol(parser* p, eu_value* out) {
	eu_result res;
	void *next, *buf;
	size_t size, remaining;
	eu_symbol* sym;

	/* initialize the auxilary buffer */
	_checkreturn(res, gbuf_init(p, &buf, &next, &size));
	remaining = size;

	do {
		/* append the current character to the symbol */
		_checkreturn(res, gbuf_append(p, &buf, &next, &size, &remaining,
			p->current));

		/* advance a character */
		_checkreturn(res, padvance(p));
	} while (issubsequent(p->current));

	/* create the symbol */
	sym = eusymbol_new(p->s, buf);
	if (sym == NULL)
		return EU_RESULT_BAD_ALLOC;

	out->type = EU_TYPE_SYMBOL | EU_TYPEFLAG_COLLECTABLE;
	out->value.object = _eusymbol_to_obj(sym);

	_checkreturn(res, gbuf_terminate(p, &buf));

	return EU_RESULT_OK;
}

/* reads a <symbol> */
eu_result pread_symbol(parser* p, eu_value* out) {

	if (isvline(p->current)) { /* vertical line symbols */
		return pread_vline_symbol(p, out);
	} else if (isinitial(p->current) ||
		(issign(p->current) && isdelimiter(p->peek)) ||
		(issign(p->current) && issignsubsequent(p->peek)) ||
		(issign(p->current) && isdot(p->peek)) ||
		(isdot(p->current) && isdotsubsequent(p->peek))) {
		return pread_insub_symbol(p, out);
	} else {
		seterrorf(p, "Parser in inconsistent state. Tried to read an identifier"
			" and got '%c'.", (char)p->current);
		return EU_RESULT_ERROR;
	}

	return EU_RESULT_OK;
}

/* reads a <list>
 * <list> := ( <datum>* )
 *         | ( <datum>+ . <datum> )
 */
eu_result pread_list(parser* p, eu_value* out) {
	eu_result res;
	int has_datum = 0;
	int has_dot = 0;
	eu_pair *pair, *nextpair;
	eu_value *slot;

	/* consume left parenthesis */
	_checkreturn(res, padvance(p));

	/* skip whitespaces and check if the list is empty */
	_checkreturn(res, pskip_itspace(p));
	if (isrpar(p->current)) {
		*out = _null;
		return EU_RESULT_OK;
	}

	/* setup the first pair */
	pair = eupair_new(p->s, &_null, &_null);
	if (pair == NULL)
		return EU_RESULT_BAD_ALLOC;
	slot = _eupair_head(pair);

	/* place the first pair in out */
	out->type = EU_TYPEFLAG_COLLECTABLE | EU_TYPE_PAIR;
	out->value.object = _eupair_to_obj(pair);

	while (!isrpar(p->current) && !iseof(p->current)) {
		/* check for dotted pair */
		if (isdot(p->current) && isitspace(p->peek)) {
			if (!has_datum) {
				seterror(p, "Invalid dot found in list. There must be at least"
					" one <datum> before a dot.");
				return EU_RESULT_ERROR;
			}
			if (has_dot) {
				seterror(p, "Only a single dot is permitted in a dotted list.");
				return EU_RESULT_ERROR;
			}

			/* mark dotted pair */
			has_dot = 1;

			/* setup the slot for the next value */
			slot = _eupair_tail(pair);

			/* consume the dot, skip spaces and restart the loop */
			_checkreturn(res, padvance(p));
			_checkreturn(res, pskip_itspace(p));
			continue;
		}

		/* try reading a datum */
		_checkreturn(res, pread_datum(p, slot));
		has_datum = 1;

		if (has_dot) {
			/* only one last datum is allowed after a dot, so we need to 
			 * terminate the list here. */
			/* we skip whitespaces in order to make sure that if this list is
			 * valid, p->current after the end of the loop is a closing par */
			_checkreturn(res, pskip_itspace(p));
			break;
		}

		/* skip intertoken spaces in order to check if we are at end of list */
		_checkreturn(res, pskip_itspace(p));

		/* if this is not the end of the list, we need to allocate a new pair
		 * to hold the next element in it head and place it in the current pair's
		 * tail */
		if (!isrpar(p->current) && !isdot(p->current)) {
			/* allocate the next pair */
			nextpair = eupair_new(p->s, &_null, &_null);
			if (nextpair == NULL)
				return EU_RESULT_BAD_ALLOC;
			/* place the next pair in the last pair's tail */
			_eupair_tail(pair)->type = EU_TYPEFLAG_COLLECTABLE | EU_TYPE_PAIR;
			_eupair_tail(pair)->value.object = _eupair_to_obj(nextpair);
			/* set the next slot */
			slot = _eupair_head(nextpair);
			/* update pairs */
			pair = nextpair;
		}
	}

	if (!isrpar(p->current)) { /* incomplete or incorrect list */
		if (iseof(p->current)) { /* incomplete */
			seterror(p, "Unexpected incomplete list. Expected a datum, got EOF.");
			return EU_RESULT_ERROR;
		}

		if (has_dot) { /* erroneous dot */
			seterror(p, "Expected end of dotted list, but found something else."
				" Do you have more than one <datum> after a dot?");
			return EU_RESULT_ERROR;
		}

		/* inconsistent */
		seterrorf(p, "Expected end of list (')'), but got character '%c'.",
			(char)p->current);
		return EU_RESULT_ERROR;
	}

	return EU_RESULT_OK;
}

/* this reads a <datum> */
eu_result pread_datum(parser* p, eu_value* out) {
	eu_result res;

	/* check and skip inter token spaces (aka <atmosphere>) */
	if (isitspace(p->current)) {
		_checkreturn(res, pskip_itspace(p));
	}

	/* check terminals that start with # */
	if (p->current == CHASH) {
		return pread_hash(p, out);
	} else if (isdecimaldigit(p->current) ||
		(issign(p->current) && isdecimaldigit(p->peek)) ||
		(isdot(p->current) && isdecimaldigit(p->peek))) {
		return pread_number(p, out);
	} else if (p->current == CDQUOT) {
		return pread_string(p, out);
	} else if (isidentifier(p->current) ||
		(isdot(p->current) && isdotsubsequent(p->peek))) {
		return pread_symbol(p, out);
	} else if (islpar(p->current)) {
		return pread_list(p, out);
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
	if ((res = pread_datum(&p, out))) {
		out->type = EU_TYPE_ERROR | EU_TYPEFLAG_COLLECTABLE;
		out->value.object = _euerror_to_obj(p.error);
		return res;
	}

	return EU_RESULT_OK;
}
