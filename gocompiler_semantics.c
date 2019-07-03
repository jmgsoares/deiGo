#include "gocompiler_semantics.h"

type parse_binary_operator_type(ast_node *node) {
	type operator1, operator2;
	operator1 = node->son->primitive_type;
	operator2 = node->son->brother->primitive_type;
	if (operator1 != operator2) {
		return _UNDEF;
	}
	else {
		return operator1;
	}
}

void semantic_analysis(ast_node *node) {
	create_symbol_table(node);
	search_function_for_semantic_analysis(node);
	check_unused_variables(node);
}

void create_symbol_table(ast_node *node) {

	//create the global symbol table
	symbol_table = create_and_insert_symbol_table_list_head(
			symbol_table, "Global", NULL, NULL, _VOID, node->position, NULL);

	//search the ast for function decl and update tables
	parse_program_symbols(node->son);

}

void parse_program_symbols(ast_node *node) {
	if (node == NULL) return;

	if (compare_strings("FuncDecl", node->node_type)) {
		char *function_name = get_function_name_from_funcdecl_node(node);
		type function_type = get_function_type_from_funcdecl_node(node);
		parameter_list_node *function_parameters = parse_function_parameters(node);
		symbol_table_list_node *function_symbols = NULL;
		function_symbols = parse_function_symbols(node->son->brother, function_symbols, function_parameters);

		symbol_table_list_head *new_function_symbol_table = create_symbol_table_list_head(
				function_name, function_parameters, function_symbols, function_type, node->position, node
		);

		if (get_function_symbol_table(function_name) != NULL || is_symbol_valid_global_scope(node->son->son)) {
			node->error_type = _REDEC;
			semantic_error++;
		}

		else {
			symbol_table = insert_symbol_table_list_head(symbol_table, new_function_symbol_table);

			symbol_table->function_symbols = insert_symbol_table_list_node(
					symbol_table->function_symbols,
					create_symbol_table_list_node(NULL, _VOID, true, new_function_symbol_table, node->position, node,
					                              false
					));
		}

		node->function_parameters = function_parameters;
		node->function_symbol_table = new_function_symbol_table;
	}

	if (compare_strings("VarDecl", node->node_type)) {

		if (is_symbol_valid_global_scope(node->son->brother)
		    || get_function_symbol_table(node->son->brother->value) != NULL) {
			node->error_type = _REDEC;
			semantic_error++;
		}
		else {
			symbol_table->function_symbols = create_and_insert_symbol_table_list_node(
					symbol_table->function_symbols, node->son->brother->value, string_to_type(
							node->son->node_type
					), false, NULL, node->position, node, true
			);

			//Global variable is always considered used to not raise errors on compile time
			node->is_used = true;
		}
	}

	if (node->brother != NULL) {
		parse_program_symbols(node->brother);
	}
}

parameter_list_node *parse_function_parameters(ast_node *funcdecl_ast_node) {
	ast_node *func_params_ast_node = strcmp("FuncParams", funcdecl_ast_node->son->son->brother->node_type) == 0
	                                 ? funcdecl_ast_node->son->son->brother : funcdecl_ast_node->son->son->brother
	                                                                                           ->brother;
	if (func_params_ast_node->son) {
		parameter_list_node *function_parameter_list = NULL;
		ast_node *param_decl_node;
		list_iterator(param_decl_node, func_params_ast_node->son, brother) {
			if (is_symbol_parameter(function_parameter_list, param_decl_node->son->brother->value)) {
				param_decl_node->error_type = _REDEC;
				semantic_error++;
			}

			function_parameter_list = create_and_insert_parameter_list_node(
					function_parameter_list, param_decl_node->son->brother->value, string_to_type(
							param_decl_node->son->node_type
					));
		}
		return function_parameter_list;
	}
	else { return NULL; }
}

symbol_table_list_node *parse_function_symbols(
		ast_node *vardecl_ast_node, symbol_table_list_node *function_symbols, parameter_list_node *function_parameters
) {
	if (vardecl_ast_node == NULL) return function_symbols;

	if (strcmp("VarDecl", vardecl_ast_node->node_type) == 0) {

		if (is_symbol_valid_function_scope(function_symbols, vardecl_ast_node->son->brother)
		    || is_symbol_parameter(function_parameters, vardecl_ast_node->son->brother->value)) {
			vardecl_ast_node->error_type = _REDEC;
			semantic_error++;
		}
		else {
			function_symbols = create_and_insert_symbol_table_list_node(
					function_symbols, vardecl_ast_node->son->brother->value, string_to_type(
							vardecl_ast_node->son->node_type
					), false, NULL, vardecl_ast_node->position, vardecl_ast_node, false
			);
		}

		if (vardecl_ast_node->brother != NULL) {
			function_symbols = parse_function_symbols(vardecl_ast_node->brother, function_symbols, function_parameters);
		}
	}
	else {
		if (vardecl_ast_node->son != NULL) {
			function_symbols = parse_function_symbols(vardecl_ast_node->son, function_symbols, function_parameters);
		}

		if (vardecl_ast_node->brother != NULL) {
			function_symbols = parse_function_symbols(vardecl_ast_node->brother, function_symbols, function_parameters);
		}
	}
	return function_symbols;
}

void search_function_for_semantic_analysis(ast_node *node) {
	if (node == NULL) return;

	if (compare_strings("FuncDecl", node->node_type)) {
		function_semantic_analysis(node->function_symbol_table, node->son);
	}
	else if (node->son != NULL) {
		search_function_for_semantic_analysis(node->son);
	}

	if (node->brother != NULL) {
		search_function_for_semantic_analysis(node->brother);
	}
}

void function_semantic_analysis(symbol_table_list_head *current_function_symbol_table, ast_node *node) {
	if (node == NULL) return;

	if (node->son != NULL) {
		function_semantic_analysis(current_function_symbol_table, node->son);
	}

	if (node->brother != NULL) {
		function_semantic_analysis(current_function_symbol_table, node->brother);
	}

	if (node->primitive_type == _TODO_VAR_STMNT) {
		type temp = get_symbol_type_from_all_scopes(
				current_function_symbol_table, node->value, node->position, true, node);
		node->primitive_type = temp;
		if (temp == _UNDEF) {
			node->error_type = _NDEF;
			semantic_error++;
		}
	}

	if (node->primitive_type == _TODO_VAR_EXPR) {
		type temp = get_symbol_type_from_all_scopes(
				current_function_symbol_table, node->value, node->position, true, node);
		node->primitive_type = temp;
		if (temp == _UNDEF) {
			node->error_type = _NDEF;
			semantic_error++;
		}
	}

	if (node->primitive_type == _TODO_FUNC) {
		symbol_table_list_head *temp = get_function_symbol_table(node->value);
		if (temp && check_function_matching_parameters_with_call(node, temp->function_parameters)) {
			node->function_parameters = temp->function_parameters;
			node->primitive_type = temp->function_type;
		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _NDEF;
			semantic_error++;
		}
	}

	else if (compare_strings("For", node->node_type)) {
		if (!(compare_strings(node->son->node_type, "Block")) && node->son->primitive_type != _BOOL) {
			node->error_type = _IMCOMP_TYPE;
			semantic_error++;
		}
	}

	else if (compare_strings("If", node->node_type)) {
		if (node->son->primitive_type != _BOOL) {
			node->error_type = _IMCOMP_TYPE;
			semantic_error++;
		}
	}

	else if (compare_strings("IntLit", node->node_type)) {
		node->symbol_scope = _INTLIT;
		if (node->value[0] == '0' && node->value[1] != 'X' && node->value[1] != 'x') {
			int i;
			for (i = 1; i < strlen(node->value); i++) {
				if (char_to_int(node->value[i]) > 7) {
					node->error_type = _INV_OCTAL;
					semantic_error++;
					break;
				}
			}
		}
	}

	else if (compare_strings("RealLit", node->node_type)) {
		node->symbol_scope = _REALLIT;
	}

	else if (compare_strings("StrLit", node->node_type)) {
		node->symbol_scope = _STRLIT;
	}

	else if (compare_strings("Add", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type == _INT || temp_type == _FLOAT32 || temp_type == _STRING) {
			node->primitive_type = temp_type;

		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Sub", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type == _INT || temp_type == _FLOAT32) {
			node->primitive_type = temp_type;

		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Mul", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type == _INT || temp_type == _FLOAT32) {
			node->primitive_type = temp_type;

		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Div", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type == _INT || temp_type == _FLOAT32) {
			node->primitive_type = temp_type;

		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Mod", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type == _INT) {
			node->primitive_type = temp_type;

		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("And", node->node_type)) {
		node->symbol_scope = _OPERATION;
		if (parse_binary_operator_type(node) != _BOOL) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Or", node->node_type)) {
		node->symbol_scope = _OPERATION;
		if (parse_binary_operator_type(node) != _BOOL) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Not", node->node_type)) {
		node->symbol_scope = _OPERATION;
		if (node->son->primitive_type != _BOOL) {
			node->error_type = _TYPE_MISMATCH_UNR;
			semantic_error++;
		}
	}

	else if (compare_strings("Gt", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Lt", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Eq", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING && temp_type != _BOOL) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Ne", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING && temp_type != _BOOL) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Le", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Ge", node->node_type)) {
		node->symbol_scope = _OPERATION;
		type temp_type = parse_binary_operator_type(node);

		if (temp_type != _INT && temp_type != _FLOAT32 && temp_type != _STRING) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Minus", node->node_type)) {
		node->symbol_scope = _OPERATION;
		if (node->son->primitive_type == _INT || node->son->primitive_type == _FLOAT32) {
			node->primitive_type = node->son->primitive_type;
		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_UNR;
			semantic_error++;
		}
	}

	else if (compare_strings("Plus", node->node_type)) {
		node->symbol_scope = _OPERATION;
		if (node->son->primitive_type == _INT || node->son->primitive_type == _FLOAT32) {
			node->primitive_type = node->son->primitive_type;
		}
		else {
			node->primitive_type = _UNDEF;
			node->error_type = _TYPE_MISMATCH_UNR;
			semantic_error++;
		}
	}


	else if (compare_strings("Call", node->node_type)) {
		node->symbol_scope = _OPERATION;
		node->primitive_type = node->son->primitive_type;
	}

	else if (compare_strings("ParseArgs", node->node_type)) {
		if (parse_binary_operator_type(node) != _INT) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Assign", node->node_type)) {
		node->primitive_type = parse_binary_operator_type(node);
		if (node->primitive_type == _UNDEF) {
			node->error_type = _TYPE_MISMATCH_BIN;
			semantic_error++;
		}
	}

	else if (compare_strings("Return", node->node_type)) {
		if (current_function_symbol_table->function_type != _VOID) {
			if (node->son != NULL) {
				if (current_function_symbol_table->function_type != node->son->primitive_type) {
					node->error_type = _IMCOMP_TYPE;
					semantic_error++;
				}
			}
			else {
				node->error_type = _IMCOMP_TYPE;
				semantic_error++;
			}
		}
		else if (node->son != NULL) {
			node->error_type = _IMCOMP_TYPE;
			semantic_error++;
		}
	}
}

void check_unused_variables(ast_node * node) {
	if (node == NULL) return;

	if (compare_strings(node->node_type, "VarDecl")) {
		if (node->is_used == false) {
			if (node->error_type == _NO_ERR) {
				node->error_type = _UNUSED;
				semantic_error++;
			}
		}
	}

	if (node->son != NULL) {
		if (!(compare_strings(node->node_type, "FuncDecl")) || node->error_type != _REDEC) {
			check_unused_variables(node->son);
		}
	}

	if (node->brother != NULL) {
		check_unused_variables(node->brother);
	}
}

void print_semantic_errors(ast_node *node) {
	if (node == NULL) return;

	switch (node->error_type) {
		case _REDEC:
			printf("Line %d, column %d: Symbol ", node->position.line, node->position.col);
			if (compare_strings(node->node_type, "FuncDecl")) {
				printf("%s ", node->son->son->value);
			}
			else { printf("%s ", node->son->brother->value); }
			printf("already defined\n");
			break;

		case _INV_OCTAL:
			printf("Line %d, column %d: Invalid octal constant: ", node->position.line, node->position.col);
			printf("%s\n", node->value);
			break;

		case _UNUSED:
			printf("Line %d, column %d: Symbol ", node->position.line, node->position.col);
			if (compare_strings(node->node_type, "VarDecl")) {
				printf("%s ", node->son->brother->value);
			}
			printf("declared but never used\n");
			break;

		case _NDEF:
			printf("Line %d, column %d: Cannot find symbol ", node->position.line, node->position.col);
			if (node->is_function_id) {
				printf("%s(", node->value);
				ast_node *temp;
				list_iterator(temp, node->brother, brother) {
					printf("%s", type_to_string(temp->primitive_type));
					if (temp->brother) printf(",");
				}
				printf(")\n");
			}
			else {
				printf("%s\n", node->value);
			}
			break;

		case _IMCOMP_TYPE:
			printf("Line %d, column %d: Incompatible type ", node->position.line, node->position.col);
			char *tempString = strdup(node->node_type);
			tempString[0] = tolower(tempString[0]);
			if (node->son == NULL) { printf("none in %s statement\n", tempString); }
			else { printf("%s in %s statement\n", type_to_string(node->son->primitive_type), tempString); }
			free(tempString);
			break;

		case _TYPE_MISMATCH_UNR:
			printf("Line %d, column %d: Operator ", node->position.line, node->position.col);
			printf("%s ", operator_to_string(node->node_type));
			printf("cannot be applied to type %s\n", type_to_string(node->son->primitive_type));
			break;

		case _TYPE_MISMATCH_BIN:
			printf("Line %d, column %d: Operator ", node->position.line, node->position.col);
			printf("%s ", operator_to_string(node->node_type));
			printf(
					"cannot be applied to types %s, %s\n", type_to_string(node->son->primitive_type),
					type_to_string(node->son->brother->primitive_type));
			break;

		default:
			break;
	}

	if (node->son != NULL) {
		if (!(compare_strings(node->node_type, "FuncDecl")) || node->error_type != _REDEC) {
			print_semantic_errors(node->son);
		}
	}

	if (node->brother != NULL) {
		print_semantic_errors(node->brother);
	}
}
