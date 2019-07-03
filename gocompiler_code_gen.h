#ifndef GOCOMPILER_GOCOMPILER_CODE_GEN_H
#define GOCOMPILER_GOCOMPILER_CODE_GEN_H

#include "gocompiler_structures.h"

int temp_counter, label_counter, string_count;

char *primitive_char_type_to_default_value(char *node_type);

char *primitive_type_to_char(type t);

void generate_includes();

void generate_assign(ast_node *node);

void generate_static_strings(ast_node *node);

void generate_int_float_literals(ast_node* node);

void handle_string(ast_node *node);

void generate_add(ast_node *node);

void generate_div(ast_node *node);

void generate_mod(ast_node *node);

void generate_mul(ast_node *node);

void generate_sub(ast_node *node);

void generate_global_variable(ast_node *node);

void parse_if_for_expressions(ast_node *node, symbol_table_list_head *symbol_table);

void generate_parameter_variable(parameter_list_node *param);

void generate_local_variable(symbol_table_list_node *symbol);

void generate_print(ast_node *node);

void generate_variable_load(ast_node *node);

void generate_llvmir(ast_node *node, symbol_table_list_head *symbol_table);

void parse_global_variables(ast_node *node, symbol_table_list_head *symbol_table);

void generate_minus(ast_node *node);

void generate_plus(ast_node *node);

void generate_not(ast_node *node);

void generate_and(ast_node *node);

void generate_or(ast_node *node);

void generate_eq(ast_node *node);

void generate_ne(ast_node *node);

void generate_lt(ast_node *node);

void generate_le(ast_node *node);

void generate_gt(ast_node *node);

void generate_ge(ast_node *node);

void generate_if(ast_node *node, symbol_table_list_head *symbol_table);

void generate_for(ast_node *node, symbol_table_list_head *symbol_table);

void generate_call(ast_node *node);

void generate_return(ast_node *node);

void generate_parse_args(ast_node *node);

void parse_functions(ast_node *node, symbol_table_list_head *symbol_table);

void parse_function_header(symbol_table_list_head *symbol_table, ast_node *node);

void parse_function_body(ast_node *node, symbol_table_list_head *symbol_table);

void generate_function_footer(symbol_table_list_head *symbol_table);

void generate_function_prototype(symbol_table_list_head *symbol_table);

#endif //GOCOMPILER_GOCOMPILER_CODE_GEN_H
