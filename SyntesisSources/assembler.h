/**********************************************
 *
 *   Panoramix Compiler 
 *   - Abstract Syntax Tree header
 *    
 *   @version 2.0.1
 *   @edited May 09, 2020
 *   @author Júlia Wotzasek Pereira
 *
**********************************************/

#ifndef _AST_H_
#define _AST_H_

struct memory_table
{
	char *label; /* label of the variable or function name */ 
	int type; /* 0 - void; 1 - int; 2 - int[] */
	int scope; /* 0 - local; 1 - global; 2 - function */
	int first_pos; /* first position allocated at the memory */
	int memory_size; /* size occupied in bytes (int = 2 bytes (at AsterixForth)) */
};

int next_free_memory_pos = 0; /* it is an auxiliar variable to allocate memory */

/*struct quadruple
{
	char *op;
	char *param1, *param2, *param3;
};

struct qudrupleList
{
	struct quadruple quad;
	struct quadrupleList *next;
};

struct quadrupleList *quad_list_begin;
struct quadrupleList *quad_list_end;
*/

FILE* assembler(char* intermediate_code_file_name);
struct quadrupleList* create_quad_list();

#endif