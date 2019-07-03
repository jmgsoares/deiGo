#ifndef _gocompiler_ast_h
#define _gocompiler_ast_h

#include "gocompiler_structures.h"

ast_node *root;

ast_node *create_root_node(char *, positioned_token *);

ast_node *create_node(char *, positioned_token *);

ast_node *create_node_with_type(char *, char *, positioned_token *);

ast_node *create_node_with_position_from_node(char *, ast_node *);

char *get_function_name_from_funcdecl_node(ast_node *);

void insert_son(ast_node *, ast_node *);

void insert_sibling(ast_node *, ast_node *);

void print_dots(int);

void print_ast(ast_node *, int);

void print_annotated_ast(ast_node *, int);

#endif
