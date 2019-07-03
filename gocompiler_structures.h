#ifndef GOCOMPILER_GOCOMPILER_STRUCTURES_H
#define GOCOMPILER_GOCOMPILER_STRUCTURES_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//#define DEBUGAAST
//#define DEBUGSYMTAB

extern int semantic_error;

#define list_iterator(i, j, k) for((i) = j; i; (i) = (i)->k)

#define list_last_node(i, j, k) for((i) = j; (i)->next; (i) = (i)->k)

#define compare_strings(i, j) strcmp((i), j) == 0 ? true : false

#define position_i_before_j(i, j) ((i).line < (j).line || ((i).line == (j).line && (i).col < (j).col)) ? true : false

typedef enum type {
	_INT, _FLOAT32, _BOOL, _STRING, _VOID, _NULL, _UNDEF, _TODO_VAR_STMNT, _TODO_VAR_EXPR, _TODO_FUNC
} type;

typedef enum error_type {
	_NO_ERR, _UNUSED, _TYPE_MISMATCH_BIN, _TYPE_MISMATCH_UNR, _REDEC, _NDEF, _IMCOMP_TYPE, _INV_OCTAL
} error_type;

typedef enum scope {_GLOBAL, _PARAM, _LOCAL, _STRLIT, _REALLIT, _INTLIT, _OPERATION} scope;

typedef struct position {
	int line;
	int col;
} position;

typedef struct positioned_token {
	char *value;
	position p;
} positioned_token;

typedef struct parameter_list_node {
	char *parameter_name;
	type parameter_type;
	struct parameter_list_node *next;
	int temp_id;
} parameter_list_node;

typedef struct ast_node {
	char *node_type;
	char *value;
	type primitive_type;
	error_type error_type;
	bool is_function_id;
	bool is_used;
	struct ast_node *son;
	struct ast_node *brother;
	position position;
	parameter_list_node *function_parameters;
	struct symbol_table_list_head *function_symbol_table;
	union {
		struct parameter_list_node* parameter;
		struct symbol_table_list_node *symbol;
	} symbol_table_entry;
	scope symbol_scope;
	int temp_id;
	int value_size;
} ast_node;

typedef struct symbol_table_list_head {
	char *function_name;
	parameter_list_node *function_parameters;
	struct symbol_table_list_node *function_symbols;
	type function_type;
	struct symbol_table_list_head *next;
	position p;
	ast_node *func_decl_ast_node;
} symbol_table_list_head;

typedef struct symbol_table_list_node {
	char *symbol_name;
	type symbol_type;
	bool is_function;
	bool is_used;
	symbol_table_list_head *function;
	struct symbol_table_list_node *next;
	position p;
	ast_node *var_decl_ast_node;
	int temp_id;
} symbol_table_list_node;

type string_to_type(char *);

char *type_to_string(type type);

int char_to_int(char d);

char *operator_to_string(char *operator);

void free_function_parameters(parameter_list_node *);

void free_function_symbols(symbol_table_list_node*);

void free_symbol_table(symbol_table_list_head *);

void free_ast(ast_node *);

#endif //GOCOMPILER_GOCOMPILER_STRUCTURES_H
