/**********************************************
 *
 *   Panoramix Compiler 
 *   - Abstract Syntax Tree Functions
 *    
 *   @version 2.0.1
 *   @edited March 13, 2020
 *   @author Júlia Wotzasek Pereira
 *
**********************************************/

/* requuired packages */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h> /* to accept variable number of arguments */
#include <string.h>
#include "AST.h" /* header of this file */

struct ast *current_function;

/* Interface with the lexer */

/* @function: yyerror 
 * 
 * procedure that communicates an error with lexer
 * 
 * @param {char*} the symbol name
 * @param {...} multiple parameters
 *
 * @returns void
*/

void yyerror(char *s, ...)
{
	va_list ap;
	va_start(ap,s);
	fprintf(stderr, "%d: error: ", yylineno);
	vfprintf(stderr,s,ap);
	fprintf(stderr, "\n");
}

/* Symble Table */

/* Function and procedure for the symtab */

/* @function: symhash 
 * @static
 * @private
 * 
 * hash function to define the position of a symbol.
 * It uses the division method for hashing.
 *
 * @param {char*} sym: the symbol name.
 *
 * @returns {unsigned int}: the evaluated hash number.
*/

static unsigned symhash(char *sym)
{
	unsigned int hash = 0;
	unsigned c;

	/* basic hash function: 9 times the previous
	 * value XOR the new character.
	*/
	while(c = *sym++) hash = hash * 9 ^ c;

	return hash;
}

/* @function: createsymbol
 * 
 * create a symbol with the name passed as parameter
 * 
 * @param {char*} the identifier name
 *
 * @returns {symbol*}: pointer to a symbol
*/

struct symbol* createsymbol(char* symname)
{
	struct symbol *sym;
	sym = (struct symbol*)malloc(sizeof(struct symbol));
	if(sym)
	{
		sym->name = strdup(symname);
		sym->value = 0;
		sym->type = 0;
		sym->isArray = 0;
		sym->length = 0;
		sym->father = NULL;
		sym->func = NULL;
		sym->lineno = yylineno;
		sym->syms = NULL;
	}
	return sym;
}

/* @function: newsymlist 
 * 
 * creates a list of simbles.
 * 
 * @param {struct symbol*} sym : symbol added
 * @param {struct symlist*} pointer to a symlist.
 *
 * @returns {struct symlist*}
*/

struct symlist* newsymlist(struct symbol *sym, struct symlist *next)
{
	struct symlist *sl = malloc(sizeof(struct symlist));

 	if(!sl) /* NULL pointer */ 
	{
		yyerror("out of space");
		exit(0);
	}
	sl->sym = sym;
	sl->next = next; /* add a new node at symlist */
	return sl;
}

/* @function: symlistfree 
 * 
 * Free the alocated memory of the symlist passed.
 * 
 * @param {struct symlist*} pointer to a symlist.
 *
 * @returns: void
*/

void symlistfree(struct symlist *sl)
{
	struct symlist *next_sl;
	while(sl) { /* not a NULL pointer */
		next_sl = sl->next;
		free(sl);
		sl = next_sl;
	}
}

/* @function: searchSym 
 * @private
 *
 * Search for a symbol at the symbol table.
 * 
 * @param {struct symbol*} pointer to the symbol
 * desired.
 *
 * @returns: {struct symbol*} the pointer to the 
 * symbol if it was found. NULL pointer if was not 
 * founded.
*/

struct symbol* searchSym(struct symbol *sym)
{
	int index = symhash(sym->name);
	struct symlist *aux;
	aux = symtab[index % NHASH]->next;
	while(aux != NULL)
	{
		if(!strcmp(aux->sym->name,sym->name))
		{
			if(aux->sym->func == sym->func)
			{
				return aux->sym; /* symbol is already defined */
			}
			if(aux->sym->func == NULL){
				return aux->sym; /* global variable */
			}
			if(sym->type == 'F')
			{
				return aux->sym;
			}
			if(aux->sym->type == 'F' && sym->type != 0)
			{
				printf("%d: semantic error: Redeclaration of function %s as a variable\n",sym->lineno,sym->name);
			}
			if(sym->type == 0)
			{
				return aux->sym;
			}
		}
		aux = aux->next;
	}
	return NULL;
}

/* @function: insertSymbol 
 * @private
 *
 * Insert a symbol at the symbol table.
 * 
 * @param {struct symbol*} pointer to the symbol
 * that should be included at the table.
 *
 * @returns: {void} 
*/

void insertSymbol(struct symbol *sym)
{
	int index = symhash(sym->name);
	struct symlist *aux;
	aux = symtab[index % NHASH];
	while(aux->next != NULL)
	{
		aux = aux->next;
	}
	struct symlist *new;
	new = (struct symlist*)malloc(sizeof(struct symlist));
	new->sym = sym;
	new->next = NULL;
	aux->next = new;
}

/* @function: countArgs 
 * @private
 *
 * Verify the arguments of functions an it calls.
 * How all nodes with parameters are included at the 
 * right child of a node, we always add it from de 
 * right call. Because of it, we take the left call 
 * 10 times to shift the value and generate a number
 * with '1' and '2' where:
 * 		1 - constant or integer arguments
 *		2 - array arguments
 * 
 * @param {struct ast*} pointer to the arguments or 
 * parameters of a function.
 *
 * @returns: {int} the calculated number 
*/


int countArgs(struct ast *p)
{
	if(p == NULL)
	{
		return 0;
	}
		if(p->nodetype == 'N' || p->nodetype == 'L'|| p->nodetype == 'K'){
			int value = 0;
			if(p->nodetype == 'K')
			{
				value = 1;
			}
			else if(p->nodetype == 'N')
			{
				if(((struct symref*)p)->sym->length != 0)
				{
					value = 2;

				}
				else value = 1;
			}
			return 10 * countArgs(p->left) + countArgs(p->right) + value;		
		}
	return 0;	
}

/* @function: insertnode 
 * @private
 *
 * Insert the nodes at the symbol table. It does the
 * semantic analysis of the compiler, verifying if this
 * node should be inserted now.
 *
 * @param {struct ast*} a: pointer to the node to be inserted.
 *
 * @returns: {void}
*/


void insertnode(struct ast *a)
{
	if(a == NULL)
	{
		printf("semantic error: internal error, null symbol");
	}
	struct symbol *sym;
	struct symbol *symTabRef;
	switch(a->nodetype)
	{
		case 'N': /* variable reference */
			sym = ((struct symref*)a)->sym; /* get the symbol of ast symref node */
			sym->func = current_function; /* scope */
			if(sym->type == 260) /* void type */
			{
				printf("%d: semantic error: Invalid declaration of a void variable: %s\n",sym->lineno,sym->name);
			}
			if(sym->type == 259) /* int type declaration */
			{
				if(searchSym(sym) != NULL)  /* this symbol is  already declared */ 
				{
					printf("%d: semantic error: Redeclaration of variable: %s\n",sym->lineno,sym->name);
				}
				else
				{
					insertSymbol(sym);
				}
			}
			if(sym->type == 0) /* use of a declared variable */
			{
				if(searchSym(sym) == NULL) /* this symbol is not declared */
				{
					printf("%d: semantic error: %s is undeclared\n",sym->lineno,sym->name);
				}
				else
				{
					/* indexed variables */
					struct symbol* varRef = searchSym(sym);
					sym->father = varRef;
					if(varRef->length != 0 && sym->length == 0 && (sym->value == varRef->value) && sym->value == 0)
					{
						printf("%s/%d - %s/%d",varRef->name,varRef->value,sym->name,sym->value);
						printf("%d: semantic error: %s should have an index. It was declared as an matrix at line %d\n",sym->lineno,sym->name,varRef->lineno);
					}
					if(sym->length != 0)
					{
						if(varRef->length == 0)
						{
							printf("%d: semantic error: %s shouldn't have an index. It was declared as an integer at line %d\n",sym->lineno,sym->name,varRef->lineno);	
						}
					}
					if(sym->length == 0 && sym->value == -1)
						if(varRef->length != 0)
							sym->length = -1;
				}
			}
			break;
		
		case 'F': /* function declaration */
			sym = ((struct fndef*)a)->sym;
			current_function = a;
			sym->func = current_function;
			sym->type = 'F';
			if(searchSym(sym) != NULL) /* symbol already at the Symtab */
			{
				printf("%d: semantic error: Redeclaration of function %s\n",sym->lineno,sym->name);
			}
			sym->syms = NULL;
			
			struct symbol *main;
			main = (struct symbol*)malloc(sizeof(struct symbol));
			main->name = "main";
			main->type = 'F';
			if(searchSym(main) != NULL)
			{
				printf("%d: semantic error: main function should be the last one to be declared\n",sym->lineno);
			}
			else
			{
				if(!strcmp(sym->name,"main"))
				{
					if(((struct fndef*)a)->ret != 260) /* not void */
					{
						printf("%d: semantic error: main function should be void\n",sym->lineno);
					}
					if(countArgs(((struct fndef*)a)->params) != 0)
					{
						printf("%d: semantic error: main function should not have parameters\n",sym->lineno);
					}
				}
				insertSymbol(sym);
			}
			break;
		
		case 'C': /* function call */
			sym = ((struct fncall*)a)->sym;
			sym->calls = a;
			sym->type ='F';
			sym->father = searchSym(sym);
			if(searchSym(sym) == NULL) /* function is not declared */
			{
				if(strcmp(sym->name,"input") && strcmp(sym->name,"output"))
				{
					printf("%d: semantic error: Implicit declaration of function %s\n",sym->lineno,sym->name);
				}
			}
			else /* function is already declared */
			{
				/* we now look for the arguments */
				struct symbol *function = searchSym(sym);
				//printf("TAHTAHTA\n");
				//printf("$$$%d\n",((struct fndef*)(sym->father->func))->ret );
			
				function->syms = newsymlist(sym, function->syms);
			}
			break;
		case 'R': /* return at the end of a function */
			if(((struct fndef*)current_function)->ret == 260 && a->right != NULL)
			{
				printf("%d: semantic error: return with a value when it should be void\n",((struct fndef*)current_function)->sym->lineno);
			}
			if(((struct fndef*)current_function)->ret == 259 && a->right == NULL)
			{
				printf("%d: semantic error: int function without any return value\n",((struct fndef*)current_function)->sym->lineno);
			}
			break;
		case '=': /* assigments */
			if((a->left != NULL) && (a->left->nodetype == 'N') && (a->right != NULL) && (a->right->nodetype == 'C'))
			{
				sym = ((struct fncall*)(a->right))->sym;
				int lineno = sym->lineno;
				sym = searchSym(sym);
				if(sym != NULL && sym->func != NULL)
				{
					if(((struct fndef*)(sym->func))->ret == 260)
					{
						printf("%d: semantic error: void value was not ignored as it ought to be\n",lineno);
					}
				}
			}
	}
}

/* @function: seeParameters 
 * @private
 *
 * Verify if the parameters of a function call are appropriate. 
 * Verify their position and their type.
 *
 * @see: countArgs
 *
 * @param {struct symbol*} s: pointer to the function at the symbol
 * table.
 *
 * @returns: {void}
*/

void seeParameters(struct symbol *s)
{
	struct symlist *sl = s->syms;
	if(sl == NULL && strcmp(s->name,"main"))
	{
		printf("%d: semantic error: %s was declared but was not used\n",s->lineno,s->name);
	}else
	{
		int nd, nc;
		while(sl != NULL)
		{
			nd = countArgs(((struct fndef*)(s->func))->params);
			nc = countArgs(((struct fncall*)(sl->sym->calls))->args);
		//	printf("%d %d\n",nd,nc );
			if(nd != nc){
				int countD = 0, countC = 0, aux = nd;
				while(aux != 0)
				{
					aux = aux/10;
					countD++;
				}
				aux = nc;
				while(aux != 0)
				{
					aux = aux/10;
					countC++;
				}
				if(countD > countC){
					printf("%d: semantic error: Too few arguments to function %s\n",sl->sym->lineno,sl->sym->name);
				}
				else if(countD < countC){
					printf("%d: semantic error: Too many arguments to function %s\n",sl->sym->lineno,sl->sym->name);
				}else{
					while(countC >= 0)
					{
						if(nd%10 > nc%10)
							printf("%d: semantic error: variable at position %d should be a matrix \n",sl->sym->lineno,countC);
						if(nd%10 < nc%10)
							printf("%d: semantic error: variable at position %d should be an integer \n",sl->sym->lineno,countC);							
						countC--;
						nd = nd/10;
						nc = nc/10;
					}	
				}
			}
			sl = sl->next;
		}
	}
}

void verifyMain()
{
	struct symbol *main;
	main = (struct symbol*)malloc(sizeof(struct symbol));
	main->name = "main";
	main->type = 'F';
	
	if(searchSym(main) == NULL)
	{
		printf("Semantic error: Function 'void main()' was not declared");
	}		
}

void printSymbol(struct symbol *sym)
{	
	char *funName = ((struct fndef*)(sym->func) != NULL)? ((struct fndef*)(sym->func))->sym->name : "global";
		
	if(sym->type == 259) /* integer */
	{	
		if(sym->length > 0){
			printf("| %10s |int[] | %10s | %10d |    -    |\n",sym->name,funName,sym->lineno);
		}
		else
		{
			printf("| %10s |int   | %10s | %10d |    -    |\n",sym->name,funName,sym->lineno);
		}
	}
	if(sym->type == 'F') /* function */
	{
		int ret = ((struct fndef*)(sym->func))->ret;
		if(ret == 259)
		{
			printf("| %10s |func  | %10s | %10d |   int   |\n",sym->name,funName,sym->lineno);	
		}
		if(ret == 260)
		{
			printf("| %10s |func  | %10s | %10d |   void  |\n",sym->name,funName,sym->lineno);	
		}
	}
}

/* @function: printSymTab 
 * @private
 *
 * Print symbol table.
 *
 * @returns: {void}
*/


void printSymTab()
{
	printf("|-------------------------------------------------------|\n");
	printf("|                      Symbol Table                     |\n");
	printf("|-------------------------------------------------------|\n");
	printf("| identifier | type |      scope |  decl_line | return  |\n");
	printf("|-------------------------------------------------------|\n");
	int i;
	struct symlist *sl;
	for(i = 0;i < NHASH;i++)
	{
		if(symtab[i]->next != NULL)
		{
			sl = symtab[i]->next;
			printSymbol(sl->sym);
		}
	}
	printf("|-------------------------------------------------------|\n");
	
}

/* @function: valid 
 * @private
 *
 * Verify if the node is of a valid type.
 * It fix the problem of pointers to trash data.
 *
 * @param {int} nodetype: nodetype to be verified.
 *
 * @returns: {int} boolean verication.
*/

int valid(int nodetype)
{
	switch(nodetype)
	{
		case 'K': case 'R': case 'N': case '=':
		case '+': case '-': case '*': case '/':
		case '1': case '2': case '3': case '4':
		case '5': case '6': case 'I': case 'W':
		case 'L': case 'F': case 'C': case 259:
		case 260: case 0: return 1;
		default: return 0; 
	}
}


/* @function: createSymTabRec 
 * @private
 *
 * Create the Symbol table of the AST in order.
 * 
 * @param {struct ast*} ast: which tree.
 *
 * @returns: {void} 
*/

void createSymTabRec(struct ast *ast)
{
	if(ast != NULL && valid(ast->nodetype))
	{	
		insertnode(ast);

		if(ast->left != NULL)
		{
			createSymTabRec(ast->left);
		}
		if(ast->right != NULL)
		{
			createSymTabRec(ast->right);
		}
	}
}

/* @function: createSymTab
 * 
 * creates a symbol table for the tree passed as parameter.
 * 
 * @param {struct ast*} ast: which tree.
 *
 * @returns {void} 
*/

void createSymTab(struct ast *ast)
{
	int i;
	struct symbol *input, *output;
	input = createsymbol("input");
	input->type = 260;
	output = createsymbol("output");
	output->type = 260;
	for(i = 0; i < NHASH; i++)
	{
		symtab[i] = (struct symlist*)malloc(sizeof(struct symlist));
		symtab[i]->sym = (struct symbol*)malloc(sizeof(struct symbol));
		symtab[i]->sym->name = "head";
		symtab[i]->next = NULL;
	}
	current_function = NULL;
	insertSymbol(input);
	insertSymbol(output);
	createSymTabRec(ast);
	struct symlist *sl;
	for(i = 0;i < NHASH;i++)
	{
		if(symtab[i]->next != NULL)
		{
			sl = symtab[i]->next;
			if(sl->sym->type == 'F')
				seeParameters(sl->sym);
		}
	}
	verifyMain();
}


/* Abstract Syntax Tree */

/* Node types: We have defined a lot of node types
 * for the AST. There are the possible types:
 *
 * Simple operations: + - = * /
 * Comparisions: 1-7 possibilities > >= < <= == !=
 * L expression or statement list
 * F - user function call
 * I - if-else statement
 * W - while statement
 * K - constant value
 * N - symbol reference
 * = - assigment node.
*/

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

struct ast* newast(int nodetype, struct ast* l, struct ast* r)
{
	/* make a usual ast pointer */
	struct ast *a = malloc(sizeof(struct ast));
	if(!a) /* NULL pointer */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = nodetype;
	a->left = l;
	a->right = r;
	return a;
}

/* @function: newcmp
 * 
 * creates a abstract syntax tree pointer for a
 * comparision statement.
 * 
 * @param {int} cmptype: which type of comparision
 * @param {struct ast*} l: left child
 * @param {struct ast*} r: right child
 *
 * @returns {struct ast*}
*/

struct ast* newcmp(int cmptype, struct ast *l, struct ast *r)
{
	/* comparision uses normal ast node */
	struct ast *a = malloc(sizeof(struct ast));
	if(!a) /* NULL pointer */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = '0' + cmptype; /* which comparision */
	a->left = l; 
	a->right = r;
	return a;
}

/* @function: newcall
 * 
 * makes a call to an user created function.
 * 
 * @param {struct symbol*} s : which function
 * @param {struct ast*} l: arguments of the function
 *
 * @returns {struct ast*} node of type fncall
*/

struct ast* newcall(struct symbol *s, struct ast *l)
{
	/* alocate a pointer to fncall node type */
	struct fncall *a = malloc(sizeof(struct fncall));
	if(!a) /* NULL pointer */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = 'C'; /* note type for CALL */
	a->args = l; /* arguments */
	a->sym = s; /* which function */
	a->sym->calls = l;
	struct ast *aux;
	aux = l;
	
	/* Cast the pointer to a ast pointer. It allows
	 * to build the AST properly.
	*/
	return (struct ast*)a;
}

/* @function: newref
 * 
 * creates a new reference for an identifier
 * 
 * @param {struct symbol*} s : symbol name
 *
 * @returns {struct ast*} pointer to the symbol
*/

struct ast* newref(struct symbol *s, struct ast* type, struct ast* index)
{
	/* allocate a symref node */
	struct symref *a = malloc(sizeof(struct symref));
	if(!a) /* pointer is NULL */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = 'N'; /* node type for references */
	a->sym = s;
	a->sym->lineno = s->lineno;
	/* 0 means not a declaration */
	a->sym->type = (type == NULL)? 0 : type->nodetype;
	a->index = index;
	return (struct ast*)a; /* casting to ast */
}

/* @function: newasgn
 * 
 * makes the assigning of a value into a variable
 * 
 * @param {struct symbol*} s : symbol name
 * @param {struct ast*} v: AST of the calculated
 * value;
 *
 * @returns {struct ast*} pointer to the variable
*/

struct ast* newasgn(struct ast *s, struct ast *v)
{
	/* alocate memory for a new symasgn */
	struct symasgn *a = malloc(sizeof(struct symasgn));
	if(!a) /* NULL pointer */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = '='; /* node type for assigning */ 
	a->symref = (struct symref*)s;
	a->value = v;
	return (struct ast*)a; /* casting to ast pointer */
}

/* @function: newnum
 * 
 * creates a node typed as 'K' to receive a number.
 * 
 * @param {int} i : integer value
 * 
 * @returns {struct ast*} pointer to the node.
*/

struct ast* newnum(int i)
{
	/* creates a numval node */
	struct numval *a = malloc(sizeof(struct numval));
	if(!a) /* NULL pointer */
	{
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = 'K'; /* node type for constants */
	if(i == 0) a->number = 0;
	else a->number = i;
	return (struct ast*)a; /* casting to ast pointer */
}

/* @function: newflow
 * 
 * creates a abstract syntax tree pointer for a
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

struct ast* newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el)
{
	/* creates a flow node */
	struct flow *a = malloc(sizeof(struct flow));
	if(!a) { /* NULL pointer */
		yyerror("out of space");
		exit(0);
	}
	a->nodetype = nodetype; /* I or W */
	a->condition = cond; /* expression evaluated */
	a->then_list = tl; /* then-list of statements */
	a->else_list = el; /* else-list of statements */
	return (struct ast *)a; /* casting to ast pointer */
}

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

struct ast* newfn(struct ast* ret, struct symbol *name, struct ast *syms, struct ast *func)
{
	struct fndef *fd;
	fd = (struct fndef*)malloc(sizeof(struct fndef));			
	if(!fd) { /* NULL pointer */
		yyerror("out of space");
		exit(0);
	}
	fd->nodetype = 'F';
	fd->sym = name;
	fd->sym->lineno = name->lineno;
	fd->params = syms;
	fd->body = func;
	fd->ret = ret->nodetype;
	return (struct ast*)fd;
}

/* @function: treefree 
 * 
 * Free the alocated memory of the ast passed.
 * 
 * @param {struct ast*} ast
 *
 * @returns: void
*/

void treefree(struct ast *a)
{
	if(a)
	{
		switch(a->nodetype) {
	/* two subtrees */
		case '+':
		case '-':
		case '*':
		case '/':
		case '1': case '2': case '3': case '4': case '5': case '6':
		case 'L':
			treefree(a->right);
		/* one subtree */
		case 'F':
			treefree(a->left);
		/* no subtree */
		case 'K': case 'N': case 259: case 260: 
			break;
		case '=':
			free(((struct symasgn*)a)->value);
			break;
		/* up to three subtrees */
		case 'I': case 'W':
			free(((struct flow*)a)->condition);
			if(((struct flow *)a)->then_list) 
				treefree(((struct flow*)a)->then_list);
			if(((struct flow *)a)->else_list) 
				treefree(((struct flow*)a)->else_list);
			break;
		default: 
			printf("internal error: free bad node %c\n", a->nodetype);
	}
	free(a); /* always free the node itself */
	}
	
}

/* @function: printnode 
 * @private
 * 
 * Print the node appropriatelly.
 * 
 * @param {struct ast*} a
 *
 * @returns: void
*/

void printnode(struct ast *a)
{
	if(!a)  /* NULL pointer */
	{
		yyerror("internal error, null eval");
	}
	struct symref *sr;
	struct symbol *s;
	switch(a->nodetype)
	{
		case 'K': /* constant */ 
			printf("%d",((struct numval *)a)->number);
			break;

		case 'R':
			printf("return");
			break;
		
		case 'N': /* name reference */
			sr = (struct symref*)a;
			s = sr->sym;	
			printf("%s",s->name);
			if(sr->sym->length > 0)
			{
				printf("[%d]",sr->sym->length );
			}
			if(sr->sym->length == -1)
			{
				printf("[]");
			}
			break;
		
		case '=': /* assignment */
			printf("=");
			break;

		/* expressions */
		case '+': 
			printf("+");
			break;
		case '-': 
			printf("-"); 
			break;
		case '*': 
			printf("*"); 
			break;
		case '/': 
			printf("/"); 
			break;
		
		/* comparisons */
		case '1': 
			printf(">");
			break;
		case '2': 
			printf("<");
			break;
		case '3': 
			printf("!=");
			break;
		case '4': 
			printf("==");
			break;
		case '5': 
			printf(">=");
			break;
		case '6':
			 printf("<=");
			break;
		
		/* control flow */
		/* null expressions allowed in the grammar, so check for them */
		/* if/then/else */
		case 'I':
			printf("if");
			break;
		
		/* while/do */
		case 'W':
			printf("while");
			break; /* value of last statement is value of while/do */

		/* list of statements */
		case 'L': 
			printf("L");
			break;
		
		case 'F': 
			if(((struct fndef*)a)->sym->name == NULL) printf("MEH\n");
			else printf("%s",((struct fndef*)a)->sym->name);
			break;
		
		case 'C':
			if(((struct fncall*)a)->sym->name == NULL) printf("MEH2\n");
			else printf("%s",((struct fncall*)a)->sym->name);
			break;
		case 259:
			printf("int");
			break;
		case 260:
			printf("void");
			break;
		case 0:
			printf("int");
			break;
		default: 
			printf("internal error: bad node %c\n", a->nodetype);
			a->left = NULL;
			a->right = NULL;
	}
}

/* @function: printtree
 * 
 * print the syntax tree. It calls @printnode
 * to print the node with the necessary information.
 * In order execution.
 * 
 * @param {struct ast*} ast: which tree should be printed.
 *
 * @returns {void}
*/

void printtree_old(struct ast *ast)
{
	if(ast != NULL && valid(ast->nodetype))
	{	
		printnode(ast);
		if(ast->left != NULL)
		{
			printf("(");
			printtree_old(ast->left);
			printf(")");
		}
		if(ast->right != NULL)
		{
			printf("(");
			printtree_old(ast->right);
			printf(")");
		}
	}
}



void printtree2(struct ast *ast, int space)
{
	if(ast != NULL && valid(ast->nodetype))
	{
		for(int i = 0; i < space * 10; i++)
			printf(" ");
		printnode(ast);
		printf("\n");
		if(ast->left != NULL)
		{
			printtree2(ast->left, space - 1);
		}
		if(ast->right != NULL)
		{
			printtree2(ast->right, space + 1);
		}
	}
}

void printtree(struct ast *ast, int space)
{
	if(ast != NULL && valid(ast->nodetype))
	{
		if(ast->right != NULL && valid(ast->right->nodetype))
		{
			printtree(ast->right, space + 8);
			printf("\n");
			for(int i = 0; i < space + 4; i++)
			{
				printf(" ");
			}
			printf("/");
		}
		printf("\n");
		for(int i = 0; i < space + 3; i++)
		{
			if(i > space - 4) printf("-");
			else printf(" ");
		}
		printnode(ast);
		if(ast->left != NULL && valid(ast->left->nodetype))
		{
			printf("\n");
			for(int i = 0; i < space + 4; i++)
			{
				printf(" ");
			}
			printf("\\");
			printtree(ast->left, space + 8);
		}
	}
}



int levels(struct ast* ast)
{
	int h = 0;
	if(ast != NULL && valid(ast->nodetype))
	{
		if(ast->left != NULL && ast->right != NULL)
			return (levels(ast->left) > levels(ast->right))? levels(ast->left) + 1 : levels(ast->right) + 1;
		if(ast->left != NULL && ast->right == NULL)
			return levels(ast->left) + 1;
		if(ast->right != NULL && ast->left == NULL)
			return levels(ast->right) + 1;
	}
	return 0;
}

/* @function: newtype
 * 
 * creates a node with the type (void or int) on it.
 * 
 * @param {int} type: which type.
 *
 * @returns {struct ast*} 
*/

struct ast* newtype(int type)
{
	struct ast* a = (struct ast*)malloc(sizeof(struct ast));
	if(a) a->nodetype = type;
	return a;
}

/***********************************************/
/* Functions to generate the intermidiate code */
/***********************************************/

struct symlist *stack;
char tempNameNumber = 0;
char labelNameNumber = 0;
char paramsNumber = 0;

/* @function: itoa
 * @private 
 *
 * int to string conversion.
 * 
 * @param {int} val: which type.
 * @param {int} base: which base.
 *
 * @returns {char*} name 
*/

char* itoa(int val, int base){
	if(val == 0) return "0";
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}

struct symlist* createStack()
{
	struct symlist *stack;
	stack = (struct symlist*)malloc(sizeof(struct symlist));
	if(stack != NULL)
	{
		stack->sym = NULL;
		stack->next = NULL;
		return stack;
	}
	return NULL;
}

void push_symStack(struct symbol *sym)
{
	struct symlist *new;
	new = createStack();
	new->sym = sym;
	new->next = symStack;
	symStack = new;
}

struct symbol* pop_symStack()
{
	struct symlist *aux;
	aux = symStack;
	symStack = symStack->next;
	aux->next = NULL;
	return aux->sym;
}

void push_labelStack(struct symbol *sym)
{
	struct symlist *new;
	new = createStack();
	new->sym = sym;
	new->next = labelStack;
	labelStack = new;
}

struct symbol* pop_labelStack()
{
	struct symlist *aux;
	aux = labelStack;
	labelStack = labelStack->next;
	aux->next = NULL;
	return aux->sym;
}

void swap_labelStack()
{
	struct symlist *aux;
	aux = labelStack;
	labelStack = aux->next;
	aux->next = labelStack->next;
	labelStack->next = aux;
}

struct symlist* createArgList()
{
	struct symlist *list;
	list = (struct symlist*)malloc(sizeof(struct symlist));
	if(list != NULL)
	{
		list->sym = NULL;
		list->next = NULL;
		return list;
	}	
	return NULL;
}

void addArgList(struct symbol *sym)
{
	struct symlist *new;
	struct symlist *aux;
	new = createArgList();
	new->sym = sym;
	aux = argList;
	while(aux->next != NULL) aux = aux->next;
	aux->next = new;
}

char* tempName()
{
	char *s, *t;
	s = strdup("\0");
	s = itoa(++tempNameNumber%16,10);
	t = strdup("$t");
	return strcat(t,s);				
}

char* labelName()
{
	char *s, *L;
	s = strdup("\0");
	s = itoa(++labelNameNumber,10);
	L = strdup("L");
	return strcat(L,s);				
}

char* nParamName()
{
	char *s;
	s = strdup("\0");
	s = itoa(paramsNumber,10);
	paramsNumber = 0;
	return s;				
}


struct quadrupleList* newQuadruple(struct ast *a)
{
	struct quadrupleList *new,*aux;
	struct symbol *temp_sym, *foundSym, *label_sym;
	new = (struct quadrupleList*)malloc(sizeof(struct quadrupleList));
	if(new != NULL)
	{
		new->quad.sym1 = NULL;
		new->quad.sym2 = NULL;
		new->quad.sym3 = NULL;
		
		/* creating the head quadruple */
		if(a == NULL)
		{
			new->quad.op = -1;
			return new;
		}

		/* creating a quadruple for the node a */
		new->quad.op = a->nodetype;
		switch(a->nodetype)
		{
			case 'K': /* constant */ 
				if(((struct numval *)a)->number == -9999) temp_sym = createsymbol(itoa(0,10));
				else temp_sym = createsymbol(itoa(((struct numval *)a)->number,10));
				push_symStack(temp_sym);
				break;

			case 'R':
				new->quad.sym1 = pop_symStack();
				break;
			
			case 'N': /* name reference */
				if(((struct symref*)a)->sym->type == 259) /* variable declaration */
				{
					new->quad.sym1 = ((struct symref*)a)->sym;
					if((struct fndef*)(((struct symref*)a)->sym->func) != NULL)
						new->quad.sym2 = ((struct fndef*)(((struct symref*)a)->sym->func))->sym;			
					else new->quad.sym2 = createsymbol("GLOBAL");
					if(new->quad.sym1->length != 0)
					{
						new->quad.sym3 = createsymbol(itoa(new->quad.sym1->length,10));
					}
				}					
				else /* variable use */
				{
					temp_sym = createsymbol(tempName());
					foundSym = searchSym(((struct symref*)a)->sym);
					temp_sym->father = foundSym;
					temp_sym->syms = (struct symlist*)malloc(sizeof(struct symlist));
					new->quad.sym1 = temp_sym;
					new->quad.sym2 = foundSym;
					if(foundSym->length != 0)
					{
						new->quad.sym3 = pop_symStack();
					}
					temp_sym->syms->sym = new->quad.sym3;
					foundSym->father = new->quad.sym3;
					push_symStack(temp_sym);
				}

				break;
			
			case '=': /* assignment */
				new->quad.sym1 = searchSym((((struct symasgn*)a)->symref)->sym);
				new->quad.sym2 = pop_symStack();
				new->quad.sym1 = pop_symStack();
				if(new->quad.sym1->father->length != 0) new->quad.sym3 = new->quad.sym1->syms->sym;
				break;

			/* expressions */
			case '+': 
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '-': 
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '*': 
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '/': 
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			
			/* comparisons */
			case '1': 
				//printf(">");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '2': 
				//printf("<");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '3': 
				//printf("!=");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '4': 
				//printf("==");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '5': 
				//printf(">=");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			case '6':
				//printf("<=");
				temp_sym = createsymbol(tempName());	
				new->quad.sym1 = temp_sym;
				new->quad.sym3 = pop_symStack();
				new->quad.sym2 = pop_symStack();
				push_symStack(temp_sym);
				break;
			/* control flow */
			/* null expressions allowed in the grammar, so check for them */
			/* if/then/else */
			case 'Z':
				label_sym = createsymbol(labelName());
				new->quad.sym2 = label_sym;
				push_labelStack(label_sym);
				swap_labelStack();
				new->quad.sym1 = pop_symStack();
				break;
			case 'I':
				label_sym = createsymbol(labelName());
				new->quad.sym2 = label_sym;
				push_labelStack(label_sym);
				new->quad.sym1 = pop_symStack();
				break;
			/* while/do */
			case 'W':
				label_sym = createsymbol(labelName());
				push_labelStack(label_sym);
				new->quad.sym1 = label_sym;
				break; /* value of last statement is value of while/do */
			case 'G':
				label_sym = pop_labelStack();
				new->quad.sym1 = label_sym;
				break;
			case 'A':
				label_sym = pop_labelStack();
				new->quad.sym1 = label_sym;
				break;
			case 'E':
				label_sym = createsymbol(labelName());
				new->quad.sym1 = label_sym;
				push_labelStack(label_sym);
				swap_labelStack();
				break;
			/* list of statements */
			case 'L': 
				break;
			
			case 'F': 
				new->quad.sym1 = ((struct fndef*)a)->sym;
				new->quad.sym1->type = ((struct fndef*)a)->ret;		
				break;
			
			case 'C':
				temp_sym = createsymbol(tempName());
				temp_sym->father = ((struct fncall*)a)->sym;
				push_symStack(temp_sym);
				foundSym = searchSym(((struct fncall*)a)->sym);
				if(foundSym != NULL) foundSym->father = temp_sym;
				new->quad.sym1 = temp_sym;
				new->quad.sym2 = ((struct fncall*)a)->sym;
				new->quad.sym3 = createsymbol(nParamName());
				break;
			case 'B':
				temp_sym = pop_symStack();
				new->quad.sym1 = temp_sym;
				break;
			case 'D':
				new->quad.sym1 = ((struct symref*)a)->sym;
				new->quad.sym2 = ((struct fndef*)(((struct symref*)a)->sym->func))->sym;			
				addArgList(new->quad.sym1);
				break;
			case 'J':
				new->quad.sym1 = ((struct fndef*)a)->sym;
				break;
			case 259:
				break;
			case 260:
				break;
			case 0:
				break;
			default: 
				printf("internal error: bad node %c\n", a->nodetype);
				a->left = NULL;
				a->right = NULL;
		}


		return new;
	}
	return NULL;
}

void addQuadruple(struct ast *a)
{
	struct quadrupleList *newQuad,*aux;
	if(a->nodetype == 'L' || a->nodetype == 260 ||
	 a->nodetype == 259) return; 
	 /* nodes that should be ignored*/
	newQuad = newQuadruple(a);
	newQuad->quad.op = a->nodetype;
	aux = quadList;
	while(aux->next != NULL)
	{
		aux = aux->next;
	}
	aux->next = newQuad;
}

void insertArgList()
{
	struct quadrupleList *auxQuad; /* aux for the quadList */
	struct symlist *auxArgList; /* aux for the arglist */
	struct quadrupleList *new;
	struct symbol *found_sym, *temp_sym;
	auxQuad = quadList;
	/* go to the end of quadList */
	while(auxQuad->next != NULL)
	{
		auxQuad = auxQuad->next;
	}
	auxArgList = argList->next;
	while(auxArgList != NULL)
	{
		new = (struct quadrupleList*)malloc(sizeof(struct quadrupleList));
		if(new != NULL)
		{
			new->quad.op = 'M';
			temp_sym = createsymbol(tempName());
			found_sym = searchSym(auxArgList->sym);
			temp_sym->father = found_sym;
			found_sym->father = temp_sym;

			new->quad.sym1 = temp_sym;
			new->quad.sym2 = found_sym;
			push_symStack(temp_sym);

			auxQuad->next = new;
			auxQuad = auxQuad->next;
		}
		auxArgList = auxArgList->next;
	}
}

void freeArgList()
{
	struct symlist *aux;
	aux = argList;
	while(aux->next != NULL)
	{
		argList = argList->next;
		aux->next = NULL;
		free(aux);
		aux = argList;
	}
}

int isSymmetric(int type)
{
	switch(type)
	{
		case '=': case '+': case '-' :case '*':
		case '/': case '1': case '2': case '3':
		case '4': case '5': case '6': case 'C':
		case 'N': return 1;
		default: return 0; 
	}
}


void generateInterCodeCalls(struct ast *a)
{
	if(a->left  != NULL && valid(a->nodetype))
	{
		generateInterCodeCalls(a->left);
	}
	if(a->right != NULL && valid(a->nodetype))
	{
		generateInterCodeRecursive(a->right);
		a->nodetype = 'B';
		addQuadruple(a);
		++paramsNumber;	
	}
}

void generateInterCodeDef(struct ast *a)
{
	if(a->left != NULL)
	{
		generateInterCodeDef(a->left);
	}
	a->right->nodetype = 'D';
	addQuadruple(a->right);
}


void generateInterCodeRecursive(struct ast *a)
{
	if(a != NULL && valid(a->nodetype))
	{
		if(a->nodetype == 'W') // while
		{
			addQuadruple(a);
			if(a->left != NULL)
			{
				generateInterCodeRecursive(a->left);
			}
			a->nodetype = 'Z';
			addQuadruple(a);
			if(a->right != NULL)
			{
				generateInterCodeRecursive(a->right);
			}
			a->nodetype = 'G';
			addQuadruple(a);
			a->nodetype = 'A';
			addQuadruple(a);	
		}
		else if(a->nodetype == 'I')
		{
			if(((struct flow*)a)->condition != NULL)
			{
				generateInterCodeRecursive(((struct flow*)a)->condition);
			}
			addQuadruple(a);
			if(((struct flow*)a)->then_list != NULL)
			{
				generateInterCodeRecursive(((struct flow*)a)->then_list);
			}
			if(((struct flow*)a)->else_list != NULL)
			{
			    a->nodetype = 'E';
				addQuadruple(a);
				a->nodetype = 'A';
				addQuadruple(a);
				generateInterCodeRecursive(((struct flow*)a)->else_list);
			}
			a->nodetype = 'A';
			addQuadruple(a);
		}
		else if(!isSymmetric(a->nodetype))
		{
			addQuadruple(a);
			if(a->nodetype == 'F' && ((struct fndef*)a)->ret == 259)
			{
				if(a->left != NULL)
				{
					argList = createArgList();
					generateInterCodeDef(a->left);
					insertArgList();
					freeArgList();
				}	
				if(a->right != NULL)
				{
					generateInterCodeRecursive(a->right);
				}
				a->nodetype = 'J';
				addQuadruple(a);
			}
			else
			{
				if(a->left != NULL)
				{
					generateInterCodeRecursive(a->left);
				}
				if(a->right != NULL)
				{
					generateInterCodeRecursive(a->right);
				}
				if(a->nodetype == 'F')
				{
					a->nodetype = 'J';
					addQuadruple(a);
				}
			}	
		}
		else
		{	
			if(a->nodetype == 'C')
			{
				if(a->left != NULL)generateInterCodeCalls(a->left);
				a->nodetype = 'C';
				addQuadruple(a);	
			}
			else
			{
				if(a->left != NULL)
				{
					generateInterCodeRecursive(a->left);
				}
				if(a->right != NULL)
				{
					generateInterCodeRecursive(a->right);
				}
				addQuadruple(a);
			}
		}	
	}
}

void generateInterCode(struct ast *a)
{
	/* initialize the list */
	quadList = newQuadruple(NULL);
	symStack = createStack();
	symStack->sym = createsymbol("HEAD");
	symStack->sym->value = -1;
	labelStack = createStack();
	labelStack->sym = createsymbol("HEAD");
	labelStack->sym->value = -1;
	generateInterCodeRecursive(a);
}


void printQuadruple(struct quadruple quad)
{
	char *s,*t;
	switch(quad.op)
	{
		case -1: break;
		case 'F':
			printf("(FUN,");
			if(quad.sym1->type == 259) printf("INT,");
			else printf("VOID,");
			printf("%s,-)\n",quad.sym1->name);
			break;
		case 'N':
			/* it is a variable declaration */
			if(quad.sym1->type == 259)
				{
					printf("(ALLOC,%s,%s,",quad.sym1->name,quad.sym2->name);
					if(quad.sym3 == NULL) printf("-)\n");
					else printf("%s)\n",quad.sym3->name);
				}
			/* it is a variable use */
			else 
			{
				printf("(LOAD,%s,%s,",quad.sym1->name,quad.sym2->name);
				if(quad.sym3 != NULL) printf("%s)\n",quad.sym3->name);
				else printf("-)\n");
			}

			break;
		case 'C':
			printf("(CALL,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '=':
			printf("(ASSIGN,%s,%s,-)\n",quad.sym1->name,quad.sym2->name);
			printf("(STORE,%s,%s,", quad.sym1->name,quad.sym1->father->name);
			if(quad.sym3 != NULL) printf("%s)\n",quad.sym3->name);
			else printf("-)\n");
			break;
		case '+':
			printf("(ADD,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '-':
			printf("(SUB,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '*':
			printf("(MULT,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '/':
			printf("(DIV,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '1': // '>'
			printf("(GT,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '2': // '<' 
			printf("(LT,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '3': // '!='
			printf("(NEQ,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '4': // '==' 
			printf("(EQ,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '5': // '>='
			printf("(GEQ,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case '6': // '<='
			printf("(LEQ,%s,%s,%s)\n",quad.sym1->name,quad.sym2->name,quad.sym3->name);
			break;
		case 'K':
			break;
		case 'W':
			printf("(LABEL,%s,-,-)\n",quad.sym1->name);
			break;
		case 'G':
			printf("(GOTO,%s,-,-)\n",quad.sym1->name);
			break;
		case 'Z':
			printf("(IFF,%s,%s,-)\n",quad.sym1->name,quad.sym2->name);
			break;
		case 'A':
			printf("(LABEL,%s,-,-)\n",quad.sym1->name);
			break;
		case 'I':
			printf("(IFF,%s,%s,-)\n",quad.sym1->name,quad.sym2->name);
			break;
		case 'E':
			printf("(GOTO,%s,-,-)\n",quad.sym1->name);
			break;
		case 'B':
			printf("(PARAM,%s,-,-)\n",quad.sym1->name);
			break;
		case 'D':
			printf("(ARG,INT,%s,%s)\n",quad.sym1->name,quad.sym2->name);
			break;
		case 'M':
			printf("(LOAD,%s,%s,-)\n",quad.sym1->name,quad.sym2->name);
			break;
		case 'R':
			printf("(RET,%s,-,-)\n",quad.sym1->name);
			break;
		case 'J':
			printf("(END,%s,-,-)\n",quad.sym1->name);
			break;
		default:
			printf("%c\n", quad.op);
			break;
	}
}

void printInterCode()
{
	struct quadrupleList *aux;
	aux = quadList;
	while(aux != NULL)
	{
		printQuadruple(aux->quad);
		aux = aux->next;
	}
	printf("(HALT,-,-,-)\n");
}