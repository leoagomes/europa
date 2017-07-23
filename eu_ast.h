#ifndef __EU_AST_H__
#define __EU_AST_H__

enum eu_ast_node_type {
    TOKEN
};

struct eu_ast_node {
    int id;
    union {
        void* string;
        long ivalue;
        double fvalue;
    } data;
};

#endif