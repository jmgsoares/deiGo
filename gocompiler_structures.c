#include "gocompiler_structures.h"

type string_to_type(char *string) {

	if (string) {
		if (compare_strings(string, "Int")) return _INT;
		if (compare_strings(string, "Float32")) return _FLOAT32;
		if (compare_strings(string, "Bool")) return _BOOL;
		if (compare_strings(string, "String")) return _STRING;
		if (compare_strings(string, "Todo_var_stmnt")) return _TODO_VAR_STMNT;
		if (compare_strings(string, "Todo_var_expr")) return _TODO_VAR_EXPR;
		if (compare_strings(string, "Todo_func")) return _TODO_FUNC;
		if (compare_strings(string, "undef")) return _UNDEF;
	}
	return _VOID;
}

char *type_to_string(type type) {
	switch (type) {
		case _INT:
			return "int";
		case _FLOAT32:
			return "float32";
		case _BOOL:
			return "bool";
		case _STRING:
			return "string";
		case _VOID:
			return "none";
		case _TODO_VAR_STMNT:
			return "todo_var_stmnt";
		case _TODO_VAR_EXPR:
			return "todo_var_expr";
		case _TODO_FUNC:
			return "todo_func";
		case _UNDEF:
			return "undef";
		default:
			return "";
	}
}

int char_to_int(char d) {
	char str[2];
	str[0] = d;
	str[1] = '\0';
	return (int) strtol(str, NULL, 10);
}

char *operator_to_string(char *operator) {
	if (compare_strings(operator, "Or")) return "||";
	if (compare_strings(operator, "And")) return "&&";
	if (compare_strings(operator, "Lt")) return "<";
	if (compare_strings(operator, "Gt")) return ">";
	if (compare_strings(operator, "Eq")) return "==";
	if (compare_strings(operator, "Ne")) return "!=";
	if (compare_strings(operator, "Le")) return "<=";
	if (compare_strings(operator, "Ge")) return ">=";
	if (compare_strings(operator, "Plus")) return "+";
	if (compare_strings(operator, "Minus")) return "-";
	if (compare_strings(operator, "Star")) return "*";
	if (compare_strings(operator, "Div")) return "/";
	if (compare_strings(operator, "Mod")) return "%";
	if (compare_strings(operator, "Assign")) return "=";
	if (compare_strings(operator, "Add")) return "+";
	if (compare_strings(operator, "Sub")) return "-";
	if (compare_strings(operator, "Mul")) return "*";
	if (compare_strings(operator, "Not")) return "!";
	if (compare_strings(operator, "ParseArgs")) return "strconv.Atoi";
	return "";
}

void free_ast(ast_node* node){
	if(node == NULL) {
		return;
	}

    if (node->son != NULL) {
        free_ast(node->son);
    }

    if (node->brother != NULL) {
        free_ast(node->brother);
    }

    free(node->node_type);
    free(node->value);
    free(node);
}

void free_function_parameters(parameter_list_node* parameter_node){
    parameter_list_node* temp_parameter_node;
    parameter_list_node* last_temp_parameter_node = NULL;

    list_iterator(temp_parameter_node, parameter_node, next){
	    free(last_temp_parameter_node);
	    free(temp_parameter_node->parameter_name);
        last_temp_parameter_node = temp_parameter_node;
    }
    free(last_temp_parameter_node);
}

void free_function_symbols(symbol_table_list_node* symbol_node){
    symbol_table_list_node* temp_symbol_node;
    symbol_table_list_node* last_temp_symbol_node = NULL;

    list_iterator(temp_symbol_node, symbol_node, next){
	    free(last_temp_symbol_node);
	    free(temp_symbol_node->symbol_name);
        last_temp_symbol_node = temp_symbol_node;
    }
    free(last_temp_symbol_node);
}

void free_symbol_table(symbol_table_list_head* symbol_table){
    symbol_table_list_head* temp_head_node;
    symbol_table_list_head* last_temp_head_node = NULL;

    list_iterator(temp_head_node, symbol_table, next){
    	free(temp_head_node->function_name);
        free_function_parameters(temp_head_node->function_parameters);
        free_function_symbols(temp_head_node->function_symbols);
        free(last_temp_head_node);
        last_temp_head_node = temp_head_node;
    }
    free(last_temp_head_node);
}
