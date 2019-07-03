#include "gocompiler_symtab.h"
#include "gocompiler_structures.h"

symbol_table_list_head *insert_symbol_table_list_head(symbol_table_list_head *head, symbol_table_list_head *new_head) {
	if (!head) {
		head = new_head;
		return head;
	}
	symbol_table_list_head *temp;
	list_last_node(temp, head, next);
	temp->next = new_head;
	return head;
}

symbol_table_list_head *create_symbol_table_list_head(char *function_name, parameter_list_node *function_parameters,
                                                      symbol_table_list_node *function_symbols, type function_type,
                                                      position pos, ast_node *func_decl_ast_node) {
	symbol_table_list_head *new_head_node;
	new_head_node = (symbol_table_list_head *) malloc(sizeof(symbol_table_list_head));
	new_head_node->func_decl_ast_node = func_decl_ast_node;
	new_head_node->function_name = strdup(function_name);
	new_head_node->function_parameters = function_parameters;
	new_head_node->function_symbols = function_symbols;
	new_head_node->function_type = function_type;
	new_head_node->next = NULL;
	new_head_node->p = pos;
	return new_head_node;
}

symbol_table_list_head *create_and_insert_symbol_table_list_head(symbol_table_list_head *head, char *function_name,
                                                                 parameter_list_node *function_parameters,
                                                                 symbol_table_list_node *function_symbols,
                                                                 type function_type, position pos,
                                                                 ast_node *func_decl_ast_node) {
	return insert_symbol_table_list_head(head, create_symbol_table_list_head(function_name, function_parameters,
	                                                                         function_symbols, function_type, pos,
	                                                                         func_decl_ast_node));
}

symbol_table_list_node *insert_symbol_table_list_node(symbol_table_list_node *head, symbol_table_list_node *new_node) {
	if (!head) {
		head = new_node;
		return head;
	}
	symbol_table_list_node *temp;
	list_last_node(temp, head, next);
	temp->next = new_node;
	return head;
}

symbol_table_list_node *
create_symbol_table_list_node(char *symbol_name, type symbol_type, bool is_function, symbol_table_list_head *function,
                              position pos, ast_node *var_decl_ast_node, bool is_global_var_symbol) {
	symbol_table_list_node *new_node;
	new_node = (symbol_table_list_node *) malloc(sizeof(symbol_table_list_node));
	if (symbol_name) new_node->symbol_name = strdup(symbol_name);
	else new_node->symbol_name = NULL;
	new_node->is_used = is_global_var_symbol ? true : false;
	new_node->var_decl_ast_node = var_decl_ast_node;
	new_node->symbol_type = symbol_type;
	new_node->function = function;
	new_node->is_function = is_function;
	new_node->p = pos;
	new_node->next = NULL;
	return new_node;
}

symbol_table_list_node *
create_and_insert_symbol_table_list_node(symbol_table_list_node *head, char *symbol_name, type symbol_type,
                                         bool is_function, symbol_table_list_head *function, position pos,
                                         ast_node *var_decl_ast_node, bool is_global_var_symbol) {
	return insert_symbol_table_list_node(head,
	                                     create_symbol_table_list_node(symbol_name, symbol_type, is_function, function,
	                                                                   pos, var_decl_ast_node, is_global_var_symbol));
}

parameter_list_node *insert_parameter_list_node(parameter_list_node *head, parameter_list_node *new_node) {
	if (!head) {
		head = new_node;
		return head;
	}
	parameter_list_node *temp;
	list_last_node(temp, head, next);
	temp->next = new_node;
	return head;
}

parameter_list_node *create_parameter_list_node(char *parameter_name, type parameter_type) {
	parameter_list_node *new_node;
	new_node = (parameter_list_node *) malloc(sizeof(parameter_list_node));
	new_node->parameter_name = strdup(parameter_name);
	new_node->parameter_type = parameter_type;
	new_node->next = NULL;
	return new_node;
}

parameter_list_node *
create_and_insert_parameter_list_node(parameter_list_node *head, char *parameter_name, type parameter_type) {
	return insert_parameter_list_node(head, create_parameter_list_node(parameter_name, parameter_type));
}


bool is_symbol_valid_function_scope(symbol_table_list_node *function_symbols, ast_node *node) {
	symbol_table_list_node *temp;

	list_iterator(temp, function_symbols, next) {
		if (compare_strings(node->value, temp->symbol_name)) return true;
	}
	return false;
}

bool is_symbol_valid_global_scope(ast_node *node) {
	symbol_table_list_node *temp;

	list_iterator(temp, symbol_table->function_symbols, next) {
		if (!temp->is_function)
			if (compare_strings(node->value, temp->symbol_name)) return true;
	}
	return false;
}

bool is_symbol_parameter(parameter_list_node *function_parameters, char *symbol_to_check) {
	parameter_list_node *temp;
	list_iterator(temp, function_parameters, next) {
		if (compare_strings(temp->parameter_name, symbol_to_check)) return true;
	}
	return false;
}

bool check_function_matching_parameters_with_call(ast_node *func_id_ast_node,
                                                  parameter_list_node *declared_function_parameters) {

	if (!func_id_ast_node->brother) {
		//Both functions have no parameters
		if (!declared_function_parameters) {
			return true;
		}
			//One of the functions has parameters and the other don't, we continue to check if any other function matches
		else {
			return false;
		}
	}
		//One of the functions has parameters and the other don't, we continue to check if any other function matches
	else if (!declared_function_parameters) {
		return false;
	}

	//both functions have parameters, we have to check if they have the same number and type
	bool parameter_mismatch_flag = false;
	ast_node *current_parameter_call_ast_node = func_id_ast_node->brother;
	parameter_list_node *current_declared_function_parameter;

	list_iterator(current_declared_function_parameter, declared_function_parameters, next) {

		//Parameter's match
		if (current_declared_function_parameter->parameter_type == current_parameter_call_ast_node->primitive_type) {
			if (current_declared_function_parameter->next) {
				if (current_parameter_call_ast_node->brother) {
					//Both functions (called and the one on symtab) have more parameters to check
					current_parameter_call_ast_node = current_parameter_call_ast_node->brother;
					continue;
				}
				else {
					//One of the functions has mores parameters and the other don't
					parameter_mismatch_flag = true;
					break;
				}
			}
			else {
				//One of the functions has mores parameters and the other don't
				if (current_parameter_call_ast_node->brother) {
					parameter_mismatch_flag = true;
					break;
				}
					//both functions have no more parameters
				else break;
			}
		}
		else {
			parameter_mismatch_flag = true;
			break;
		}
	}
	if (!parameter_mismatch_flag)
		return true;

	return false;
}

type get_function_type_from_funcdecl_node(ast_node *funcdecl_ast_node) {
	if (compare_strings("FuncParams", funcdecl_ast_node->son->son->brother->node_type)) return _VOID;
	return string_to_type(funcdecl_ast_node->son->son->brother->node_type);
}

symbol_table_list_head *get_function_symbol_table(char *function_name) {
	symbol_table_list_head *temp;
	list_iterator(temp, symbol_table->next, next) {
		if (compare_strings(function_name, temp->function_name)) {
			temp->func_decl_ast_node->is_used = true;
			return temp;
		}
	}
	return NULL;
}

type get_symbol_type_from_global_symtab(char *symbol_to_search, ast_node *node) {
	symbol_table_list_node *current_list_node;
	list_iterator(current_list_node, symbol_table->function_symbols, next) {
		if (!current_list_node->is_function)
			if (compare_strings(symbol_to_search, current_list_node->symbol_name)) {
				current_list_node->is_used = true;
				current_list_node->var_decl_ast_node->is_used = true;
				node->symbol_table_entry.symbol = current_list_node;
				node->symbol_scope = _GLOBAL;
				return current_list_node->symbol_type;
			}
	}
	return _UNDEF;
}

type get_symbol_type_from_function_parameters(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, ast_node *node
) {
	parameter_list_node *current_parameter;
	list_iterator(current_parameter, function_symbol_table->function_parameters, next) {
		if (compare_strings(current_parameter->parameter_name, symbol_to_search)) {
			node->symbol_table_entry.parameter = current_parameter;
			node->symbol_scope = _PARAM;
			return current_parameter->parameter_type;
		}
	}
	return _UNDEF;
}

type
get_symbol_type_from_function_symbols(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, position p, bool mark_as_used,
		ast_node *node
) {
	symbol_table_list_node *current_list_node;
	list_iterator(current_list_node, function_symbol_table->function_symbols, next) {
		if (!current_list_node->is_function)
			if (compare_strings(symbol_to_search, current_list_node->symbol_name))
				if (position_i_before_j(current_list_node->p, p)) {
					if (!current_list_node->is_used) {
						current_list_node->is_used = mark_as_used;
						current_list_node->var_decl_ast_node->is_used = mark_as_used;
					}
					node->symbol_table_entry.symbol = current_list_node;
					node->symbol_scope = _LOCAL;
					return current_list_node->symbol_type;
				}
	}
	return _UNDEF;
}

type get_symbol_type_from_all_scopes(
		symbol_table_list_head *function_symbol_table, char *symbol_to_search, position p, bool mark_as_used,
		ast_node *node
) {
	type temp_type;
	temp_type = get_symbol_type_from_function_symbols(function_symbol_table, symbol_to_search, p, mark_as_used, node);
	if (temp_type == _UNDEF) {
		temp_type = get_symbol_type_from_function_parameters(function_symbol_table, symbol_to_search, node);
		if (temp_type == _UNDEF) {
			temp_type = get_symbol_type_from_global_symtab(symbol_to_search, node);
		}
	}
	return temp_type;
}

void print_function_details(symbol_table_list_head *function) {
	parameter_list_node *current_parameter;

	printf("%s\t", function->function_name);
	printf("(");
	list_iterator(current_parameter, function->function_parameters, next) {
		printf("%s", type_to_string(current_parameter->parameter_type));
		if (current_parameter->next) printf(",");
	}
	printf(")");
	printf("\t%s", type_to_string(function->function_type));
#ifdef DEBUGSYMTAB
	printf("\tl:%d, c:%d", function->p.line, function->p.col);
#endif
	printf("\n");
}

void print_symbol_details(symbol_table_list_node *symbol) {
	printf("%s\t", symbol->symbol_name);
	printf("\t%s", type_to_string(symbol->symbol_type));
#ifdef DEBUGSYMTAB
	printf("\tl:%d, c:%d", symbol->p.line, symbol->p.col);
#endif
	printf("\n");
}

void print_symbol_table() {
	symbol_table_list_head *current_function_symbol_table_head;
	symbol_table_list_node *symbol_table_entry;
	parameter_list_node *current_parameter;

	//Print global symbol table
	printf("===== Global Symbol Table =====");
#ifdef DEBUGSYMTAB
	printf("\tl:%d, c:%d", symbol_table->p.line, symbol_table->p.col);
#endif
	printf("\n");

	list_iterator(symbol_table_entry, symbol_table->function_symbols, next) {
		if (symbol_table_entry->is_function) print_function_details(symbol_table_entry->function);
		else print_symbol_details(symbol_table_entry);
	}

	//Print all functions symbol tables
	list_iterator(current_function_symbol_table_head, symbol_table->next, next) {
		printf("\n");
		//print table title row for function
		printf("===== Function %s", current_function_symbol_table_head->function_name);
		printf("(");
		list_iterator(current_parameter, current_function_symbol_table_head->function_parameters, next) {
			printf("%s", type_to_string(current_parameter->parameter_type));
			if (current_parameter->next) printf(",");
		}
		printf(")");
		printf(" Symbol Table =====");
#ifdef DEBUGSYMTAB
		printf("\tl:%d, c:%d", current_function_symbol_table_head->p.line, current_function_symbol_table_head->p.col);
#endif
		printf("\n");

		//print return type
		if (current_function_symbol_table_head->function_type == _VOID) printf("return\t\tnone");
		else printf("return\t\t%s", type_to_string(current_function_symbol_table_head->function_type));
#ifdef DEBUGSYMTAB
		printf("\tl:%d, c:%d", current_function_symbol_table_head->p.line, current_function_symbol_table_head->p.col);
#endif
		printf("\n");

		//print function args
		list_iterator(current_parameter, current_function_symbol_table_head->function_parameters, next) {
			printf("%s\t\t%s\tparam", current_parameter->parameter_name,
			       type_to_string(current_parameter->parameter_type));
#ifdef DEBUGSYMTAB
			printf("\tl:%d, c:%d", current_function_symbol_table_head->p.line,
				   current_function_symbol_table_head->p.col);
#endif
			printf("\n");
		}

		//print function symbols
		list_iterator(symbol_table_entry, current_function_symbol_table_head->function_symbols,
		              next) print_symbol_details(symbol_table_entry);
	}
}
