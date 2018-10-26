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

void printobj(europa* s, eu_object* obj) {

	switch (_euobj_type(obj)) {
	case EU_TYPE_SYMBOL:
		printf("%s", _eusymbol_text(_euobj_to_symbol(obj)));
		break;
	case EU_TYPE_STRING:
		printf("\"%s\"", _eustring_text(_euobj_to_string(obj)));
		break;
	case EU_TYPE_ERROR:
		printf("#<error: %s>", _euerror_message(_euobj_to_error(obj)));
		break;

	case EU_TYPE_PAIR: {
		eu_value* lv, v;

		_eu_makepair(&v, _euobj_to_pair(obj));

		printf("(");

		for (lv = &v;
			!_euvalue_is_null(lv) && _euvalue_is_type(lv, EU_TYPE_PAIR);
			lv = _eupair_tail(_euvalue_to_pair(lv))) {
			if (lv != &v)
				printf(" ");
			printv(s, _eupair_head(_euvalue_to_pair(lv)));
		}

		if (!_euvalue_is_null(lv)) {
			printf(" . ");
			printv(s, lv);
		}

		printf(")");
		break;
	}
	case EU_TYPE_VECTOR: {
		int i;
		eu_vector* vec = _euobj_to_vector(obj);

		printf("#(");
		for (i = 0; i < _euvector_length(vec); i++) {
			printv(s, _euvector_ref(vec, i));
			printf(" ");
		}
		printf(")");
		break;
	}
	case EU_TYPE_BYTEVECTOR: {
		int i;
		eu_bvector* bv = _euobj_to_bvector(obj);

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
		printf("#<table: %p>{", obj);

		int i, first = 1;
		eu_table* t = _euobj_to_table(obj);
		for (i = 0; i < _eutable_size(t); i++) {
			if (!_euvalue_is_null(_eutnode_key(_eutable_node(t, i)))) {
				if (!first) {
					printf(" ");
				}
				first = 0;

				printf("[");
				printv(s, _eutnode_key(_eutable_node(t, i)));
				printf(" ");
				printv(s, _eutnode_value(_eutable_node(t, i)));
				printf("]");
			}
		}

		printf("}");
		break;
	case EU_TYPE_PORT:
		printf("#<port: %p>", obj);
		break;

	case EU_TYPE_CLOSURE:
		printf("#<%sclosure: %p>", _euobj_to_closure(obj)->cf ? "c" : "", obj);
		break;
	case EU_TYPE_CONTINUATION:
		printf("#<%scontinuation: %p>", _euobj_to_cont(obj)->cl->cf ? "c" : "", obj);
		break;
	case EU_TYPE_PROTO:
		printf("#<prototype: %p>", obj);
		break;

	case EU_TYPE_CPOINTER:
		printf("#<c-pointer: %p>", obj);
		break;
	case EU_TYPE_USERDATA:
		printf("#<userdata: %p>", obj);
		break;
	default:
		printf("UNKNOWN TYPE");
		break;
	}

}

void printv(europa* s, eu_value* v) {
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

	default:
		if (_euvalue_is_collectable(v)) {
			printobj(s, _euvalue_to_obj(v));
		} else {
			printf("UNKNOWN TYPE");
		}
		break;
	}
}

void printvln(europa* s, eu_value* v) {
	printv(s, v);
	printf("\n");
}

void printobjln(europa* s, eu_object* obj) {
	printobj(s, obj);
	printf("\n");
}

void disas_inst(europa* s, eu_proto* proto, eu_instruction inst) {
	static const char* opc_names[] = {
		"nop", "refer", "const", "close", "test", "jump", "assign", "argument",
		"conti", "apply", "return", "frame", "halt"
	};
	static const int opc_types[] = {
		0, 1, 1, 3, 2, 2, 1, 0, 2, 0, 0, 0, 0,
	};
	#define opc_part(i) ((i >> 24) & 0xFF)
	#define val_part(i) (i & 0xFFFFFF)
	#define off_part(i) (val_part(i) - (0xFFFFFF >> 1))

	int opindex = opc_part(inst);

	if (opindex > EU_OP_HALT) {
		printf("\tUNKNOWN INSTRUCTION\n");
		return;
	}

	printf("%s ", opc_names[opindex]);

	switch (opc_types[opindex]) {
	case 1:
		printf(" [%d]", val_part(inst));
		if (proto) {
			printf("\t; ");
			printv(s, &(proto->constants[val_part(inst)]));
		}
		break;
	case 2:
		printf(" %d", off_part(inst));
		break;
	case 3:
		printf(" <%d>", val_part(inst));
		if (proto) {
			printf("\t; #<proto: %p>", proto->subprotos[val_part(inst)]);
		}
		break;
	case 4:
		printf(" %d", val_part(inst));
		break;
	case 0:
		break;
	default:
		break;
	}

	printf("\n");
}

void disas_proto(europa* s, eu_proto* proto) {
	printf("Prototype #%p\n", proto);
	printf("source: ");
	printvln(s, &proto->source);
	printf("formals: ");
	printvln(s, &proto->formals);
	printf("code:\n");

	int i;
	for (i = 0; i < proto->code_length; i++) {
		printf("\t");
		disas_inst(s, proto, proto->code[i]);
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