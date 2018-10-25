/** Auxilary functions for the testing environment.
 * 
 * @file helpers.c
 * @author Leonardo G.
 * @ingroup tests
 */
#include "helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include "munit.h"

#include "eu_number.h"
#include "eu_character.h"
#include "eu_symbol.h"
#include "eu_pair.h"
#include "eu_vector.h"
#include "eu_bytevector.h"
#include "eu_string.h"
#include "eu_error.h"
#include "eu_rt.h"

/** Realloc-like function to be used by the garbage collector.
 * 
 * @todo Use a function provided by an auxilary library (to implement).
 */
void* rlike(void* ud, void* ptr, unsigned long long size) {
	return realloc(ptr, size);
}

europa* bootstrap_default_instance(void) {
	europa* s;
	eu_result err;

	/* we need to allocate memory for the state, because it tries to leave
	 * memory management to the GC
	 * 
	 * TODO: maybe sometime use something provided by an auxilary library */
	s = eu_new(rlike, NULL, NULL, &err);
	return s;
}

void terminate_default_instance(europa* s) {
	eu_terminate(s);
}

void print_value(europa* s, eu_value* v) {
	switch (_euvalue_type(v)) {
	case EU_TYPE_NULL:
		printf("'()");
		break;
	case EU_TYPE_BOOLEAN:
		printf("#%s", _euvalue_to_bool(v) ? "true" : "false");
		break;
	case EU_TYPE_NUMBER:
		if (_eunum_is_exact(v)) {
			printf("%d", _eunum_i(v));
		} else {
			printf("%lf", _eunum_r(v));
		}
		break;
	case EU_TYPE_CHARACTER:
		printf("#\\%c", _euvalue_to_char(v));
		break;
	case EU_TYPE_EOF:
		printf("#<EOF>");
		break;

	case EU_TYPE_SYMBOL:
		printf("%s", _eusymbol_text(_euvalue_to_symbol(v)));
		break;
	case EU_TYPE_STRING:
		printf("\"%s\"", _eustring_text(_euvalue_to_string(v)));
		break;
	case EU_TYPE_ERROR:
		printf("#<error: %s>", _euerror_message(_euvalue_to_error(v)));
		break;

	case EU_TYPE_PAIR: {
		eu_value* lv;

		printf("(");

		for (lv = v;
			!_euvalue_is_null(lv) && _euvalue_is_type(lv, EU_TYPE_PAIR);
			lv = _eupair_tail(_euvalue_to_pair(lv))) {
			if (lv != v)
				printf(" ");
			print_value(s, _eupair_head(_euvalue_to_pair(lv)));
		}

		if (!_euvalue_is_null(lv)) {
			printf(" . ");
			print_value(s, lv);
		}

		printf(")");
		break;
	}
	case EU_TYPE_VECTOR: {
		int i;
		eu_vector* vec = _euvalue_to_vector(v);

		printf("#(");
		for (i = 0; i < _euvector_length(vec); i++) {
			print_value(s, _euvector_ref(vec, i));
			printf(" ");
		}
		printf(")");
		break;
	}
	case EU_TYPE_BYTEVECTOR: {
		int i;
		eu_bvector* bv = _euvalue_to_bvector(v);

		printf("#u8(");
		for (i = 0; i < _eubvector_length(bv); i++) {
			printf("%d", _eubvector_ref(bv, i));

			if (i != _eubvector_length(bv) - 1)
				printf(" ");
		}
		printf(")");
		break;
	}
	case EU_TYPE_TABLE:
		printf("#<table: %p>", _euvalue_to_obj(v));
		break;
	case EU_TYPE_PORT:
		printf("#<port: %p>", _euvalue_to_obj(v));
		break;

	case EU_TYPE_CLOSURE:
		printf("#<%sclosure>", _euvalue_to_closure(v)->cf ? "c" : "");
		break;
	case EU_TYPE_CONTINUATION:
		printf("#<%scontinuation>", _euvalue_to_cont(v)->cl->cf ? "c" : "");
		break;
	case EU_TYPE_PROTO:
		printf("#<prototype>");
		break;

	case EU_TYPE_CPOINTER:
		printf("#<c-pointer: %p>", _euvalue_to_obj(v));
		break;
	case EU_TYPE_USERDATA:
		printf("#<userdata: %p>", _euvalue_to_obj(v));
		break;
	default:
		printf("UNKNOWN TYPE");
		break;
	}
}

void print_valueln(europa* s, eu_value* v) {
	print_value(s, v);
	printf("\n");
}

void disas_proto(europa* s, eu_proto* proto) {
	printf("Prototype #%p\n", proto);
	printf("source: ");
	print_valueln(s, &proto->source);
	printf("formals: ");
	print_valueln(s, &proto->formals);
	printf("code:\n");

	int i;
	static const char* opc_names[] = {
		"nop", "refer", "const", "close", "test", "jump", "assign", "argument",
		"conti", "apply", "return", "frame", "halt"
	};
	static const int opc_types[] = {
		0, 1, 1, 3, 2, 2, 1, 0, 0, 0, 0, 0, 0,
	};
	#define opc_part(i) ((i >> 24) & 0xFF)
	#define val_part(i) (i & 0xFFFFFF)
	#define off_part(i) (val_part(i) - (0xFFFFFF >> 1))

	for (i = 0; i < proto->code_length; i++) {
		int opindex = opc_part(proto->code[i]);

		if (opindex > EU_OP_HALT) {
			printf("\tUNKNOWN INSTRUCTION\n");
			continue;
		}

		printf("\t%s ", opc_names[opindex]);
		switch (opc_types[opindex]) {
		case 1:
			printf(" [%d]\t; ", val_part(proto->code[i]));
			print_valueln(s, &(proto->constants[val_part(proto->code[i])]));
			break;
		case 2:
			printf(" %d\n", off_part(proto->code[i]));
			break;
		case 3:
			printf(" <%d>\t; #<proto: %p>\n", val_part(proto->code[i]),
				proto->subprotos[val_part(proto->code[i])]);
			break;
		case 0:
			printf("\n");
			break;
		default:
			break;
		}
	}

	printf("subprototypes: [");
	for (i = 0; i < proto->subprotoc; i++) {
		printf("\n");
		disas_proto(s, proto->subprotos[i]);
	}
	printf("]\n");
}

void disas_closure(europa* s, eu_closure* cl) {
	if (cl->cf) {
		printf("Closure is C closure.\n");
		return;
	}
	disas_proto(s, cl->proto);
}