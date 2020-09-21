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

struct memoryField
{
	char *label; /* label of the variable or function name */ 
	int type; /* 0 - void; 1 - int; 2 - int[] */
	int scope; /* 0 - local; 1 - global; 2 - function */
	int first_pos; /* first position allocated at the memory */
	int memory_size; /* size occupied in bytes (int = 2 bytes (at AsterixForth)) */
	int instruction_line; /* it is necessary to label fields (labels and functions) */
};

struct memoryList
{
	struct memoryField mem;
	struct memoryList *next;
};

struct assembly_instruction
{
	char *operation;
	char *parameter;
	int value;
	int line;
	int type; /* control - 0; or use - 1*/
};

typedef struct assembly_code
{
	struct assembly_instruction inst;
	struct assembly_code *next;
}assembly_code;

int next_free_memory_pos = 0; /* it is an auxiliar variable to allocate memory */
struct memoryList *begin_memList = NULL;
struct memoryList *end_memList = NULL;

int assembly_code_line = 0;
struct assembly_code *begin_assembly = NULL;
struct assembly_code *end_assembly = NULL;


FILE* assembler(char* intermediate_code_file_name);
struct memoryList* create_mem_list(char *label, int type, int scope, int first_pos, int memory_size, int instruction_line);
void generate_assembly(char* operator, char* arg_1, char* arg_2, char* arg_3);
void assembly_FUN(char* arg_1, char* arg_2);

#endif