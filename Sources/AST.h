/**********************************************
 *
 *   Panoramix Compiler 
 *   - Abstract Syntax Tree header
 *    
 *   @version 1.1.10
 *   @edited December 04, 2019
 *   @author Júlia Wotzasek Pereira
 *	 @author Vinícius juliasse Dantas
 *
**********************************************/

#ifndef _AST_H_
#define _AST_H_

/*
 * Declarations for a C- compiler. Here we
 * define the requiered structures and necessary
 * functions to create the Abstract Syntax Tree
 * and the Symbol Table of the Program
*/

/* interface with the panoramix lexer */
extern int yylineno;

/* @function: yyerror 
 * 
 * procedure that communicates an error with lexer
 * 
 * @param {char*} the symbol name
 * @param {...} multiple parameters
 *
 * @returns void
*/

void yyerror(char *s, ...);

/***********************************************/
/*      Function prototypes for the symtab     */
/***********************************************/

/* Symbol table */

/* struct symbol: This structure receives the name, the
 * value, which function it belongs and its 
 * arguments. The symlist can be a NULL pointer, if
 * it doesn't have arguments.
*/

struct symbol { 
	char *name;
	int value;
	int type; /* int or void */
	int isArray; /* not an array by default */
	int length;
	struct symbol *father; /* it is not null if it is an indexed symbol */
	struct ast *func; /* statements for the function */
	struct symlist *syms; /* list of arguments */
	struct ast *calls; /* calls if it is a function */
	int lineno;
};

/* struct symlist: This structure is a linked-list 
 * of symbols, for an argument list.
*/

struct symlist {
	struct symbol *sym;
	struct symlist *next;
};

/* Definition of a simple symtab of fixed size */
#define NHASH 9997
struct symlist* symtab[NHASH];


/* Function and procedure prototypes for the symtab */

/* @function: createsymbol
 * 
 * create a symbol with the name passed as a parameter
 * 
 * @param {char*} the identifier name
 *
 * @returns {symbol*}: pointer to a symbol
*/

struct symbol* createsymbol(char* symname);

/* @function: newsymlist 
 * 
 * creates a list of symbols.
 * 
 * @param {struct symbol*} sym : symbol added
 * @param {struct symlist*} pointer to a symlist.
 *
 * @returns {struct symlist*}
*/

struct symlist *newsymlist(struct symbol* sym, struct symlist *next);

/* @function: symlistfree 
 * 
 * Free the alocated memory of the symlist passed.
 * 
 * @param {struct symlist*} pointer to a symlist.
 *
 * @returns: void
*/

void symlistfree(struct symlist *s1);

/* @function: createSymTab
 * 
 * creates a symbol table for the tree passed as parameter.
 * 
 * @param {struct ast*} ast: which tree.
 *
 * @returns {void} 
*/

void createSymTab(struct ast* ast);

/* @function: printSymTab
 * 
 * print the symbol table.
 * 
 * @returns {void} 
*/

void printSymTab();

/***********************************************/
/*             Abstract Syntax Tree            */
/***********************************************/

/* Node types: We have defined a lot of node types
 * for the AST. These are the possible types:
 *
 * Simple operations: + - = * /
 * Comparisons: 1-7 possibilities > >= < <= == !=
 * L - expression or statement list
 * F - user function call
 * I - if-else statement
 * W - while statement
 * K - constant value
 * N - symbol reference
 * R - return
 * = - assignment node.
*/

/* Basic node for an AST */
struct ast {
	int nodetype;
	struct ast *left;
	struct ast *right;
};

struct fndef {
	int nodetype; /* type F */
	struct ast* params;
	struct ast* body;
	struct symbol *sym;
	int ret; /*return */
};

/* Node for calls to created function */
struct fncall {
	int nodetype; /* type C */
	struct ast *args; /* list of arguments */
	struct symbol *sym;
};

/* Node for flow statements, as IF-ELSE or WHILE */
struct flow {
	int nodetype; /* type I or W */
	struct ast *condition;
	struct ast *then_list; /* then branch */
	struct ast *else_list; /*optional else branch */
};

/* Node for integer numbers */
struct numval {
	int nodetype; /* type K */
	int number;
};

/* Node for  symbol references */
struct symref {
	int nodetype; /* type N */
	struct symbol *sym;
	struct ast* index;
};

/* Node for assignments */
struct symasgn {
	int nodetype; /* type = */
	struct symref *symref;
	struct ast *value;
};

/* Function prototypes to build an AST */

/* @function: newast 
 * 
 * creates a generic abstract syntax tree pointer.
 * 
 * @param {int} nodetype : which type of node
 * @param {struct ast*} l: left child
 * @param {struct ast*} r: right child
 *
 * @returns {struct ast*} new AST pointer.
*/

struct ast* newast(int nodetype, struct ast* l, struct ast* r);

/* @function: newcmp
 * 
 * creates an abstract syntax tree pointer for a
 * comparison statement.
 * 
 * @param {int} cmptype: which type of comparison
 * @param {struct ast*} l: left child
 * @param {struct ast*} r: right child
 *
 * @returns {struct ast*}
*/

struct ast* newcmp(int cmptype, struct ast* l, struct ast* r);

/* @function: newcall
 * 
 * makes a call to an user created function.
 * 
 * @param {struct symbol*} s : which function
 * @param {struct ast*} l: arguments of the function
 *
 * @returns {struct ast*} node of type fncall
*/

struct ast* newcall(struct symbol *s, struct ast *l);


/* @function: newref
 * 
 * creates a new reference for an identifier
 * 
 * @param {struct symbol*} s : symbol name
 * @param {int} type : type of the variable
 *
 * @returns {struct ast*} pointer to the symbol
*/

struct ast* newref(struct symbol *s, struct ast* type, struct ast *index);

/* @function: newasgn
 * 
 * makes the assignment of a value into a variable
 * 
 * @param {struct symbol*} s : symbol name
 * @param {struct ast*} v: AST of the calculated
 * value;
 *
 * @returns {struct ast*} pointer to the variable
*/

struct ast* newasgn(struct ast *s, struct ast *v);

/* @function: newnum
 * 
 * creates a node typed as 'K' to receive a number.
 * 
 * @param {int} i : integer value
 * 
 * @returns {struct ast*} pointer to the node.
*/

struct ast* newnum(int i);

/* @function: newflow
 * 
 * creates an abstract syntax tree pointer for a
 * flow like IF-ELSE or WHILE.
 * 
 * @param {int} nodetype: which type of flow
 * @param {struct ast*} cond: expression to be
 * tested.
 * @param {struct ast*} tl: then-list pointer
 * @param {struct ast*} el: else-list pointer
 *
 * @returns {struct ast*}
*/

struct ast* newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el);

/* @function: treefree
 * 
 * free the memory allocated for a tree with nodes
 * of type struct ast
 * 
 * @param {struct ast*} ast: which tree
 *
 * @returns {void}
*/

void treefree(struct ast *ast);

/* @function: printtree
 * 
 * print the syntax tree. It calls @printnode
 * to print the node with the necessary information.
 * 
 * @param {struct ast*} ast: which tree should be printed.
 *
 * @returns {void}
*/
void printtree_old(struct ast *ast);
int levels(struct ast* ast);
void printtree(struct ast *ast, int space);

/* @function: newfn
 * 
 * creates a fndef node to a new function created.
 * 
 * @param {struct symbol*} ret : if it is void or int
 * @param {struct symbol*} name : holds the ID
 * @param {struct ast*} syms: parameters of the function
 * @param {struct ast*} func: statements that should be
 * executed.
 *
 * @returns {struct ast*} node of type fndef
*/

struct ast* newfn(struct ast* ret, struct symbol *name, struct ast *syms, struct ast* func);

/* @function: newtype
 * 
 * creates a node with the type (void or int) on it.
 * 
 * @param {int} type: which type.
 *
 * @returns {struct ast*} 
*/

struct ast* newtype(int type);

/***********************************************/
/* Functions to generate the intermediate code */
/***********************************************/

/* @function: generateInterCode
 * 
 * generates the intermediate Code as a stack.
 * 
 * @param {struct ast*} ast: which tree.
 *
 * @returns {void} 
 *
 * @todo: include if-else routines
 * @todo: include while routines
 * @todo: define how to include labels
 * @todo: fix tree problems 
 *
*/
void generateInterCode(struct ast *a);


#endif