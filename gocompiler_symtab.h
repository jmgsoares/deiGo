#ifndef _gocompiler_symtab_h
#define _gocompiler_symtab_h

#include "gocompiler_structures.h"
#include "gocompiler_ast.h"

symbol_table_list_head *symbol_table;

symbol_table_list_head *insert_symbol_table_list_head(symbol_table_list_head *, symbol_table_list_head *);

symbol_table_list_head *
create_symbol_table_list_head(char *, parameter_list_node *, symbol_table_list_node *, type, position, ast_node *);

symbol_table_list_head *
create_and_insert_symbol_table_list_head(symbol_table_list_head *, char *, parameter_list_node *,
                                         symbol_table_list_node *, type, position, ast_node *);

symbol_table_list_node *insert_symbol_table_list_node(symbol_table_list_node *, symbol_table_list_node *);

symbol_table_list_node *
create_symbol_table_list_node(char *symbol_name, type symbol_type, bool is_function, symbol_table_list_head *function,
                              position pos, ast_node *var_decl_ast_node, bool is_global_var_symbol);

symbol_table_list_node *
create_and_insert_symbol_table_list_node(symbol_table_list_node *head, char *symbol_name, type symbol_type,
                                         bool is_function, symbol_table_list_head *function, position pos,
                                         ast_node *var_decl_ast_node, bool is_global_var_symbol);

parameter_list_node *insert_parameter_list_node(parameter_list_node *, parameter_list_node *);

parameter_list_node *create_parameter_list_node(char *, type);

parameter_list_node *create_and_insert_parameter_list_node(parameter_list_node *, char *, type);

bool check_function_matching_parameters_with_call(ast_node *, parameter_list_node *);

bool is_symbol_valid_global_scope(ast_node *node);

bool is_symbol_valid_function_scope(symbol_table_list_node *function_symbols, ast_node *node);

bool is_symbol_parameter(parameter_list_node *, char *);

symbol_table_list_head *get_function_symbol_table(char *);

type get_symbol_type_from_global_symtab(char *symbol_to_search, ast_node *node);

type get_symbol_type_from_function_parameters(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, ast_node *node
);

type
get_symbol_type_from_function_symbols(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, position p, bool mark_as_used,
		ast_node *node
);

type get_symbol_type_from_all_scopes(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, position p, bool mark_as_used,
		ast_node *node
);

type get_function_type_from_funcdecl_node(ast_node *);

void print_function_details(symbol_table_list_head *function);

void print_symbol_details(symbol_table_list_node *symbol);

void print_symbol_table();

#endif
