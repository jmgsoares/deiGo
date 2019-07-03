#include "gocompiler_ast.h"

ast_node *create_root_node(char *node_type, positioned_token *value) {
	ast_node *new_node = (ast_node *) malloc(sizeof(ast_node));

	if (node_type) new_node->node_type = (char *) strdup(node_type);
	else new_node->node_type = NULL;
	new_node->value = NULL;
	if (value) {
		if (compare_strings(node_type, "Int") || compare_strings(node_type, "Float32") || compare_strings(node_type,
		                                                                                                  "Bool")
		                                                                               || compare_strings(node_type,
		                                                                                                  "RealLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "IntLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "String")
		                                                                               || compare_strings(node_type,
		                                                                                                  "Id")
		                                                                               || compare_strings(node_type,
		                                                                                                  "StrLit")) {
			if (value->value) new_node->value = (char *) strdup(value->value);
		}
		new_node->position = value->p;
	}
	new_node->is_used = false;
	new_node->function_symbol_table = NULL;
	new_node->son = NULL;
	new_node->error_type = _NO_ERR;
	new_node->is_function_id = false;
	new_node->primitive_type = _NULL;
	new_node->brother = NULL;
	new_node->temp_id = -1;
	return new_node;
}

ast_node *create_node(char *node_type, positioned_token *value) {
	ast_node *new_node = (ast_node *) malloc(sizeof(ast_node));

	if (node_type) new_node->node_type = (char *) strdup(node_type);
	else new_node->node_type = NULL;
	new_node->value = NULL;
	if (value) {
		if (compare_strings(node_type, "Int") || compare_strings(node_type, "Float32") || compare_strings(node_type,
		                                                                                                  "Bool")
		                                                                               || compare_strings(node_type,
		                                                                                                  "RealLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "IntLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "String")
		                                                                               || compare_strings(node_type,
		                                                                                                  "Id")
		                                                                               || compare_strings(node_type,
		                                                                                                  "StrLit")) {
			if (value->value) new_node->value = (char *) strdup(value->value);
		}
		new_node->position = value->p;
	}
	new_node->son = NULL;
	new_node->function_symbol_table = NULL;
	new_node->is_used = false;
	new_node->error_type = _NO_ERR;
	new_node->is_function_id = false;
	new_node->temp_id = -1;
	new_node->primitive_type = _NULL;
	new_node->brother = NULL;
	return new_node;
}

ast_node *create_node_with_position_from_node(char *node_type, ast_node *node) {
	ast_node *new_node = (ast_node *) malloc(sizeof(ast_node));

	if (node_type) new_node->node_type = (char *) strdup(node_type);
	else new_node->node_type = NULL;
	new_node->value = NULL;
	new_node->position = node->position;
	new_node->son = NULL;
	new_node->function_symbol_table = NULL;
	new_node->is_used = false;
	new_node->error_type = _NO_ERR;
	new_node->is_function_id = false;
	new_node->temp_id = -1;
	new_node->primitive_type = _NULL;
	new_node->brother = NULL;
	return new_node;
}

ast_node *create_node_with_type(char *node_type, char *primitive_type, positioned_token *value) {
	ast_node *new_node = (ast_node *) malloc(sizeof(ast_node));

	if (node_type) new_node->node_type = (char *) strdup(node_type);
	else new_node->node_type = NULL;
	new_node->value = NULL;
	if (value) {
		if (compare_strings(node_type, "Int") || compare_strings(node_type, "Float32") || compare_strings(node_type,
		                                                                                                  "Bool")
		                                                                               || compare_strings(node_type,
		                                                                                                  "RealLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "IntLit")
		                                                                               || compare_strings(node_type,
		                                                                                                  "String")
		                                                                               || compare_strings(node_type,
		                                                                                                  "Id")
		                                                                               || compare_strings(node_type,
		                                                                                                  "StrLit")) {
			if (value->value) new_node->value = (char *) strdup(value->value);
		}
		new_node->position = value->p;
	}
	new_node->primitive_type = string_to_type(primitive_type);
	new_node->is_function_id = new_node->primitive_type == _TODO_FUNC ? true : false;
	new_node->son = NULL;
	new_node->temp_id = -1;
	new_node->function_symbol_table = NULL;
	new_node->error_type = _NO_ERR;
	new_node->is_used = false;
	new_node->brother = NULL;
	return new_node;
}

char *get_function_name_from_funcdecl_node(ast_node *funcdecl_ast_node) {
	return funcdecl_ast_node->son->son->value;
}

void insert_sibling(ast_node *sibling, ast_node *new_sibling) {
	if (sibling == NULL || new_sibling == NULL) return;
	ast_node *aux = sibling;
	while (aux->brother != NULL) {
		aux = aux->brother;
	}
	aux->brother = new_sibling;
}

void insert_son(ast_node *parent, ast_node *son) {
	if (parent == NULL || son == NULL) return;
	if (parent->son == NULL) {
		parent->son = son;
	}
}

void print_dots(int depth) {
	char depth_transformation[2 * depth + 1];
	printf("%.*s", 2 * depth, (char *) memset(depth_transformation, (int) '.', 2 * depth));
}

void print_ast(ast_node *n, int depth) {
	if (n == NULL) return;
	print_dots(depth);
	if (n->value != NULL) printf("%s(%s)\n", n->node_type, n->value);
	else printf("%s\n", n->node_type);
	if (n->son != NULL) {
		print_ast(n->son, depth + 1);
	}
	if (n->brother != NULL) {
		print_ast(n->brother, depth);
	}
}

void print_annotated_ast(ast_node *n, int depth) {
	if (n == NULL) return;

#ifdef DEBUGAAST
	if (n->primitive_type != _NULL) printf("%s\t\t", type_to_string(n->primitive_type));
	else printf("null\t\t");


	if (n->error_type == _TYPE_MISMATCH_BIN) printf("misB\t");
	else if (n->error_type == _TYPE_MISMATCH_UNR) printf("misU\t");
	else if (n->error_type == _REDEC) printf("rdec\t");
	else if (n->error_type == _NDEF) printf("ndef\t");
	else if (n->error_type == _INV_OCTAL) printf("IOct\t");
	else if (n->error_type == _IMCOMP_TYPE) printf("incT\t");
	else if (compare_strings(n->node_type, "VarDecl") || compare_strings(n->node_type, "FuncDecl")) {
		if (n->is_used == false) printf("nUse\t");
		else printf("\t");
	}
	else printf("\t");

	if (n->position.col) printf("(l:%d,c:%d)", n->position.line, n->position.col);
	else printf("\t");
	printf("\t");
#endif

	print_dots(depth);
	if (n->value) printf("%s(%s)", n->node_type, n->value);
	else printf("%s", n->node_type);

	if (compare_strings(n->node_type, "Id")) {
		if (n->is_function_id) {
			parameter_list_node *temp;
			printf(" - (");
			list_iterator(temp, n->function_parameters, next) {
				printf("%s", type_to_string(temp->parameter_type));
				if (temp->next) printf(",");
			}
			printf(")");
		}
		else {
			if (n->primitive_type != _NULL && n->primitive_type != _TODO_VAR_STMNT)
				printf(" - %s", type_to_string(n->primitive_type));
		}
	}
	else if (compare_strings(n->node_type, "Call")) {
		if (n->primitive_type != _VOID)
			printf(" - %s", type_to_string(n->primitive_type));
	}
	else if (n->primitive_type != _NULL && n->primitive_type != _TODO_VAR_STMNT)
		printf(" - %s", type_to_string(n->primitive_type));

	printf("\n");
	if (n->son != NULL) {
		print_annotated_ast(n->son, depth + 1);
	}
	if (n->brother != NULL) {
		print_annotated_ast(n->brother, depth);
	}
}
