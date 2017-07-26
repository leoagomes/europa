#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <math.h>

#include "utf8.h"

#define CHAR_EOS 0

#define utf8_string

#define TRUE (1 == 1)
#define FALSE (1 == 0)

typedef struct snode snode;
typedef struct number num;
typedef struct procdata procdata;
typedef struct environment environment;
typedef struct parser_state parser_state;
typedef snode* (*native_proc)(snode*, environment*);

#define PROCFLAG_NATIVE (1 << 0)
#define PROCFLAG_NO_EVAL_ARGS (1 << 1)

struct procdata {
    int flags;

    union {
        native_proc native;
        snode* lang;
    } data;
};

typedef enum number_type {
    NTYPE_INTEGER,
    NTYPE_DOUBLE
} num_type;

struct number {
    num_type type;
    union {
        int ivalue;
        double dvalue;
    } data;
};

typedef enum sn_type {
    ATYPE_CELL,
    ATYPE_SYMBOL,
    ATYPE_NUMBER,
    ATYPE_BOOLEAN,
    ATYPE_STRING,
    ATYPE_PROCEDURE,
    ATYPE_ERROR
} sn_type;

struct snode {
    sn_type type;
    union {
        void* symbol;
        void* string;
        num nvalue;
        procdata* proc;
        int boolvalue;
    } data;

    int quotes;
    snode *car, *cdr;
};

struct parser_state {
    void* buffer;
    int32_t current_cp;
    void* current_cp_pos;
    void* next_cp;

    int line, col;

    int has_error;
    int32_t expected_cp;
};

struct env_hl_cell {
    int32_t hash;
    void* key;
    snode* value;

    struct env_hl_cell* next;
};

struct environment {
    environment* up_env;
    struct env_hl_cell* list;
};

#define procdata_is_native(p) (((p)->flags & PROCFLAG_NATIVE) != 0)
#define procdata_is_no_eval(p) (((p)->flags & PROCFLAG_NO_EVAL_ARGS) != 0)

procdata* procdata_new(int flags, snode* lang, native_proc native);

void parser_state_init(parser_state* parser, const void* input) {
    parser->buffer = malloc(utf8size(input));
    utf8cpy(parser->buffer, input);

    parser->next_cp = parser->buffer;
    parser->current_cp = -1;
    parser->current_cp_pos = NULL;

    parser->line = parser->col = 0;

    parser->has_error = FALSE;
    parser->expected_cp = -1;
}

void parser_state_terminate(parser_state* p) {
    if (p == NULL)
        return;

    free(p->buffer);
}

int32_t parser_next(parser_state* p) {
    int32_t codepoint;

    if (p->current_cp == CHAR_EOS)
        return CHAR_EOS;

    p->current_cp_pos = p->next_cp;
    p->next_cp = utf8codepoint(p->next_cp, &codepoint);
    p->current_cp = codepoint;
    return codepoint;
}

void parser_skip_whitespace(parser_state* p) {
    while(isspace(p->current_cp)) {
        if (p->current_cp == (int32_t)'\n')
            p->line++;
        parser_next(p);
    }
}

void parser_walk_until(parser_state* p, int32_t codepoint) {
    while (p->current_cp != codepoint && p->current_cp != CHAR_EOS)
        parser_next(p);
}

void parser_skip_comments(parser_state* p) {
    if (p->current_cp != (int32_t)';')
        return;

    parser_walk_until(p, '\n');
    parser_next(p);
}

void parser_skip_wc(parser_state* p) {
    parser_skip_whitespace(p);

    while (p->current_cp == (int32_t)';') {
        parser_skip_comments(p);
        parser_skip_whitespace(p);
    }
}

void parser_enforce_current(parser_state* p, int32_t cp) {
    if (p->current_cp != cp) {
        p->has_error = TRUE;
        p->expected_cp = cp;
    }
}

utf8_string void* parser_get_til_whitespace(parser_state* p) {
    void* buffer = NULL;
    size_t blen = 1;
    int cpcount = 0;
    int32_t current_cp;
    void* next_cp;

    next_cp = utf8codepoint(p->current_cp_pos, &current_cp);
    while (!isspace(current_cp) && current_cp != CHAR_EOS && current_cp != (int32_t)')' && current_cp != (int32_t)';') {
        blen += utf8codepointsize(current_cp);
        cpcount += 1;
        next_cp = utf8codepoint(next_cp, &current_cp);
    }

    buffer = calloc(blen, 1);
    utf8ncpy(buffer, p->current_cp_pos, blen - 1);

    while (!isspace(p->current_cp) && p->current_cp != CHAR_EOS && p->current_cp != (int32_t)')' && p->current_cp != (int32_t)';') // now consume the parser's characters
        parser_next(p);
    // leave the space in there as we should call "skip_wc" after this func

    return buffer;
}

utf8_string void* parser_get_til_dquote(parser_state* p) {
    void* buffer = NULL;
    size_t blen = 1;
    int cpcount = 0;
    int32_t current_cp;
    void* next_cp;

    next_cp = utf8codepoint(p->current_cp_pos, &current_cp);
    while (current_cp != '"' && current_cp != CHAR_EOS) {
        blen += utf8codepointsize(current_cp);
        cpcount += 1;
        next_cp = utf8codepoint(next_cp, &current_cp);
    }

    buffer = calloc(blen, 1);
    utf8ncpy(buffer, p->current_cp_pos, blen - 1);

    while (p->current_cp != '"' && p->current_cp != CHAR_EOS) // now consume the parser's characters
        parser_next(p);
    parser_next(p); // consume second '"'

    return buffer;
}

snode* snode_new(int type, snode* car, snode* cdr) {
    snode* s;
    s = (snode*)calloc(1, sizeof(*s));

    s->type = type;
    s->car = car; s->cdr = cdr;

    return s;
}

void snode_release(snode** node) { //should be a simple tree trasversal with freeing algorithm
    snode* tmp;

    if (node == NULL || *node == NULL)
        return;

    tmp = *node;

    snode_release(&(tmp->car));
    snode_release(&(tmp->cdr));

    if (tmp->type == ATYPE_SYMBOL) {
        free(tmp->data.symbol);
    } else if (tmp->type == ATYPE_STRING) {
        free(tmp->data.string);
    } else if (tmp->type == ATYPE_PROCEDURE && !procdata_is_native(tmp->data.proc)) {
        snode_release(&(tmp->data.proc->data.lang)); // should implement a GC at some point and then put this on the GC
    }

    *node = NULL;
    free(tmp);
}

num parser_parse_num(utf8_string void* input, int* ok) {
    int ic = 0, dc = 0;
    int ival = 0;
    double dval = 0.0;
    num ret;

    int32_t cp;
    void* next_cp;

    *ok = TRUE;

    next_cp = utf8codepoint(input, &cp);

    // numbers need to be ASCII

    // get integer part
    while (isdigit(cp)) {
        ival *= 10;
        ival += cp - (int32_t)'0';
        ic++;

        next_cp = utf8codepoint(next_cp, &cp);
    }

    if (cp == (int32_t)'.') {
        next_cp = utf8codepoint(next_cp, &cp);
        while (isdigit(cp)) {
            dval *= 10;
            dval += (cp - (int32_t)'0');
            dc++;

            next_cp = utf8codepoint(next_cp, &cp);
        }
        dval = dval / pow(10, (double)dc);
    }

    if (dc) {
        ret.type = NTYPE_DOUBLE;
        ret.data.dvalue = ival + dval;
    } else {
        ret.type = NTYPE_INTEGER;
        ret.data.ivalue = ival;
    }

    if (ic == 0 || dc == 0) {
        *ok = FALSE;
    }

    return ret;
}

void print_parser_state(parser_state* p) {
    printf("line: %d; buff = %s; next %s;\n", p->line, p->buffer, p->next_cp);
    putc(p->current_cp, stdout);
}

void print_expr(snode* expr) {
    if (!expr) {
        printf("(null)");
        return;
    }

    switch(expr->type) {
        case ATYPE_NUMBER:
            if (expr->data.nvalue.type == NTYPE_INTEGER)
                printf("i%d", expr->data.nvalue.data.ivalue);
            else
                printf("d%lf", expr->data.nvalue.data.dvalue);
            break;
        case ATYPE_STRING:
            printf("\"%s\"", expr->data.string);
            break;
        case ATYPE_SYMBOL:
            printf("%s", expr->data.symbol);
            break;
        case ATYPE_CELL:
            printf("(");
            print_expr(expr->car);
            printf(" . ");
            print_expr(expr->cdr);
            printf(")");
            break;
        case ATYPE_ERROR:
            printf("ERROR: %s", expr->data.string);
            break;
        case ATYPE_PROCEDURE:
            printf("#<procedure: ");
            if (procdata_is_native(expr->data.proc)) {
                printf("%x>", expr->data.proc->data.native);
            } else {
                printf("lang>");
            }
            break;
        case ATYPE_BOOLEAN:
            printf("#%c", (expr->data.boolvalue ? 't' : 'f'));
            break;
        default:
            break;
    }
}

snode* parser_parse_sexp_element(parser_state* p) {
    int32_t cp;
    snode* root;
    snode** c_cell;
    void* tmp_buffer;
    int ok;

    cp = p->current_cp;

    if (cp == (int32_t)'(') { // is a list
        c_cell = &root;

        parser_next(p); // consume '('

        while (p->current_cp != (int32_t)')') {
            *c_cell = snode_new(ATYPE_CELL, NULL, NULL);
            parser_skip_wc(p);

            (*c_cell)->car = parser_parse_sexp_element(p);

            if ((*c_cell)->car == NULL) { // end of list case
                free(*c_cell);
                break;
            }

            c_cell = &((*c_cell)->cdr);
        }
        (*c_cell) = NULL;

        // consume the last ')'
        parser_next(p);

        return root;
    } else if (cp == (int32_t)')') { // end of list
        return NULL;
    } else if (cp == (int32_t)'"') { // is a string literal
        parser_next(p);

        tmp_buffer = parser_get_til_dquote(p);

        root = snode_new(ATYPE_STRING, NULL, NULL);
        root->data.string = tmp_buffer;

        return root;
    } else if (isdigit(cp)) { // is a number
        root = snode_new(ATYPE_NUMBER, NULL, NULL);
        void* tmp = parser_get_til_whitespace(p);

        root->data.nvalue = parser_parse_num(tmp, &ok);

        free(tmp);
        return root;
    } else if (cp == (int32_t)'#') {
        parser_next(p);

        root = snode_new(ATYPE_BOOLEAN, NULL, NULL);

        if (p->current_cp == (int32_t)'t' || p->current_cp == (int32_t)'f') {
            root->data.boolvalue = (p->current_cp == (int32_t)'t');
            parser_next(p);
            return root;
        }

        return NULL;
    } else if (cp == (int32_t)'.') { // is a dotted pair?
        // *REAL* TODO
    } else if (cp == '\'') { // is a quoted expr
        // TODO
    } else { // is an identifier
        tmp_buffer = parser_get_til_whitespace(p);
        parser_skip_wc(p);

        root = snode_new(ATYPE_SYMBOL, NULL, NULL);
        root->data.symbol = tmp_buffer;

        return root;
    }

    return NULL; // shouldn't get to here, I think (?)
}

void parser_terminate(parser_state* p) {
    if (!p)
        return;

    free(p->buffer);
}

snode* parse_expr(const char* input) {
    parser_state parser, *p;
    int32_t cp;
    snode* ret;

    p = &parser;
    parser_state_init(&parser, input);

    // read first char
    parser_next(p);

    // skip whitespace and comments
    parser_skip_wc(p);

    ret = parser_parse_sexp_element(p);

    //print_parser_state(p);

    parser_state_terminate(&parser);

    return ret;
}

snode* snode_deep_copy(snode* node) {
    snode* new_node;

    if (!node)
        return NULL;

    new_node = snode_new(node->type, NULL, NULL);
    switch(node->type) {
        case ATYPE_NUMBER:
            new_node->data.nvalue = node->data.nvalue;
            break;
        case ATYPE_STRING:
            new_node->data.string = utf8dup(node->data.string);
            break;
        case ATYPE_SYMBOL:
            new_node->data.symbol = utf8dup(node->data.symbol);
            break;
        case ATYPE_BOOLEAN:
            new_node->data.boolvalue = node->data.boolvalue;
            break;
        case ATYPE_CELL:
            new_node->car = snode_deep_copy(node->car);
            new_node->cdr = snode_deep_copy(node->cdr);
            break;
        case ATYPE_PROCEDURE:
            new_node->data.proc = procdata_new(node->data.proc->flags, node->data.proc->data.lang, node->data.proc->data.native);

            break;
        default:
            break;
    }

    return new_node;
}

environment* env_new(environment* up_env) {
    environment* env;

    env = (environment*)malloc(sizeof(*env));
    env->up_env = up_env;
    env->list = NULL;

    return env;
}

struct env_hl_cell* hlc_new(void* key, snode* value, struct env_hl_cell* next) {
    struct env_hl_cell* cell;

    cell = (struct env_hl_cell*)malloc(sizeof(*cell));
    cell->key = utf8dup(key);
    cell->value = snode_deep_copy(value);
    cell->next = next;

    return cell;
}

int env_push(environment* env, void* key, snode* value) {
    struct env_hl_cell *ccell, *pcell;
    int ret_result = -1;

    if (env->list == NULL) {
        env->list = hlc_new(key, value, NULL);
        return TRUE;
    }

    ccell = env->list;
    pcell = NULL;

    do {
        ret_result = utf8cmp(key, ccell->key);
        pcell = ccell;
        ccell = ccell->next;
    } while (ret_result < 0 && ccell != NULL);

    if (ret_result == 0) {
        return FALSE;
    }

    pcell->next = hlc_new(key, value, ccell);

    return TRUE;
}

snode* env_get(environment* env, void* key) { // NOTE, DO NOT FREE
    struct env_hl_cell *ccell;

    ccell = env->list;
    while (ccell != NULL) {
        if (utf8cmp(key, ccell->key) == 0)
            return (ccell->value);

        ccell = ccell->next;
    }

    if (env->up_env != NULL) {
        return env_get(env->up_env, key);
    }

    return NULL;
}

int env_set(environment* env, void* key, snode* value) {
    struct env_hl_cell *ccell;

    ccell = env->list;
    while (ccell != NULL) {
        if (utf8cmp(ccell->key, key) == 0) {
            snode_release(&(ccell->value));
            ccell->value = value;

            return TRUE;
        }

        ccell = ccell->next;
    }

    if (env->up_env != NULL) {
        return env_set(env->up_env, key, value);
    }

    return FALSE;
}

void rec_delete_hcells(struct env_hl_cell* cell) {
    if (cell == NULL)
        return;

    rec_delete_hcells(cell->next);

    free(cell->key);
    snode_release(&(cell->value));
    free(cell);
}

void env_delete(environment* env, int delete_up) {
    if (env == NULL)
        return;

    rec_delete_hcells(env->list);

    if (delete_up)
        env_delete(env->up_env, delete_up);

    free(env);
}

snode* snode_new_error(const void* error_str) {
    snode* node;

    node = snode_new(ATYPE_ERROR, NULL, NULL);
    node->data.string = utf8dup(error_str);

    return node;
}

snode* eval(snode*,environment*);

snode* eval_if(snode* expr, environment* env) {
    snode *eval_ret, *tmp;

    if (expr->cdr == NULL)
        return snode_new_error("NULL cdr when evaling if.");

    tmp = expr->cdr->car;
    eval_ret = eval(tmp, env);

    if (eval_ret->type != ATYPE_BOOLEAN) {
        snode_release(&eval_ret);
        return snode_new_error("Expression does not eval to boolean.");
    }

    if (eval_ret->data.boolvalue) {
        tmp = expr->cdr->cdr;
        if (!tmp || !tmp->car) {
            snode_release(&eval_ret);
            return snode_new_error("Missing #t arm on if");
        }

        tmp = tmp->car;
        
        snode_release(&eval_ret);

        return eval(tmp, env);
    } else {
        tmp = expr->cdr->cdr;
        if (!tmp || !tmp->cdr || !tmp->cdr->car) {
            snode_release(&eval_ret);
            return snode_new_error("Missing #f arm on if");
        }

        tmp = tmp->cdr->car;
        
        snode_release(&eval_ret);

        return eval(tmp, env);
    }

    return NULL;
}

snode* eval_define(snode* expr, environment* env) {
    snode* tmp;
    void* key;
    snode* value;

    tmp = expr->cdr;

    if (!tmp || !tmp->car)
        return snode_new_error("Missing symbol name.");

    key = tmp->car->data.symbol;

    if (!tmp || !tmp->cdr || !tmp->cdr->car)
        return snode_new_error("Missing value.");

    tmp = expr->cdr->cdr->car;

    value = eval(tmp, env);

    env_push(env, key, value);

    return value;
}

snode* eval_set(snode* expr, environment* env) {
    snode* tmp;
    void* key;
    snode* value;

    tmp = expr->cdr;

    if (!tmp || !tmp->car)
        return snode_new_error("Missing symbol.");
    
    if (!tmp || !tmp->cdr || !tmp->cdr->car)
        return snode_new_error("Missing value.");

    if (tmp->car->type != ATYPE_SYMBOL)
        return snode_new_error("Set value not symbol.");

    key = tmp->car->data.symbol;

    value = eval(tmp->cdr->car, env);

    env_set(env, key, snode_deep_copy(value));

    return value;
}

snode* eval_lambda(snode* expr, environment* env) {
    snode* tmp;
    return NULL;
}

snode* eval_list_rec(snode* expr, environment* env) {
    snode* node;

    if (expr == NULL)
        return NULL;

    node = snode_new(ATYPE_CELL, eval(expr->car, env), eval_list_rec(expr->cdr, env));

    return node;
}

snode* eval(snode* expr, environment* env) {
    snode *tmp, *procnode;
    void* tmpstr;

    if (expr == NULL || env == NULL)
        return NULL;

    if (expr->quotes > 0) { //TODO
        tmp = snode_deep_copy(expr);
        (tmp->quotes)--;

        return tmp;
    }

    switch (expr->type) {
        case ATYPE_NUMBER:
        case ATYPE_ERROR:
        case ATYPE_STRING:
        case ATYPE_BOOLEAN:
            return snode_deep_copy(expr);
        case ATYPE_SYMBOL:
            return snode_deep_copy(env_get(env, expr->data.symbol));
        case ATYPE_CELL:
        case ATYPE_PROCEDURE:
            tmp = expr->car;

            if (tmp == NULL) {
                tmp = snode_new(ATYPE_ERROR, NULL, NULL);
                tmp->data.string = utf8dup("List/Procedure has NULL car.");
                return tmp;
            }

            // resolve what to run if it is a symbol
            if (tmp->type == ATYPE_SYMBOL) {
                tmpstr = tmp->data.symbol;
                procnode = env_get(env, tmpstr);

                if (procnode == NULL || (procnode->type != ATYPE_PROCEDURE))
                    return snode_new_error("No such procedure.");

                if (procdata_is_native(procnode->data.proc)) { // native C procedure
                    if (procdata_is_no_eval(procnode->data.proc)) { // should we eval args?
                        return (procnode->data.proc->data.native)(expr, env);
                    } else { // we need to eval args
                        snode* evald_args = eval_list_rec(expr->cdr, env);
                        tmp = (procnode->data.proc->data.native)(evald_args, env);
                        snode_release(&evald_args);
                        return tmp;
                    }
                } else { // APPLY here?
                    
                }
            }
        default:
            break;
    }

    return NULL;
}

procdata* procdata_new(int flags, snode* lang, native_proc native) {
    procdata* p;

    p = (procdata*)malloc(sizeof(*p));
    if (!p)
        return NULL;

    p->flags = flags;
    
    if (procdata_is_native(p))
        p->data.native = native;
    else
        p->data.lang = snode_deep_copy(lang);

    return p;
}

void procdata_release(procdata* p) {
    if (!p)
        return;

    if (!procdata_is_native(p))
        snode_release(&(p->data.lang));

    free(p);
}

void env_init_defaults(environment* env) {
    snode tmp;
    procdata tmppd;

    // common snode data
    tmp.type = ATYPE_PROCEDURE;
    tmp.data.proc = &tmppd;

    // common procdata
    tmppd.flags = (PROCFLAG_NATIVE | PROCFLAG_NO_EVAL_ARGS);

    // if
    tmppd.data.native = eval_if;
    env_push(env, "if", &tmp);

    // begin // TODO

    // define
    tmppd.data.native = eval_define;
    env_push(env, "define", &tmp);

    // set!
    tmppd.data.native = eval_set;
    env_push(env, "set!", &tmp);

    // lambda
    tmppd.data.native = eval_lambda;
    env_push(env, "lambda", &tmp);
}

char* readline(FILE* input) {
    char c, *str = NULL;
    int len = 0;

    c = getc(input);

    while (c != '\n') {
        str = (char*)realloc(str, sizeof(char) * (len + 1));
        str[len++] = c;
        c = getc(input);
    }

    str = (char*)realloc(str, sizeof(char) * (len + 1));
    str[len] = '\0';

    return str;
}

int main(int argc, char** argv) {
    int done = 0;
    char* input = NULL;
    snode* expr, *evald;
    environment* env;
    snode* test;

    test = snode_new(ATYPE_STRING, NULL, NULL);
    test->data.string = utf8dup("HE GIVING ME THAT GOOD SHIT.");

    env = env_new(NULL);
    env_push(env, "test", test);

    env_init_defaults(env);

    snode_release(&test);

    while (!done) {
        printf("> ");
        input = readline(stdin);

        if (!strcmp(input, "exit")) {
            done = 1;
        }

        expr = parse_expr(input);

        printf("expr: ");
        print_expr(expr);
        printf("\n");

        printf("eval'd: ");
        evald = eval(expr, env);
        print_expr(evald);
        printf("\n");

        snode_release(&expr);
        snode_release(&evald);
        free(input);
        input = NULL;
    }

    env_delete(env, TRUE);

    return 0;
}

