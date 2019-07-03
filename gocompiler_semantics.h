#ifndef GOCOMPILER_GOCOMPILER_SEMANTICS_H
#define GOCOMPILER_GOCOMPILER_SEMANTICS_H

#include "gocompiler_structures.h"
#include "gocompiler_symtab.h"
#include "gocompiler_ast.h"

type parse_binary_operator_type(ast_node *);

void semantic_analysis(ast_node *node);

void create_symbol_table(ast_node *);

void parse_program_symbols(ast_node *node);

parameter_list_node *parse_function_parameters(ast_node *funcdecl_ast_node);

symbol_table_list_node *parse_function_symbols(ast_node *vardecl_ast_node, symbol_table_list_node *function_symbols,
                                               parameter_list_node *function_parameters);

void search_function_for_semantic_analysis(ast_node *);

void function_semantic_analysis(symbol_table_list_head *current_function_symbol_table, ast_node *node);

void print_semantic_errors(ast_node *node);

void check_unused_variables(ast_node *);

#endif //GOCOMPILER_GOCOMPILER_SEMANTICS_H
