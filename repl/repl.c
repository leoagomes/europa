#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linenoise.h"
#include "europa.h"

int main(int argc, char* argv) {
	europa* s;
	eu_result res;

	/* create an Europa instance */
	s = eu_new(eutil_stdlib_realloclike, NULL, NULL, &res);
	if (s == NULL) {
		fprintf(stderr, "Error creating Europa instance (%d).\n", res);
		return EXIT_FAILURE;
	}

	/* register standard library functions */
	if ((res = eutil_register_standard_library(s))) {
		fprintf(stderr, "Could not register standard library functions.\n");
		return EXIT_FAILURE;
	}

	/* set standard ports */
	if ((res = eutil_set_standard_ports(s))) {
		fprintf(stderr, "Could not set standard ports.\n");
	}

	/* configure linenoise */
	linenoiseHistorySetMaxLen(1024);

	/* print out welcome message */
	printf("Welcome to the Europa REPL. Type ',quit' to quit.\n");

	/* do the REPL */
	char* line;
	eu_value output;

	while ((line = linenoise("> ")) != NULL) {
		/* check if it is a command */
		if (!strncmp(line, ",quit", sizeof(",quit"))) {
			break;
		}

		/* add it to the history */
		linenoiseHistoryAdd(line);

		/* execute the string */
		if ((res = eu_do_string(s, line, &output))) {
			fprintf(stderr, "Error (%d) executing line '%s': %s\n", res, line,
				_euerror_message(_eu_err(s)));
			eu_recover(s, NULL);
			continue;
		}

		/* print the result out to stdout */
		if ((res = euport_write(s, s->output_port, &output))) {
			fprintf(stderr, "Error (%d) writing output to output port: %s\n",
				res, _euerror_message(_eu_err(s)));
			eu_recover(s, NULL);
			continue;
		}
		putchar('\n');
	}

	/* terminate the Europa state */
	res = eu_terminate(s);
	return res;
}
