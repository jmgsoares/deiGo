#include "gocompiler_code_gen.h"
#include "gocompiler_structures.h"

char *primitive_char_type_to_default_value(char *node_type) {
	if (compare_strings(node_type, "Int")) {
		return "i32 0\n";
	}
	else if (compare_strings(node_type, "Bool")) {
		return "i1 0\n";
	}
	else if (compare_strings(node_type, "Float32")) {
		return "double 0.0\n";
	}
	else if (compare_strings(node_type, "String")) {
		return "i8* getelementptr inbounds ([1 x i8], [1 x i8]* @.emptyString, i32 0, i32 0)\n";
	}
	return "\0";
}

char *primitive_type_to_char(type t) {
	switch (t) {
		case _VOID:
			return "i32";
		case _INT:
			return "i32";
		case _FLOAT32:
			return "double";
		case _BOOL:
			return "i1";
		case _STRING:
			return "i8*";
		default:
			return "\0";
	}
}

char *primitive_type_print(type t) {
	switch (t) {
		case _INT:
			return " ([4 x i8] , [4 x i8]* @.printInt";
		case _FLOAT32:
			return " ([7 x i8] , [7 x i8]* @.printFloat";
		case _STRING:
			return " @.printString";
		default:
			return "\0";
	}
}

void generate_llvmir(ast_node *node, symbol_table_list_head *symbol_table) {
	label_counter = 0;
	string_count = 0;
	generate_includes();
	printf(";Static String Literals\n");
	generate_static_strings(node);
	printf(";Global Variables\n");
	parse_global_variables(node->son, symbol_table);
	printf(";Program Functions\n");
	parse_functions(node->son, symbol_table);
}

void generate_includes() {
	printf(";Defines\n");
	printf("declare i32 @printf(i8*, ...)\n");
	printf("declare i32 @atoi(i8*)\n");
	printf("declare i32 @puts(i8*)\n");
	printf(";Print Formatting Strings\n");
	printf("@.printFloat = private unnamed_addr constant [7 x i8] c\"\\25\\2E\\30\\38\\66\\0A\\00\"\n");
	printf("@.printInt = private unnamed_addr constant [4 x i8] c\"\\25\\64\\0A\\00\"\n");
	printf("@.emptyString = private unnamed_addr constant [1 x i8] c\"\\00\"\n");
	printf("@.trueString = private unnamed_addr constant [6 x i8] c\"true\\0A\\00\"\n");
	printf("@.falseString = private unnamed_addr constant [7 x i8] c\"false\\0A\\00\"\n");
	printf(";Null values for initializations\n");
	printf("@.nullI32 = global i32 0\n");
	printf("@.nullI1 = global i1 0\n");
	printf("@.nullDouble = global double 0.0\n");
}

void handle_string(ast_node *node) {
	char *string = node->value;
	int str_size = strlen(node->value);
	int i, newStringPos = 0, new_string_len = 0;
	char *new_string = malloc(((str_size/2)+1)*3 + 1);
	for(i = 1; i < str_size - 1; i++) {
		if (string[i] == '\\') {
			if(string[i+1] == 'f' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '0';
				newStringPos++;
				new_string[newStringPos] = 'C';
				newStringPos++;
				new_string_len++;
			}
			else if(string[i+1] == 'n' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '0';
				newStringPos++;
				new_string[newStringPos] = 'A';
				newStringPos++;
				new_string_len++;
			}
			else if(string[i+1] == 'r' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '0';
				newStringPos++;
				new_string[newStringPos] = 'D';
				newStringPos++;
				new_string_len++;
			}
			else if(string[i+1] == 't' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '0';
				newStringPos++;
				new_string[newStringPos] = '9';
				newStringPos++;
				new_string_len++;
			}
			else if(string[i+1] == '\\' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '5';
				newStringPos++;
				new_string[newStringPos] = 'C';
				newStringPos++;
				new_string_len++;
			}
			else if(string[i+1] == '"' ) {
				new_string[newStringPos] = '\\';
				newStringPos++;
				new_string[newStringPos] = '2';
				newStringPos++;
				new_string[newStringPos] = '2';
				newStringPos++;
				new_string_len++;
			}
			i++;
		}
		else {
			new_string[newStringPos] = string[i];
			newStringPos++;
			new_string_len++;
		}
	}
	new_string[newStringPos] = '\\';
	newStringPos++;
	new_string[newStringPos] = '0';
	newStringPos++;
	new_string[newStringPos] = '0';
	newStringPos++;
	new_string_len++;
	new_string[newStringPos] = 0x0;
	free(node->value);
	node->value = new_string;
	node->value_size = new_string_len;
}

void generate_static_strings(ast_node *node) {
	if (node == NULL) return;

	if (node->son != NULL) {
		generate_static_strings(node->son);
	}
	if (node->brother != NULL) {
		generate_static_strings(node->brother);
	}
	if (compare_strings("StrLit", node->node_type)) {
		handle_string(node);
		string_count++;
		node->temp_id = string_count;
		printf("@.str.%d = private unnamed_addr constant ", string_count);
		printf("[%d x i8] c\"%s\"\n", node->value_size, node->value);
	}
}

void parse_global_variables(ast_node *node, symbol_table_list_head *symbol_table) {
	if (node == NULL) return;

	if (compare_strings("VarDecl", node->node_type)) {
		generate_global_variable(node);
	}
	if (node->brother != NULL) {
		parse_global_variables(node->brother, symbol_table);
	}
}

void generate_global_variable(ast_node *node) {
	printf("@%s = global %s", node->son->brother->value, primitive_char_type_to_default_value(node->son->node_type));
}

void parse_functions(ast_node *node, symbol_table_list_head *symbol_table) {
	if (node == NULL) return;

	if (compare_strings("FuncDecl", node->node_type)) {
		temp_counter = 0;
		parse_function_header(node->function_symbol_table, node->son->brother);
		parse_function_body(node->son->brother, node->function_symbol_table);
		generate_function_footer(node->function_symbol_table);
	}
	else if (node->son != NULL) {
		parse_functions(node->son, symbol_table);
	}

	if (node->brother != NULL) {
		parse_functions(node->brother, symbol_table);
	}
}

void generate_function_prototype(symbol_table_list_head *symbol_table) {
	char *func_name = symbol_table->function_name;
	char *func_type = primitive_type_to_char(symbol_table->function_type);
	printf("define %s @%s(", func_type, func_name);
	if (compare_strings(func_name, "main")) {
		printf("i32 %%argc, i8** %%argv");
	}
	else if (symbol_table->function_parameters) {
		parameter_list_node *temp;
		list_iterator(temp, symbol_table->function_parameters, next) {
			temp_counter++;
			temp->temp_id = temp_counter;
			char *var_type = primitive_type_to_char(temp->parameter_type);
			char *var_name = temp->parameter_name;
			printf("%s %%%s", var_type, var_name);
			if (temp->next) printf(", ");
		}
	}
	printf(") {\n");
}

void parse_function_header(symbol_table_list_head *symbol_table, ast_node *node) {
	generate_function_prototype(symbol_table);

	if(compare_strings(symbol_table->function_name, "main")) {
		temp_counter++;
		printf("%%%d = alloca i32\n", temp_counter);
		printf("store i32 %%argc, i32* %%%d\n", temp_counter);
		temp_counter++;
		printf("%%%d = alloca i8**\n", temp_counter);
		printf("store i8** %%argv, i8*** %%%d\n", temp_counter);
	}

	if (symbol_table->function_parameters) {
		parameter_list_node *temp;
		list_iterator(temp, symbol_table->function_parameters, next) {
			generate_parameter_variable(temp);
		}
	}
	if (symbol_table->function_symbols) {
		symbol_table_list_node *temp;
		list_iterator(temp, symbol_table->function_symbols, next) {
			temp_counter++;
			generate_local_variable(temp);
		}
	}
	generate_int_float_literals(node);
}

void generate_int_float_literals(ast_node* node) {
	if (node == NULL) return;

	if (node->son != NULL) {
		generate_int_float_literals(node->son);
	}
	if (node->brother != NULL) {
		generate_int_float_literals(node->brother);
	}
	if (compare_strings("IntLit", node->node_type) || compare_strings("IntLit", node->node_type)) {
		generate_variable_load(node);
	}
}

void generate_parameter_variable(parameter_list_node *param) {
	char *var_type = primitive_type_to_char(param->parameter_type);
	char *var_name = param->parameter_name;
	printf("%%%d = alloca %s\n", param->temp_id, var_type);
	printf("store %s %%%s, %s* %%%d\n", var_type, var_name, var_type, param->temp_id);
}

void generate_local_variable(symbol_table_list_node *symbol) {
	if (symbol->symbol_type != _STRING) {
		symbol->temp_id = temp_counter;
		char *var_type = primitive_type_to_char(symbol->symbol_type);
		printf("%%%d = alloca %s\n", symbol->temp_id, var_type);
		printf("store %s ", var_type);
		if (symbol->symbol_type == _INT || symbol->symbol_type == _BOOL) {
			printf("0");
		}
		else if (symbol->symbol_type == _FLOAT32) {
			printf("0.0");
		}
		printf(", %s* %%%d\n", var_type, symbol->temp_id);
	}
	else {
		symbol->temp_id = temp_counter;
		printf("%%%d = getelementptr [1 x i8], [1 x i8]* @.emptyString, i64 0, i64 0\n", symbol->temp_id);
	}
}

void parse_function_body(ast_node *node, symbol_table_list_head *symbol_table) {
	if (node == NULL) return;

	if (compare_strings("If", node->node_type)) {
		generate_if(node, NULL);
	}

	else if (compare_strings("For", node->node_type)) {
		generate_for(node, NULL);
	}

	else if (node->son != NULL) {
		parse_function_body(node->son, symbol_table);
	}

	if (compare_strings("Print", node->node_type)) {
		generate_print(node);
	}

	else if (compare_strings("Minus", node->node_type)) {
		generate_minus(node);
	}

	else if (compare_strings("Plus", node->node_type)) {
		generate_plus(node);
	}

	else if (compare_strings("Not", node->node_type)) {
		generate_not(node);
	}

	else if (compare_strings("Assign", node->node_type)) {
		generate_assign(node);
	}

	else if (compare_strings("Sub", node->node_type)) {
		generate_sub(node);
	}

	else if (compare_strings("Add", node->node_type)) {
		generate_add(node);
	}

	else if (compare_strings("Div", node->node_type)) {
		generate_div(node);
	}

	else if (compare_strings("Mod", node->node_type)) {
		generate_mod(node);
	}

	else if (compare_strings("Mul", node->node_type)) {
		generate_mul(node);
	}

	else if (compare_strings("And", node->node_type)) {
		generate_and(node);
	}

	else if (compare_strings("Or", node->node_type)) {
		generate_or(node);
	}

	else if (compare_strings("Eq", node->node_type)) {
		generate_eq(node);
	}

	else if (compare_strings("Ne", node->node_type)) {
		generate_ne(node);
	}

	else if (compare_strings("Lt", node->node_type)) {
		generate_lt(node);
	}

	else if (compare_strings("Le", node->node_type)) {
		generate_le(node);
	}

	else if (compare_strings("Gt", node->node_type)) {
		generate_gt(node);
	}

	else if (compare_strings("Ge", node->node_type)) {
		generate_ge(node);
	}

	else if (compare_strings("Call", node->node_type)) {
        generate_call(node);
    }

	else if(compare_strings("Return", node->node_type)){
		generate_return(node);
    }

	else if (compare_strings("ParseArgs", node->node_type)) {
		generate_parse_args(node);
	}

	if (node->brother != NULL) {
		parse_function_body(node->brother, symbol_table);
	}

}

void parse_if_for_expressions(ast_node *node, symbol_table_list_head *symbol_table) {
	if (node == NULL) return;

	if (node->son != NULL) {
		parse_if_for_expressions(node->son, symbol_table);
	}

	if (compare_strings("Minus", node->node_type)) {
		generate_minus(node);
	}

	else if (compare_strings("Plus", node->node_type)) {
		generate_plus(node);
	}

	else if (compare_strings("Not", node->node_type)) {
		generate_not(node);
	}

	else if (compare_strings("Sub", node->node_type)) {
		generate_sub(node);
	}

	else if (compare_strings("Add", node->node_type)) {
		generate_add(node);
	}

	else if (compare_strings("Div", node->node_type)) {
		generate_div(node);
	}

	else if (compare_strings("Mod", node->node_type)) {
		generate_mod(node);
	}

	else if (compare_strings("Mul", node->node_type)) {
		generate_mul(node);
	}

	else if (compare_strings("And", node->node_type)) {
		generate_and(node);
	}

	else if (compare_strings("Or", node->node_type)) {
		generate_or(node);
	}

	else if (compare_strings("Eq", node->node_type)) {
		generate_eq(node);
	}

	else if (compare_strings("Ne", node->node_type)) {
		generate_ne(node);
	}

	else if (compare_strings("Lt", node->node_type)) {
		generate_lt(node);
	}

	else if (compare_strings("Le", node->node_type)) {
		generate_le(node);
	}

	else if (compare_strings("Gt", node->node_type)) {
		generate_gt(node);
	}

	else if (compare_strings("Ge", node->node_type)) {
		generate_ge(node);
	}

	else if (compare_strings("Call", node->node_type)) {
		generate_call(node);
	}

	if (node->brother != NULL && !(compare_strings(node->brother->node_type, "Block"))) {
		parse_if_for_expressions(node->brother, symbol_table);
	}
}

void generate_function_footer(symbol_table_list_head *symbol_table) {
	temp_counter++;
	if (symbol_table->function_type == _INT || symbol_table->function_type == _VOID) {
		printf("%%%d = load i32, i32* @.nullI32\n", temp_counter);
		printf("ret i32 %%%d\n", temp_counter);
	}
	else if (symbol_table->function_type == _BOOL) {
		printf("%%%d = load i1, i1* @.nullI1\n", temp_counter);
		printf("ret i1 %%%d\n", temp_counter);
	}
	else if (symbol_table->function_type == _FLOAT32) {
		printf("%%%d = load double, double* @.nullDouble\n", temp_counter);
		printf("ret double %%%d\n", temp_counter);
	}
	else if (symbol_table->function_type == _STRING) {
		printf("%%%d = getelementptr [1 x i8], [1 x i8]* @.emptyString, i64 0, i64 0\n", temp_counter);
		printf("ret i8* %%%d\n", temp_counter);
	}
	printf("}\n");
}


void generate_variable_load(ast_node *node) {
	if (node->temp_id != -1) return;
	if (!(compare_strings(node->node_type, "IntLit")) && !(compare_strings(node->node_type, "RealLit"))) {
		char *var_type = primitive_type_to_char(node->primitive_type);
		temp_counter++;
		node->temp_id = temp_counter;
		if (node->primitive_type != _STRING) {
			if (node->symbol_scope == _LOCAL) {
				printf(
						"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type,
						node->symbol_table_entry.symbol->temp_id
				);
			}
			else if (node->symbol_scope == _PARAM) {
				printf(
						"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type,
						node->symbol_table_entry.parameter->temp_id
				);
			}
			else if (node->symbol_scope == _GLOBAL) {
				printf("%%%d = load %s, %s* @%s\n", temp_counter, var_type, var_type, node->value);
			}
		}
		else {
			printf("%%%d = getelementptr [1 x i8], [1 x i8]* @.emptyString, i64 0, i64 0\n", temp_counter);
			return;
		}
	}
	else {
		if(node->symbol_scope == _REALLIT) sprintf(node->value, "%.16e", strtod(node->value, NULL));
		temp_counter++;
		char *var_type = primitive_type_to_char(node->primitive_type);
		printf("%%%d = alloca %s\n", temp_counter, var_type);
		printf("store %s %s, %s* %%%d\n", var_type, node->value, var_type, temp_counter);
		printf("%%%d = load %s, %s* %%%d\n", temp_counter + 1, var_type, var_type, temp_counter);
		temp_counter++;
		node->temp_id = temp_counter;
	}
}

void generate_print(ast_node *node) {
	if (node->son->primitive_type == _BOOL) {
		if (node->son->symbol_scope == _GLOBAL) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* @%s\n", temp_counter, var_type, var_type, node->son->value
			);
			temp_counter++;
			printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, temp_counter - 1);
			printf("br i1 %%%d, label %%%d, label %%%d\n", temp_counter, temp_counter + 1, temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[7 x i8], [7 x i8]* @.falseString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[6 x i8], [6 x i8]* @.trueString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 1);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _LOCAL) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type,
					node->son->symbol_table_entry.symbol->temp_id
			);
			temp_counter++;
			printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, temp_counter - 1);
			printf("br i1 %%%d, label %%%d, label %%%d\n", temp_counter, temp_counter + 1, temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[7 x i8], [7 x i8]* @.falseString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[6 x i8], [6 x i8]* @.trueString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 1);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _PARAM) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type,
					node->son->symbol_table_entry.parameter->temp_id
			);
			temp_counter++;
			printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, temp_counter - 1);
			printf("br i1 %%%d, label %%%d, label %%%d\n", temp_counter, temp_counter + 1, temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[7 x i8], [7 x i8]* @.falseString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[6 x i8], [6 x i8]* @.trueString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 1);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _OPERATION) {
			temp_counter++;
			printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, node->son->temp_id);
			printf("br i1 %%%d, label %%%d, label %%%d\n", temp_counter, temp_counter + 1, temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[7 x i8], [7 x i8]* @.falseString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 3);
			temp_counter += 2;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds (", temp_counter);
			printf("[6 x i8], [6 x i8]* @.trueString, i32 0, i32 0))\n");
			printf("br label %%%d\n", temp_counter + 1);
			temp_counter++;
		}
	}
	else if (node->son->primitive_type != _STRING) {
		if (node->son->symbol_scope == _GLOBAL) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			char *print_type = primitive_type_print(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* @%s\n", temp_counter, var_type, var_type, node->son->value
			);
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds", temp_counter + 1);
			printf("%s, i32 0, i32 0), %s %%%d)\n", print_type, var_type, temp_counter);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _PARAM) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			char *print_type = primitive_type_print(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type,
					node->son->symbol_table_entry.parameter->temp_id
			);
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds", temp_counter + 1);
			printf("%s, i32 0, i32 0), %s %%%d)\n", print_type, var_type, temp_counter);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _LOCAL) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			char *print_type = primitive_type_print(node->son->primitive_type);
			printf(
					"%%%d = load %s, %s* %%%d\n", temp_counter, var_type, var_type, node->son->symbol_table_entry.symbol->temp_id
			);
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds", temp_counter + 1);
			printf("%s, i32 0, i32 0), %s %%%d)\n", print_type, var_type, temp_counter);
			temp_counter++;
		}
		else if (node->son->symbol_scope == _OPERATION) {
			temp_counter++;
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			char *print_type = primitive_type_print(node->son->primitive_type);
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds", temp_counter);
			printf("%s, i32 0, i32 0), %s %%%d)\n", print_type, var_type, node->son->temp_id);
		}
		else if (node->son->symbol_scope == _STRLIT) {
			temp_counter++;
			printf("%%%d = call i32 (i8*) @puts(i8* getelementptr inbounds (", temp_counter);
			printf("[%d x i8], [%d x i8]* @.str.%d, i32 0, i32 0))\n", node->son->value_size, node->son->value_size, node->son->temp_id);
		}
		else if (node->son->symbol_scope == _INTLIT || node->son->symbol_scope == _REALLIT) {
			char *var_type = primitive_type_to_char(node->son->primitive_type);
			char *print_type = primitive_type_print(node->son->primitive_type);
			if (node->son->symbol_scope == _REALLIT) sprintf(node->son->value, "%.16e", strtod(node->son->value, NULL));
			temp_counter++;
			printf("%%%d = call i32 (i8*, ...) @printf(i8* getelementptr inbounds", temp_counter);
			printf("%s, i32 0, i32 0), %s %s)\n", print_type, var_type, node->son->value);
		}
	}
	else {
		temp_counter++;
		printf("%%%d = call i32 (i8*) @puts(i8* getelementptr inbounds (", temp_counter);
		printf("[1 x i8], [1 x i8]* @.emptyString, i32 0, i32 0))\n");
	}

}

void generate_minus(ast_node *node) {
	generate_variable_load(node->son);
	char *operator, *constant;
	char *type = primitive_type_to_char(node->son->primitive_type);
	if (node->son->primitive_type == _INT) {
		operator = "mul";
		constant = "-1";
	}
	else {
		node->primitive_type = _FLOAT32;
		operator = "fmul";
		constant = "-1.0";
	}

	temp_counter++;
	printf("%%%d = %s %s %%%d, %s\n", temp_counter, operator, type, node->son->temp_id, constant);
	node->temp_id = temp_counter;
}

void generate_not(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	temp_counter++;
	printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, temp_counter - 1);
	node->temp_id = temp_counter;
}

void generate_plus(ast_node *node) {
	generate_variable_load(node->son);
	char *operator, *constant;
	if (node->son->primitive_type == _INT) {
		operator = "mul";
		constant = "1";
	}
	else {
		node->primitive_type = _FLOAT32;
		operator = "fmul";
		constant = "1.0";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	temp_counter++;
	printf("%%%d = %s %s %%%d, %s\n", temp_counter, operator, type, node->son->temp_id, constant);
	node->temp_id = temp_counter;
}

void generate_sub(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	char *operator;
	if (node->primitive_type == _INT) { operator = "sub"; }
	else { operator = "fsub"; }
	temp_counter++;
	char *type = primitive_type_to_char(node->primitive_type);
	printf("%%%d = %s %s %%%d, %%%d\n", temp_counter, operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_add(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	char *operator;
	if (node->primitive_type == _INT) { operator = "add"; }
	else { operator = "fadd"; }
	temp_counter++;
	char *type = primitive_type_to_char(node->primitive_type);
	printf("%%%d = %s %s %%%d, %%%d\n", temp_counter, operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_div(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	char *operator;
	if (node->primitive_type == _INT) { operator = "sdiv"; }
	else { operator = "fdiv"; }
	temp_counter++;
	char *type = primitive_type_to_char(node->primitive_type);
	printf("%%%d = %s %s %%%d, %%%d\n", temp_counter, operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_mod(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator;
	if (node->primitive_type == _INT) { operator = "srem"; }
	else { operator = "frem"; }
	char *type = primitive_type_to_char(node->primitive_type);
	printf("%%%d = %s %s %%%d, %%%d\n", temp_counter, operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_mul(ast_node *node) {
	generate_variable_load(node->son);
	generate_variable_load(node->son->brother);

	temp_counter++;
	char *operator;
	if (node->primitive_type == _INT) { operator = "mul"; }
	else { operator = "fmul"; }
	char *type = primitive_type_to_char(node->primitive_type);
	printf("%%%d = %s %s %%%d, %%%d\n", temp_counter, operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_and(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	printf("%%%d = and i1 %%%d, %%%d\n", temp_counter, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_or(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	printf("%%%d = or i1 %%%d, %%%d", temp_counter, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_eq(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT || node->son->primitive_type == _BOOL) {
		operator = "icmp";
		sub_operator = "eq";
	}
	else {
		operator = "fcmp";
		sub_operator = "oeq";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_ne(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT || node->son->primitive_type == _BOOL) {
		operator = "icmp";
		sub_operator = "ne";
	}
	else {
		operator = "fcmp";
		sub_operator = "one";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;

}

void generate_lt(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT) {
		operator = "icmp";
		sub_operator = "slt";
	}
	else {
		operator = "fcmp";
		sub_operator = "olt";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_le(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT) {
		operator = "icmp";
		sub_operator = "sle";
	}
	else {
		operator = "fcmp";
		sub_operator = "ole";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_gt(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT) {
		operator = "icmp";
		sub_operator = "sgt";
	}
	else {
		operator = "fcmp";
		sub_operator = "ogt";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_ge(ast_node *node) {
	if(node->son->symbol_scope != _OPERATION) {
		generate_variable_load(node->son);
	}
	if (node->son->brother->symbol_scope != _OPERATION) {
		generate_variable_load(node->son->brother);
	}
	temp_counter++;
	char *operator, *sub_operator;
	if (node->son->primitive_type == _INT) {
		operator = "icmp";
		sub_operator = "sge";
	}
	else {
		operator = "fcmp";
		sub_operator = "oge";
	}
	char *type = primitive_type_to_char(node->son->primitive_type);
	printf("%%%d = %s %s %s %%%d, %%%d\n", temp_counter, operator, sub_operator, type, node->son->temp_id, node->son->brother->temp_id);
	node->temp_id = temp_counter;
}

void generate_parse_args(ast_node *node) {
	generate_variable_load(node->son);
	generate_variable_load(node->son->brother);
	temp_counter++;
	printf("%%%d = load i8**, i8*** %%2\n", temp_counter);
	temp_counter++;
	printf("%%%d = getelementptr inbounds i8*, i8** %%%d, i32 %%%d\n", temp_counter, temp_counter-1, node->son->brother->temp_id);
	temp_counter++;
	printf("%%%d = load i8*, i8** %%%d\n" , temp_counter, temp_counter - 1);
	temp_counter++;
	printf("%%%d = call i32 @atoi(i8* %%%d)\n", temp_counter, temp_counter - 1);
	if(node->son->symbol_scope == _GLOBAL) {
		printf("store i32 %%%d, i32* @%s\n", temp_counter, node->son->value);
	}
	else if(node->son->symbol_scope == _PARAM) {
		printf("store i32 %%%d, i32* %%%d\n", temp_counter, node->son->symbol_table_entry.parameter->temp_id);
	}
	else if(node->son->symbol_scope == _LOCAL) {
		printf("store i32 %%%d, i32* %%%d\n", temp_counter, node->son->symbol_table_entry.symbol->temp_id);
	}
}

void generate_if(ast_node *node, symbol_table_list_head *symbol_table) {
	parse_if_for_expressions(node->son, symbol_table);
	generate_variable_load(node->son);
	temp_counter++;
	int label = label_counter;
	label_counter++;
	printf("%%%d = icmp eq i1 %%%d, 0\n", temp_counter, node->son->temp_id);
	printf("br i1 %%%d, label %%else%d, label %%if%d\n", temp_counter, label, label);
	printf("else%d:\n", label);
	if (node->son->brother->brother->son != NULL) {
		parse_function_body(node->son->brother->brother->son, symbol_table);
	}
	printf("br label %%endIf%d\n", label);
	printf("if%d:\n", label);
	parse_function_body(node->son->brother->son, symbol_table);
	printf("br label %%endIf%d\n", label);
	printf("endIf%d:\n", label);
}

void generate_for(ast_node *node, symbol_table_list_head *symbol_table) {
	int label = label_counter;
	label_counter++;
	printf("br label %%for%d\n", label);
	printf("for%d:\n", label);
	parse_if_for_expressions(node->son, symbol_table);
	generate_variable_load(node->son);
	temp_counter++;
	printf("%%%d = icmp eq i1 %%%d, 1\n", temp_counter, node->son->temp_id);
	printf("br i1 %%%d, label %%forBlock%d, label %%endFor%d\n", temp_counter, label, label);
	printf("forBlock%d:\n", label);
	parse_function_body(node->son->brother->son, symbol_table);
	printf("br label %%for%d\n", label);
	printf("endFor%d:\n", label);
}

void generate_call(ast_node *node) {
    char *function_type = primitive_type_to_char(node->primitive_type);
    char *function_name = node->son->value;
	ast_node *temp;
	list_iterator(temp, node->son->brother, brother) {
		generate_variable_load(temp);
	}
	temp_counter++;
    printf("%%%d = call %s @%s(",temp_counter,function_type,function_name);

    list_iterator(temp, node->son->brother, brother) {
        char *var_type = primitive_type_to_char(temp->primitive_type);
	    if(temp->symbol_scope == _INTLIT || temp->symbol_scope == _REALLIT){
		    printf("%s %s", var_type, temp->value);
	    }
	    else {
		    printf("%s %%%d", var_type, temp->temp_id);
	    }

        if (temp->brother) printf(", ");
    }
    printf(")\n");
    node->temp_id = temp_counter;
}

void generate_return(ast_node *node) {

	if(node->son) {
		char *variable_type = primitive_type_to_char(node->son->primitive_type);
		if(node->son->symbol_scope == _INTLIT || node->son->symbol_scope == _REALLIT){
			temp_counter++;
			if (node->son->symbol_scope == _REALLIT) {
				sprintf(node->son->value, "%.16e", strtod(node->son->value, NULL));
			}
			printf("ret %s %s\n", variable_type, node->son->value);
		}
		else if(node->son->symbol_scope == _GLOBAL){
				generate_variable_load(node->son);
				temp_counter++;
				printf("ret %s @%s\n", variable_type, node->son->value);
		}
		else if(node->son->symbol_scope == _PARAM){
			generate_variable_load(node->son);
			temp_counter++;
			printf("ret %s %%%d\n", variable_type, node->son->temp_id);
		}
		else if(node->son->symbol_scope == _LOCAL){
			generate_variable_load(node->son);
			temp_counter++;
			printf("ret %s %%%d\n", variable_type, node->son->temp_id);
		}
		else if(node->son->symbol_scope == _OPERATION){
			temp_counter++;
			printf("ret %s %%%d\n", variable_type, node->son->temp_id);
		}
	}
}

void generate_assign(ast_node *node) {
	scope var_scope = node->son->brother->symbol_scope;
	if (var_scope == _REALLIT || var_scope == _INTLIT) {
		if (var_scope == _REALLIT) sprintf(node->son->brother->value, "%.16e", strtod(node->son->brother->value, NULL));
		if (node->son->symbol_scope == _GLOBAL) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf("store %s %s, %s* @%s\n", type, node->son->brother->value, type, node->son->value);
		}
		else if (node->son->symbol_scope == _PARAM) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"store %s %s, %s* %%%d\n", type, node->son->brother->value, type,
					node->son->symbol_table_entry.parameter->temp_id
			);
		}
		else if (node->son->symbol_scope == _LOCAL) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"store %s %s, %s* %%%d\n", type, node->son->brother->value, type,
					node->son->symbol_table_entry.symbol->temp_id
			);
		}
	}
	else {
		if (var_scope != _OPERATION) generate_variable_load(node->son->brother);
		if (node->son->symbol_scope == _GLOBAL) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"store %s %%%d, %s* @%s\n", type, node->son->brother->temp_id, type, node->son->value
			);
		}
		else if (node->son->symbol_scope == _PARAM) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"store %s %%%d, %s* %%%d\n", type, node->son->brother->temp_id, type,
					node->son->symbol_table_entry.parameter->temp_id
			);
		}
		else if (node->son->symbol_scope == _LOCAL) {
			char *type = primitive_type_to_char(node->son->primitive_type);
			printf(
					"store %s %%%d, %s* %%%d\n", type, node->son->brother->temp_id, type,
					node->son->symbol_table_entry.symbol->temp_id
			);
		}
	}
}
