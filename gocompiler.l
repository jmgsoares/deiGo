/*
    João Soares - 2009113061
    José Ferreira - 2014192844
*/

numbers							[0-9]
octal							0[0-7]+
decimal							0|([1-9]{numbers}*)
hex_letters						[a-fA-F]
hexadecimal						0(x|X)({numbers}|{hex_letters})+

letters_uppercase				[A-Z_]
letters_lowercase				[a-z_]
letter							{letters_lowercase}|{letters_uppercase}
char							[^\n\r"\\]|\\[fnrt\\\"]

invalid_escape_sequences_wonl	\\[^fnrt\\\"\n\r]

invalid_escape_sequences_wnl	\\\n|\\\r\n

exp								[eE][+-]?{numbers}+
reserved						break|case|chan|const|continue|default|defer|fallthrough|go|goto|import|interface|map|range|select|struct|switch|node_type|"++"|"--"

%x COMMENTML
%x COMMENTL
%x STRING_LIT
%x STRING_LIT_ERR
%x SEMICOLON_CHECK
%x COMMENTML_NO_SEMICOLON

%{
	#include <stdbool.h>
	#include <string.h>
	#include "y.tab.h"
	#include "gocompiler_structures.h"
	#include "gocompiler_semantics.h"
	#include "gocompiler_ast.h"
	#include "gocompiler_symtab.h"
	#include "gocompiler_code_gen.h"

	extern ast_node* root;
	extern symbol_table_list_head *symbol_table;

	int line=1, col=1, errorCol, errorLine;
	int show_tokens = 0, show_ast = 0, send_token_yacc = 0, parse_token_flag = 0;
	int show_symtab_and_aast = 0, semantic_error = 0, syntactic_error = 0;

	positioned_token* position_token(position p) {
		positioned_token *token;
		token = (positioned_token*) malloc (sizeof(positioned_token));
		token->value = NULL;
		token->p = p;
		return token;
	}
%}

%%

";" {
	parse_token_flag = 0;
	if(show_tokens) printf("SEMICOLON\n"); 
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; 
	if(send_token_yacc) return SEMICOLON;
}

"_"	{
	parse_token_flag = 0;
	if(show_tokens) printf("BLANKID\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return BLANKID;
}

"package" {
	parse_token_flag = 0;
	if(show_tokens) printf("PACKAGE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; 
	if(send_token_yacc) return PACKAGE;
}

"return" {
	parse_token_flag = 0;
	if(show_tokens) printf("RETURN\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; 
	BEGIN SEMICOLON_CHECK; 
	if(send_token_yacc) return RETURN;
}

"&&" {
	parse_token_flag = 0;
	if(show_tokens) printf("AND\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; 
	if(send_token_yacc) return AND;
}

"=" {
	parse_token_flag = 0;
	if(show_tokens) printf("ASSIGN\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return ASSIGN;
}

"*" {
	parse_token_flag = 0;
	if(show_tokens) printf("STAR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return STAR;
}

","	{
	parse_token_flag = 0;
	if(show_tokens) printf("COMMA\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return COMMA;
}

"/" {
	parse_token_flag = 0;
	if(show_tokens) printf("DIV\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return DIV;
}

"==" {
	parse_token_flag = 0;
	if(show_tokens) printf("EQ\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return EQ;
}

">=" {
	parse_token_flag = 0;
	if(show_tokens) printf("GE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return GE;
}

">" {
	parse_token_flag = 0;
	if(show_tokens) printf("GT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return GT;
}

"{" {
	parse_token_flag = 0;
	if(show_tokens) printf("LBRACE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return LBRACE;
}

"<=" {
	parse_token_flag = 0;
	if(show_tokens) printf("LE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return LE;
}

"(" {
	parse_token_flag = 0;
	if(show_tokens) printf("LPAR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return LPAR;
}

"[" {
	parse_token_flag = 0;
	if(show_tokens) printf("LSQ\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return LSQ;
}

"<" {
	parse_token_flag = 0;
	if(show_tokens) printf("LT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return LT;
}

"-" {
	parse_token_flag = 0;
	if(show_tokens) printf("MINUS\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return MINUS;
}

"%" {
	parse_token_flag = 0;
	if(show_tokens) printf("MOD\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return MOD;
}

"!=" {
	parse_token_flag = 0;
	if(show_tokens) printf("NE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return NE;
}

"!" {
	parse_token_flag = 0;
	if(show_tokens) printf("NOT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return NOT;
}

"||" {
	parse_token_flag = 0;
	if(show_tokens) printf("OR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return OR;
}

"+" {
	parse_token_flag = 0;
	if(show_tokens) printf("PLUS\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return PLUS;
}

"}" {
	parse_token_flag = 0;
	if(show_tokens) printf("RBRACE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; 
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) return RBRACE;
}

")" {
	parse_token_flag = 0;
	if(show_tokens) printf("RPAR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) return RPAR;
}

"]" {
	parse_token_flag = 0;
	if(show_tokens) printf("RSQ\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK; 
	if(send_token_yacc) return RSQ;
}

"else" {
	parse_token_flag = 0;
	if(show_tokens) printf("ELSE\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return ELSE;
}

"for" {
	parse_token_flag = 0;
	if(show_tokens) printf("FOR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return FOR;
}

"if" {
	parse_token_flag = 0;
	if(show_tokens) printf("IF\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return IF;
}

"var" {
	parse_token_flag = 0;
	if(show_tokens) printf("VAR\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; if(send_token_yacc) return VAR;
}

"int" {
	parse_token_flag = 0;
	if(show_tokens) printf("INT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return INT;
}

"float32" {
	parse_token_flag = 0;
	if(show_tokens) printf("FLOAT32\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return FLOAT32;
}

"bool" {
	parse_token_flag = 0;
	if(show_tokens) printf("BOOL\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return BOOL;
}

"string" {
	parse_token_flag = 0;
	if(show_tokens) printf("STRING\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng; if(send_token_yacc) return STRING;
}

"fmt.Println" {
	parse_token_flag = 0;
	if(show_tokens) printf("PRINT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return PRINT;
}

"strconv.Atoi" {
	parse_token_flag = 0;
	if(show_tokens) printf("PARSEINT\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return PARSEINT;
}

"func" {
	parse_token_flag = 0;
	if(show_tokens) printf("FUNC\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return FUNC;
}

"os.Args" {
	parse_token_flag = 0;
	if(show_tokens) printf("CMDARGS\n");
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return CMDARGS;
}

{reserved} {
	parse_token_flag = 0;
	if(show_tokens) printf("RESERVED(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	if(send_token_yacc) return RESERVED;
}

{numbers}+ {
	parse_token_flag = 0;
	if(show_tokens) printf("INTLIT(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) {
		yylval.charVal->value = strdup(yytext);
		return INTLIT;
	}
}

{octal}|{decimal}|{hexadecimal} {
	parse_token_flag = 0; 
	if(show_tokens) printf("INTLIT(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) {
		yylval.charVal->value = strdup(yytext);
		return INTLIT;
	}
}

{letter}({letter}|{decimal})* {
	parse_token_flag = 0;
	if(show_tokens) printf("ID(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) {
		yylval.charVal->value = strdup(yytext);
		return ID;
	}
}

({decimal}+(([\.]({decimal}+{exp}?)?)|([\.]?{exp})))|[\.](({decimal}+{exp}?)) {
	if(show_tokens) printf("REALLIT(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) {
		yylval.charVal->value = strdup(yytext);
		return REALLIT;
	}
}

<SEMICOLON_CHECK>(" "*|\t*) {
	parse_token_flag = 0;
	col+=yyleng;
}

<SEMICOLON_CHECK>("\n"|"\r\n") {
	parse_token_flag = 0;
	if(show_tokens) printf("SEMICOLON\n"); 
	errorCol = col; 
	errorLine = line; 
	parse_token_flag = 1; 
	yylval.charVal = position_token((position){line, col}); 
	line++;
	col=1;
	BEGIN 0;
	if(send_token_yacc)	return SEMICOLON;
}

<SEMICOLON_CHECK>"/*" {
	parse_token_flag = 0;
	errorCol = col;
	errorLine = line;
	col+=yyleng;
	BEGIN COMMENTML;
}

<SEMICOLON_CHECK>"//" {
	parse_token_flag = 0;
	if(show_tokens) printf("SEMICOLON\n");
	errorCol = col;
	errorLine = line;
	parse_token_flag = 1;
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN COMMENTL;
	if(send_token_yacc) return SEMICOLON;
}

<SEMICOLON_CHECK><<EOF>> {
	parse_token_flag = 0;
	if(show_tokens) printf("SEMICOLON\n");
	errorCol = col;
	errorLine = line;
	parse_token_flag = 1;
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN 0;
	if(send_token_yacc) return SEMICOLON;
}

<SEMICOLON_CHECK>""/. {
	parse_token_flag = 0;
	BEGIN 0;
}

\"{char}*(\") {
	parse_token_flag = 0;
	if(show_tokens) printf("STRLIT(%s)\n",yytext);
	yylval.charVal = position_token((position){line, col}); 
	col+=yyleng;
	BEGIN SEMICOLON_CHECK;
	if(send_token_yacc) {
		yylval.charVal->value = strdup(yytext);
		return STRLIT;
	}
}

\" {
	parse_token_flag = 0;
	BEGIN STRING_LIT;
	errorCol=col;
	col+=yyleng;
}

<STRING_LIT>{invalid_escape_sequences_wonl} {
	parse_token_flag = 0;
	printf("Line %d, column %d: invalid escape sequence (%s)\n", line, col, yytext);
	col+=yyleng;
	BEGIN STRING_LIT_ERR;
}

<STRING_LIT>{invalid_escape_sequences_wnl} {
	parse_token_flag = 0;
	printf("Line %d, column %d: invalid escape sequence (\\)\n", line, col);
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}

<STRING_LIT>("\n"|"\r\n") {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}

<STRING_LIT>{char} {
	parse_token_flag = 0;
 	col+=yyleng;
}

<STRING_LIT><<EOF>> {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}

<STRING_LIT_ERR>{char} {
	parse_token_flag = 0;
 	col+=yyleng;
}

<STRING_LIT_ERR>(\") {
	parse_token_flag = 0;
	col+=yyleng;
	BEGIN 0;
}

<STRING_LIT_ERR>("\n"|"\r\n") {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}	

<STRING_LIT_ERR>. {
	parse_token_flag = 0;
 	col+=yyleng;
}

<STRING_LIT_ERR>{invalid_escape_sequences_wonl} {
	parse_token_flag = 0;
	printf("Line %d, column %d: invalid escape sequence (%s)\n", line, col, yytext);
	col+=yyleng;
}

<STRING_LIT_ERR>{invalid_escape_sequences_wnl} {
	parse_token_flag = 0;
	printf("Line %d, column %d: invalid escape sequence (\\)\n", line, col);
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}

<STRING_LIT_ERR><<EOF>> {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated string literal\n", line, errorCol);
	line++;
	col=1;
	BEGIN 0;
}	

"//" {
	parse_token_flag = 0;
	BEGIN COMMENTL;
	errorLine=line;
	errorCol=col;
	col+=yyleng;
}

<COMMENTL>. {
	parse_token_flag = 0;
 	col+=yyleng;
}

<COMMENTL>("\n"|"\r\n") {
	parse_token_flag = 0;
	line++;
	col=1;
	BEGIN 0;
}

<COMMENTL><<EOF>> {
	parse_token_flag = 0;
	col+=yyleng;
	BEGIN 0;
}

"/*" {
	parse_token_flag = 0;
	BEGIN COMMENTML_NO_SEMICOLON;
	errorLine=line;
	errorCol=col;
	col+=yyleng;
}

<COMMENTML_NO_SEMICOLON>. {
	parse_token_flag = 0;
 	col+=yyleng;
}

<COMMENTML_NO_SEMICOLON>"\n" {
	parse_token_flag = 0;
	line++;
	col=1;
}

<COMMENTML_NO_SEMICOLON>"*/" {
	parse_token_flag = 0;
	col+=yyleng;
	BEGIN 0;
}

<COMMENTML_NO_SEMICOLON><<EOF>> {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated comment\n",errorLine,errorCol);
	col+=yyleng;
	BEGIN 0;
}

<COMMENTML>. {
	parse_token_flag = 0;
 	col+=yyleng;
}

<COMMENTML>("\n"|"\r\n") {
	parse_token_flag = 0;
	line++;
	col=1;
}

<COMMENTML>"*/" {
	parse_token_flag = 0;
	col+=yyleng;
	BEGIN 0;
}

<COMMENTML>("*/\n"|"*/\r\n") {
	parse_token_flag = 0;
	if(show_tokens) printf("SEMICOLON\n");
	errorCol = col;
	errorLine = line;
	parse_token_flag = 1;
	yylval.charVal = position_token((position){line, col}); 
	line++;
	col=1;
	BEGIN 0;
	if(send_token_yacc) return SEMICOLON;
}

<COMMENTML><<EOF>> {
	parse_token_flag = 0;
	printf("Line %d, column %d: unterminated comment\n",errorLine,errorCol);
	col+=yyleng;
	BEGIN 0;
}

" "|"\t" {
	col+=yyleng;
}

"\n"|"\r\n" {
	errorCol=col;
	errorLine=line;
	line++;
	col=1;
}

<<EOF>> {
	parse_token_flag = 1;
 	return 0;
}

. {
	parse_token_flag = 0;
	printf("Line %d, column %d: illegal character (%s)\n", line, col, yytext);
	col+=yyleng;
}

%%

int main(int argc, char *argv[]){
	root = NULL;
	symbol_table = NULL;
	#if YYDEBUG == 1
	yydebug = 1;
	#endif

	if(argc >1){
		if(strcmp(argv[1], "-l") == 0 ) {
			show_tokens = 1;
			yylex();
		}
		else if(strcmp(argv[1], "-t") == 0 ) {
			show_ast = 1;
			send_token_yacc = 1;
			yyparse();
			if(show_ast) {
				print_ast(root,0);
			}
		}
		else if(strcmp(argv[1], "-s") == 0 ) {
			show_ast = 1;
			send_token_yacc = 1;
			show_symtab_and_aast = 1;
			yyparse();
			if (show_ast) {
				semantic_analysis(root);
				if(semantic_error > 0) print_semantic_errors(root);
				print_symbol_table();
				printf("\n");
				print_annotated_ast(root,0);
				show_ast = 0;
			}
		}
		else {
        		send_token_yacc = 1;
        		yyparse();
        		if (syntactic_error == 0) {
        			semantic_analysis(root);
        			show_ast = 0;
        			if(semantic_error > 0) print_semantic_errors(root);
        			else generate_llvmir(root, symbol_table);

        		}
        	}
	}
	else {
		send_token_yacc = 1;
		yyparse();
		if (syntactic_error == 0) {
			semantic_analysis(root);
			show_ast = 0;
			if(semantic_error > 0) print_semantic_errors(root);
			else generate_llvmir(root, symbol_table);

		}
	}	
	if(symbol_table != NULL) free_symbol_table(symbol_table);
	if(root != NULL) free_ast(root);
	return 0;
}

int yywrap() {
	return 1;
}

void yyerror(char *s) {
	show_ast = 0;
	show_symtab_and_aast = 0;
	semantic_error++;
	if (parse_token_flag) printf("Line %d, column %d: %s: %s\n", errorLine, errorCol, s, "");
	else printf("Line %d, column %d: %s: %s\n", line, (int)(col-yyleng), s, yytext);	
}

void yyerror_line_col(char *s, int line, int col) {
	show_ast = 0;
	show_symtab_and_aast = 0;
	semantic_error++;
	printf("Line %d, column %d: %s: \n", line, col, s);
}

