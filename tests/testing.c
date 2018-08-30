#include <stdio.h>
#include <stdlib.h>

#include "europa.h"
#include "eu_error.h"
#include "ports/eu_fport.h"
#include "helpers.h"

int main(void) {
	europa* s;
	eu_fport* port;

	s = bootstrap_default_instance();
	if (!s) {
		printf("state is null\n");
		exit(EXIT_FAILURE);
	}

	port = eufport_open(s, EU_PORT_FLAG_INPUT, "reader_test.txt");
	if (!port) {
		printf("port is null\n");
		exit(EXIT_FAILURE);
	}

	eu_value out;
	eu_result res;
	res = euport_read(s, _eufport_to_port(port), &out);
	if (res) {
		printf("result of read is %d\n", res);
	}

	printf("out.type = %d\n", _euvalue_type(&out));
	if (_euvalue_type(&out) == EU_TYPE_ERROR) {
		printf("Error message: %s\n", (char*)euerror_message(_euobj_to_error(out.value.object)));
	}

	printf("out.value.boolean = %d\n", out.value.boolean);

	return 0;
}