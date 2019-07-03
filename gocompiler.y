%{
	#include <stdio.h>
	#include <stdbool.h>
	#include <string.h>
	#include "gocompiler_ast.h"
	#include "gocompiler_symtab.h"
	int yylex(void);
	void yyerror (const char *s);
	ast_node *temp_ast_node = NULL;
	ast_node *temp_ast_node_2 = NULL;
	extern ast_node *root;
	extern symbol_table_list_head *symbol_table_global;
	extern int line, col, syntactic_error;
%}

%union {
	struct positioned_token *charVal;
	struct ast_node *ast_node;
}

%token <charVal> PACKAGE ID SEMICOLON VAR LPAR RPAR COMMA INT FLOAT32 BOOL STRING FUNC LBRACE RBRACE ASSIGN IF ELSE REALLIT RESERVED
%token <charVal> FOR RETURN PRINT STRLIT BLANKID PARSEINT CMDARGS LSQ RSQ OR AND LT GT EQ NE GE LE PLUS MINUS STAR DIV MOD NOT INTLIT 
%type <ast_node> Program  Declarations VarDecSemiOrFuncDeclSemi VarDeclaration FuncDeclaration VarSpec CommaIdRep Type TypeOpt Parameters 
%type <ast_node> ParametersOpt ParseArgs CommaExprRep CommaIdTypeRep FuncBody FuncInvocation Statement Expr VarDeclarationOrStatementOpt
%type <ast_node>  VarsAndStatements ExprCommaExprRepOpt ExprOpt ExprOrStrlit Empty ElseStmntSemiRepOpt StatementSemicolonRep

%left COMMA
%right ASSIGN
%left OR
%left AND 
%left GE LE LT GT EQ NE
%left PLUS MINUS
%left STAR DIV MOD
%right UNARY
%left LPAR RPAR LSQ RSQ LBRACE RBRACE

%%

Program:			PACKAGE ID SEMICOLON Declarations
					{
						$$=root=create_root_node("Program", $1); 
						if($4 != NULL) insert_son($$, $4);
						free($2->value);
						free($2);
						free($1);
						free($3);
					}								
					;

Declarations: 		Empty
					{$$ = NULL;}

					| VarDecSemiOrFuncDeclSemi Declarations								
					{
						if($2 != NULL) {
							$$ = $1; 
							insert_sibling($1, $2);
						} 
						else {
							$$ = $1;		
						}
					}
					;
				
VarDecSemiOrFuncDeclSemi:													
					VarDeclaration SEMICOLON											
					{
						$$ = $1;
						free($2);
					}	
					
					| FuncDeclaration SEMICOLON											
					{
						$$ = $1;
						free($2);
					}			
					;

VarDeclaration:		VAR VarSpec
					{
						$$ = $2;
						free($1);
					}

					| VAR LPAR VarSpec SEMICOLON RPAR									
					{
						$$ = $3;
						free($1);
						free($2);
						free($4);
						free($5);
					}
					;

VarSpec: 			ID CommaIdRep Type
					{
						$$=temp_ast_node = create_node("VarDecl", $1);
						insert_son(temp_ast_node, $3);
						insert_sibling(temp_ast_node->son, create_node("Id", $1));
						insert_sibling(temp_ast_node, $2);

						for (temp_ast_node=$2; temp_ast_node != NULL; temp_ast_node = temp_ast_node->brother) {
							temp_ast_node->son->node_type = strdup($3->node_type);
						}
						free($1->value);
						free($1);
					}						
					;

CommaIdRep:			Empty
					{$$ = NULL;}

					| COMMA ID CommaIdRep
					{
						if($3 != NULL) {
														
							temp_ast_node = create_node("VarDecl", $2);
							insert_son(temp_ast_node, create_node(NULL, NULL));
							insert_sibling(temp_ast_node->son, create_node("Id", $2));
							insert_sibling(temp_ast_node, $3);

							$$ = temp_ast_node;
						} 
						else {
							$$ = create_node("VarDecl", $2);
							insert_son($$, create_node(NULL,NULL));
							insert_sibling($$->son, create_node("Id", $2));
						}
						free($2->value);
						free($2);
						free($1);
					}
					;

Type:				INT
					{
						$$ = create_node("Int", $1);
						free($1);
					}
					
					| FLOAT32															
					{
						$$ = create_node("Float32", $1);
						free($1);
					}
					
					| BOOL																
					{
						$$ = create_node("Bool", $1);
						free($1);
					}
					
					| STRING															
					{
						$$ = create_node("String", $1);
						free($1);
					}
					;

TypeOpt:			Empty
					{$$ = NULL;}

					| Type
					{$$ = $1;}
					;

FuncDeclaration:	FUNC ID LPAR ParametersOpt RPAR TypeOpt FuncBody
					{
						$$ = create_node("FuncDecl", $2);
						temp_ast_node = create_node("FuncHeader", NULL);
						insert_son(temp_ast_node, create_node("Id", $2));
						if ($6 != NULL) insert_sibling(temp_ast_node->son, $6);
						if ($4 != NULL) {
							temp_ast_node_2 = create_node("FuncParams", NULL);
							insert_son(temp_ast_node_2, $4);
							insert_sibling(temp_ast_node->son, temp_ast_node_2);
						} 
						else {
							insert_sibling(temp_ast_node->son, create_node("FuncParams", NULL));
						}
						insert_son($$, temp_ast_node);
						insert_sibling($$->son, $7);	
						free($2->value);	
						free($2);
						free($1);
						free($3);
						free($5);			
					}
					;

ParametersOpt:		Empty
					{$$ = NULL;}
					
					| Parameters
					{$$ = $1;}
					;

Parameters:			ID Type CommaIdTypeRep
					{
						if($3 != NULL) {
							temp_ast_node = create_node("ParamDecl", $1);
							insert_son(temp_ast_node, $2);
							insert_sibling(temp_ast_node->son, create_node("Id", $1));
							insert_sibling(temp_ast_node,$3);
							$$ = temp_ast_node;
						}
						else {
							$$ = create_node("ParamDecl", $1);
							insert_son($$, $2);
							insert_sibling($$->son, create_node("Id", $1));
						}	
						free($1->value);
						free($1);	
					}
					;

CommaIdTypeRep:		Empty
					{$$ = NULL;}
					
					| COMMA ID Type CommaIdTypeRep
					{
						if($4 != NULL) {

							temp_ast_node = create_node("ParamDecl", $2);
							insert_son(temp_ast_node, $3);
							insert_sibling(temp_ast_node->son, create_node("Id", $2));

							insert_sibling(temp_ast_node, $4);

							$$ = temp_ast_node;
						}
						else {

							$$ = create_node("ParamDecl", $2);
							insert_son($$, $3);
							insert_sibling($$->son, create_node("Id", $2));

						}
						free($2->value);
						free($2);
						free($1);	
					}

					;

FuncBody:			LBRACE VarsAndStatements RBRACE
					{
						$$ = create_node("FuncBody", NULL);
						insert_son($$, $2);
						free($1);
						free($3);
					}
					;

VarsAndStatements:	Empty
					{$$ = NULL;}
					
					| VarsAndStatements VarDeclarationOrStatementOpt SEMICOLON
					{
						if($1 != NULL) {
							$$ = $1;
							insert_sibling($1, $2);
						} else {
							$$ = $2;
						}
						free($3);
					}
					;

VarDeclarationOrStatementOpt:
					Empty																
					{$$ = NULL;}
					
					| VarDeclaration
					{$$ = $1;}
					
					| Statement
					{$$ = $1;}
					;

Statement:			ID ASSIGN Expr
					{
						$$ = create_node("Assign", $2);
						insert_son($$, create_node_with_type("Id", "Todo_var_stmnt", $1));
						insert_sibling($$->son, $3);
						free($1->value);
						free($1);
						free($2);
					}

					| LBRACE StatementSemicolonRep RBRACE
					{
						if ($2 != NULL) {
							if ($2->brother != NULL) {
								temp_ast_node = create_node("Block", NULL);
								insert_son(temp_ast_node, $2);
								$$ = temp_ast_node;
							}
							else {
								$$ = $2;
							}
						}
						else {
							$$ = $2;
						}
						free($1);
						free($3);
					}

					| IF Expr LBRACE StatementSemicolonRep RBRACE ElseStmntSemiRepOpt
					{
						$$ = create_node_with_position_from_node("If", $2);
						insert_son($$, $2);
						temp_ast_node = create_node("Block", NULL);
						insert_son(temp_ast_node, $4);
						insert_sibling($$->son, temp_ast_node);
						if($6 != NULL) {
							temp_ast_node = create_node("Block", NULL);
							insert_son(temp_ast_node, $6);
							insert_sibling($$->son, temp_ast_node);
						}
						else {
							temp_ast_node = create_node("Block", NULL);
							insert_sibling($$->son, temp_ast_node);
						}
						free($1);
						free($3);
						free($5);
					}

					| FOR ExprOpt LBRACE StatementSemicolonRep RBRACE
					{						
						if ($2 != NULL) {
							$$ = create_node_with_position_from_node("For", $2);
							insert_son($$, $2);
							temp_ast_node = create_node("Block", NULL);
							insert_sibling($$->son, temp_ast_node);

							if($4 != NULL){
								insert_son(temp_ast_node, $4);	
							}	
						}
						else {
							$$ = create_node("For", NULL);
							temp_ast_node = create_node("Block", NULL);
							insert_son($$, temp_ast_node);
							if($4 != NULL){
								insert_son(temp_ast_node, $4);	
							}
						}
						free($1);
						free($3);
						free($5);
					}
					
					| RETURN ExprOpt
					{
						if($2 != NULL) {
							$$ = create_node_with_position_from_node("Return", $2);
							insert_son($$, $2);
						}
						else {
							$$ = create_node("Return", $1);
						}
						free($1);
					}
					
					| FuncInvocation
					{$$ = $1;}

					| ParseArgs
					{$$ = $1;}
					
					| PRINT LPAR ExprOrStrlit RPAR
					{
						$$ = create_node("Print", $1);
						insert_son($$, $3);
						free($1);
						free($2);
						free($4);
					}
					
					| error
					{
						$$ = NULL;
						syntactic_error++;
					}
					;

ParseArgs:			ID COMMA BLANKID ASSIGN PARSEINT LPAR CMDARGS LSQ Expr RSQ RPAR
					{
						$$ = temp_ast_node = create_node_with_type("ParseArgs", "Int", $5);
						insert_son(temp_ast_node, create_node_with_type("Id", "Todo_var_stmnt", $1));
						insert_sibling(temp_ast_node->son, $9);
						free($1->value);
						free($1);
						free($2);
						free($3);
						free($4);
						free($5);
						free($6);
						free($7);
						free($8);
						free($10);
						free($11);
					}

					| ID COMMA BLANKID ASSIGN PARSEINT LPAR error RPAR
					{
						$$ = NULL;
						syntactic_error++;
						free($1->value);
						free($1);
						free($2);
						free($3);
						free($4);
						free($5);
						free($6);
						free($8);
					}
					;

FuncInvocation:		ID LPAR ExprCommaExprRepOpt RPAR
					{
						$$ = create_node("Call", NULL);
						insert_son($$, create_node_with_type("Id", "Todo_func", $1));
						if ($3 != NULL) {
							insert_sibling($$->son, $3);
						}
						free($1->value);
						free($1);
						free($2);
						free($4);
					}

					| ID LPAR error RPAR
					{
						$$ = NULL;
						syntactic_error++;
						free($1->value);
						free($1);
						free($2);
						free($4);
					}
					;

ExprCommaExprRepOpt:Empty
					{$$ = NULL;}
					
					| Expr CommaExprRep
					{
						$$ = $1;

						if($2 != NULL) {
							insert_sibling($1, $2);
						}
					}
					;

CommaExprRep:		Empty
					{$$ = NULL;}
					
					| COMMA Expr CommaExprRep	
					{
						if($3 != NULL) {
							$$ = $2;
							insert_sibling($2, $3);
						} 
						else {
							$$ = $2;
						}
						free($1);
					}
					;		

ExprOrStrlit:		Expr
					{$$ = $1;}
					
					| STRLIT															
					{
						$$ = create_node("StrLit", $1);
						free($1->value);
						free($1);
					}
					;

ExprOpt:			Empty
					{$$ = NULL;}
					
					| Expr																
					{$$ = $1;}
					;

ElseStmntSemiRepOpt:Empty																
					{$$ = NULL;}

					| ELSE LBRACE StatementSemicolonRep RBRACE
					{
						$$ = $3;
						free($1);
						free($2);
						free($4);
					}
					;

StatementSemicolonRep:
					Empty																
					{$$ = NULL;}

					| Statement SEMICOLON StatementSemicolonRep
					{
						if ($3 != NULL) {
							if ($1 != NULL) {
								$$ = $1;
								insert_sibling($1, $3);
							}
							else {
								$$ = $3;
							}
						}
						else {
							$$ = $1;
						}
						free($2);
					}
					;

Expr:				Expr OR Expr
					{
						$$ = create_node_with_type("Or", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}
					
					| Expr AND Expr														
					{
						$$ = create_node_with_type("And", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr LT Expr														
					{
						$$ = create_node_with_type("Lt", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr EQ Expr														
					{
						$$ = create_node_with_type("Eq", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr NE Expr														
					{
						$$ = create_node_with_type("Ne", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr LE Expr														
					{
						$$ = create_node_with_type("Le", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr GE Expr														
					{
						$$ = create_node_with_type("Ge", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr GT Expr														
					{
						$$ = create_node_with_type("Gt", "Bool", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr PLUS Expr													
					{
						$$ = create_node("Add", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr MINUS Expr													
					{
						$$ = create_node("Sub", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr STAR Expr													
					{
						$$ = create_node("Mul", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr DIV Expr														
					{
						$$ = create_node("Div", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| Expr MOD Expr														
					{
						$$ = create_node("Mod", $2); insert_son($$,$1); insert_sibling($1,$3);
						free($2);
					}

					| NOT Expr %prec UNARY												
					{
						$$ = create_node_with_type("Not", "Bool", $1); insert_son($$,$2);
						free($1);
					}

					| MINUS Expr %prec UNARY											
					{
						$$ = create_node("Minus", $1); insert_son($$,$2);
						free($1);
					}

					| PLUS Expr %prec UNARY				        						
					{
						$$ = create_node("Plus", $1); insert_son($$,$2);
						free($1);
					}

					| INTLIT															
					{
						$$ = create_node_with_type("IntLit", "Int", $1);
						free($1->value);
						free($1);
					}

					| REALLIT															
					{
						$$ = create_node_with_type("RealLit", "Float32", $1);
						free($1->value);
						free($1);
					}

					| ID																
					{
						$$ = create_node_with_type("Id", "Todo_var_expr", $1);
						free($1->value);
						free($1);
					}

					| FuncInvocation													
					{$$ = $1;}

					| LPAR Expr RPAR
					{
						$$ = $2;
						free($1);
						free($3);
					}

					| LPAR error RPAR													
					{
						$$ = NULL;
						syntactic_error++;
						free($1);
						free($3);
					}
					;

Empty: 				/*EMPTY*/
					{$$ = NULL;}
					;

%%
