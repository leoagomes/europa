#include "europa/ccont.h"

#include "europa/error.h"

int eucc_frame(europa* s) {
    struct europa_continuation* cont;

    /* create the continuation */
    cont = eucont_new(s,
        s->previous,
        s->environment,
        &(s->rib),
        s->rib_last_position,
        s->current_closure,
        s->program_counter
    );
    if (cont == NULL) {
        _eu_checkreturn(eu_set_error(s, EU_ERROR_NONE, NULL,
            "Could not create continuation."));
        return EU_RESULT_ERROR;
    }

    /* update state */
    s->previous = cont;
    s->rib = _null;
    s->rib_last_position = &s->rib;

    return EU_RESULT_OK;
}

int eucc_define_cclosure(
    europa* s,
    struct europa_table* t,
    struct europa_table* env,
    void* text,
    europa_c_callback cf
) {
    struct europa_closure* cl;
    struct europa_value* tv, closure;

    /* check parameters */
    if (!s || !t || !cf)
        return EU_RESULT_NULL_ARGUMENT;

    /* create a closure for the cfunction */
    cl = eucl_new(s, cf, NULL, env);
    if (cl == NULL)
        return EU_RESULT_BAD_ALLOC;

    /* set the value up */
    _eu_makeclosure(&closure, cl);
    tv = &closure;

    /* define this closure into the table */
    return eutable_define_symbol(s, t, text, &tv);
}
