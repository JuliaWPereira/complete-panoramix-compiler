/**********************************************
 *
 *   Panoramix Compiler - Parser
 *   
 *   @version 1.1.10
 *   @edited December 04, 2019
 *   @author Júlia Wotzasek Pereira
 *
**********************************************/

%{
	#include <stdio.h>
	#include <stdlib.h>
	#include <fcntl.h>
	#include <unistd.h>
	#include "AST.h" /* Abstract Syntax Tree
	 Header */
	int yylex(); /* integration with lex */	
	static struct ast *tree;
%}

%debug

%union {
	struct ast *ast;
	int i; /* which number */
	int fn; /* which function */
	struct symbol *s;
	struct symlist *sl;
}

%token <i>  NUM INT VOID
%token <s>  ID
%token EOL 
%token ELSE IF RETURN WHILE

%nonassoc <fn> CMP
%right '='
%left '+' '-'
%left '*' '/'

%type <ast> exp stmt sel_decl els_decl exp_decl it_decl ret_decl com_decl loc_decl var_decl stmt_list fun_decl decl decl_list factor activate args arg_list param_list params param var e_type


%start program

%%

program : decl_list
			{
				tree = $1; /* save tree */ 
			}
		;


decl_list: decl_list decl
		 	{
		 		$$ = newast('L',$1,$2);
		 	}
		 | decl
		 	{
		 		$$ = $1;
		 	}
		 ;

decl 	: var_decl
			{
				$$ = $1;
			}
		| fun_decl
			{
				$$ = $1;
			}
		;

e_type	: INT
			{
				$$ = newtype(259);
				$$->left = NULL;
				$$->right = NULL;
			}
		| VOID
			{
				$$ = newtype(260);
				$$->left = NULL;
				$$->right = NULL;
			}
		;

fun_decl: e_type ID '(' params ')' com_decl
			{
				$$ = newfn($1,$2,$4,$6);
			}
		;

params	: param_list
			{
				$$ = $1;
			}
		| VOID
			{
				$$ = newtype(260);
				$$->left = NULL;
				$$->right = NULL;
			}
		;

param_list: param_list ',' param
			{
				$$ = newast('L',$1,$3);
			}
		  | param
		  	{
		  		$$ = newast('L',NULL,$1);
		  	}
		  ;

param 	: e_type ID
			{	
				$$ = newref($2,$1,NULL);
			}
		| e_type ID '[' ']'
			{
				$2->length = -1;
				$$ = newref($2,$1,NULL);
			}
		;

stmt	: sel_decl
			{
				$$ = $1;
			}
		| exp_decl
			{
				$$ = $1;
			}
		| it_decl
			{
				$$ = $1;
			}
		| ret_decl
			{
				$$ = $1;
			}
		| com_decl
			{
				$$ = $1;
			}
		;

stmt_list: /* nothing */
			{
				$$ = NULL;	
			}
		| stmt_list stmt
			{	
				$$ = newast('L',$1,$2);
			}
		;

sel_decl: IF '(' exp ')' stmt els_decl
			{
				$$ = newflow('I',$3,$5,$6);
			}
		;

els_decl: ELSE stmt
			{
				$$ = $2;
			}
		| /* nothing */
		 	{
		 		$$ = NULL;
		 	}
		;

it_decl	: WHILE '(' exp ')' stmt
			{
				$$ = newflow('W',$3,$5,NULL);
			}
			;

ret_decl: RETURN ';'
			{
				$$ = newast('R',NULL,NULL);
			}
		| RETURN exp ';'
			{
				$$ = newast('R',NULL,$2);
			}
		;

com_decl: '{' loc_decl stmt_list '}'
			{
				$$ = newast('L',$2,$3);
			}
		;

loc_decl: /* nothing */
			{
				$$ = NULL;
			}
		| loc_decl var_decl
			{
				$$ = newast('L',$1,$2);
			}
		;

var_decl: e_type ID ';'
			{
				$2->lineno = yylineno;
				$$ = newref($2,$1,NULL);
			}
		| e_type ID '[' NUM ']' ';'
			{
				$2->lineno = yylineno;
				$2->length = $4;
				$$ = newref($2,$1,NULL);
			}
		;

exp_decl: exp ';'
			{
				$$ = $1;
			}
		| ';'
			{
				$$ = NULL;
			}
		;

exp		: exp CMP exp
			{
				$$ = newcmp($2,$1,$3);
			}
		| exp '+' exp
			{
				$$ = newast('+',$1,$3);
			}
		| exp '-' exp
			{
				$$ = newast('-',$1,$3);
			}
		| exp '*' exp
			{
				$$ = newast('*',$1,$3);
			}
		| exp '/' exp
			{
				$$ = newast('/',$1,$3);
			}
		| var '=' exp
			{
				$$ = newasgn($1,$3);
			}
		| factor
			{
				$$ = $1;
			}
		;

factor	: '(' exp ')'
			{
				$$ = $2;
			}
		| NUM
			{
				$$ = newnum($1);
			}
		| var
			{
				$$ = $1;
			}
		| activate
			{
				$$ = $1;
			}
		;

var 	: ID
			{
				$1->value = -1;
				$$ = newref($1,NULL,NULL);
			}
		| ID '[' exp ']'
			{
				$1->value = -1;
				$1->length = -1;
				$$ = newref($1,NULL,$3);
			}
		;

activate: ID '(' args ')'
			{
				$$ = newcall($1,$3);
			}
		;

args 	: /* nothing */
			{
				$$ = NULL;
			}
		| arg_list
			{
				$$ = $1;
			}
		;

arg_list: arg_list ',' exp
			{
				$$ = newast('L',$1,$3);
			}
		| exp
			{
				$$ = newast('L',NULL,$1);
			}
		;

%%

int main(int argc, char **argv)
{
	/* Create AST and print logs into a file */
	int out = open("analysis.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening symtab.txt"); return 255; }
	int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
    yyparse();
    createSymTab(tree);

    fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

    /* Print Symbol Table */
	out = open("symtable.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening symtable.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
	printSymTab();

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);


    /* Print AST Table */
	out = open("ast.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening ast.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
	printf("\n\n>>Abstract Syntax Tree<<\n\n");
	printtree(tree,levels(tree) / 2);

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);
	
	/* Print Intermediate Code */
	out = open("interCode.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening interCode.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
    generateInterCode(tree);
	printInterCode();	
	
	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);
	
}