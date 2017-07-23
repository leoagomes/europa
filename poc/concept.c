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
    //ATYPE_QSYMBOL,
    ATYPE_NUMBER,
    ATYPE_STRING
} sn_type;

struct snode {
    sn_type type;
    union {
        void* symbol;
        void* string;
        num nvalue;
    } data;

    int quoted;

    snode *car, *cdr;
};

typedef struct parser_state parser_state;
struct parser_state {
    void* buffer;
    int32_t current_cp;
    void* current_cp_pos;
    void* next_cp;

    int line, col;

    int has_error;
    int32_t expected_cp;
};

typedef struct environment environment;
struct environment {
    environment* up_env;


};

void parser_state_init(parser_state* parser, const void* input) {
    parser->buffer = malloc(utf8size(input));
    utf8cpy(parser->buffer, input);

    parser->next_cp = parser->buffer;
    parser->current_cp = -1;
    parser->current_cp_pos = NULL;

    parser->line = parser->col = 0;

    parser->has_error = FALSE;
    parser->expected_cp = -1;

    parser->quoted = FALSE;
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
    tmp = *node;

    if (tmp->car != NULL) {
        snode_release(&(tmp->car));
    }
    if (tmp->cdr != NULL) {
        snode_release(&(tmp->car));
    }

    if (tmp->type == ATYPE_SYMBOL) {
        free(tmp->data.symbol);
    } else if (tmp->type == ATYPE_STRING) {
        free(tmp->data.string);
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
        root->data.nvalue = parser_parse_num(parser_get_til_whitespace(p), &ok);
        return root;
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

    return ret;
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
    snode* expr;

    while (!done) {
        printf("> ");
        input = readline(stdin);

        if (!strcmp(input, "exit")) {
            done = 1;
        }

        print_expr(parse_expr(input));
        printf("\n");

        free(input);
        input = NULL;
    }

    return 0;
}

