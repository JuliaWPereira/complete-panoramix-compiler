/**********************************************
 *
 *   Panoramix Compiler 
 *   - Assembler Functions
 *    
 *   @version 2.0.1
 *   @edited May 09, 2020
 *   @author Júlia Wotzasek Pereira
 *
**********************************************/

/* required packages */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include "assembler.h" /* header of this file */

struct memoryList* create_mem_list(char *label, int type, int scope, int first_pos, int memory_size, int instruction_line)
{
	struct memoryList *memlist;
	int len;
	memlist = (struct memoryList*)malloc(sizeof(struct memoryList));
	if(memlist != NULL){
		memlist->next = NULL;
		len = strlen(label);
		memlist->mem.label = (char*)malloc(len * sizeof(char));
		strcpy(memlist->mem.label, label);
		memlist->mem.type = type;
		memlist->mem.scope = scope;
		memlist->mem.first_pos = first_pos;
		memlist->mem.memory_size = memory_size;
		memlist->mem.instruction_line = instruction_line;
	}
	return memlist;
}

struct assembly_code* create_assembly_code(char *operation, char *parameter, int value, int line, int type)
{
	struct assembly_code *assembly;
	assembly = (struct assembly_code*)malloc(sizeof(struct assembly_code));
	if(assembly != NULL){
		assembly->next = NULL;
		assembly->inst.operation = operation;
		assembly->inst.parameter = parameter;
		assembly->inst.value = value;
		assembly->inst.line = line;
		assembly->inst.type = type;
	}
	return assembly;
}

void add_assembly(assembly_code *assembly)
{
	if(begin_assembly == NULL){
			begin_assembly = assembly;
			end_assembly = assembly;
		}else{
			end_assembly->next = assembly;
			end_assembly = assembly;
		}
}

void add_mem(struct memoryList *new_mem)
{
	if(begin_memList == NULL){
		begin_memList = new_mem;
		end_memList = new_mem;
	}else{
		end_memList->next = new_mem;
		end_memList = new_mem;
	}
	next_free_memory_pos += new_mem->mem.memory_size ;
}

/*
 * In FUN, we should allocate a memory field to start the scope of the function 
 *
*/
void assembly_FUN(char* arg_1, char* arg_2)
{
	struct memoryList *new_mem;
	//struct assembly_code *assembly;
	int type;
	type = (!strcmp(arg_1, "INT"))? 1:0;
	new_mem = create_mem_list(arg_2, type, 2, next_free_memory_pos, 0, assembly_code_line);
	
	add_mem(new_mem);
	
	//assembly = create_assembly_code("FUN", strdup(arg_2)	, 0, assembly_code_line++, 0);
	
	//add_assembly(assembly);
}

int string_to_int(char *str)
{
	int len;
	int result = 0;
	len = strlen(str);
	for(int i = 0; i < len; i++){
		result = 10 * result + (str[i] - '0');
	}
	return result;
}

char* itoa(int val, int base){
	if(val == 0) return "0";
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
}


void assembly_ALLOC(char* arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	int type;
	int size;
	int scope;
	type = (!strcmp(arg_3, "-") | !strcmp(arg_3, "--"))? 1:2;
	size = (!strcmp(arg_3, "-") | !strcmp(arg_3, "--"))? 1:string_to_int(arg_3) * 2;
	size = (!strcmp(arg_3, "vet_pointer"))? 0:size;
	scope = (!strcmp(arg_2, "GLOBAL"))? 1:0;
	//printf("%d - %s\n", size, arg_3);
	new_mem = create_mem_list(arg_1, type, scope, next_free_memory_pos, size, -1);

	if(begin_memList == NULL){
		begin_memList = new_mem;
		end_memList = new_mem;
	}else{
		end_memList->next = new_mem;
		end_memList = new_mem;
	}
	next_free_memory_pos += 2 * size;

}

int get_address(char *label)
{
	struct memoryList *aux;
	aux = begin_memList;
	while(aux != NULL){
		if(!strcmp(label, aux->mem.label)){
			return aux->mem.first_pos;
		}
		aux = aux->next;
	}
	return -1; /* not found */
}


void assembly_LOAD(char* arg_1, char* arg_2, char* arg_3)
{
	//printf("LOAD ,%s ,%s ,%s\n", arg_1, arg_2, arg_3);
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(arg_3[0] == '$'){ /* indexed variable */
		t = strdup("&");	
		//printf(">>%s\n", arg_3);
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("+", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}
}

void assembly_STORE(char* arg_1, char* arg_2)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	t = strdup("&");	
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);		
}

void assembly_ASSIGN(char* arg_1, char* arg_2)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	t = strdup("&");	
	assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
}

int get_type(char *label)
{
	struct memoryList *aux;
	aux = begin_memList;
	while(aux != NULL){
		if(!strcmp(label, aux->mem.label)){
			return aux->mem.type;
		}
		aux = aux->next;
	}
	return -1; /* not found */
}

void assembly_CALL(char *arg_1, char* arg_2)
{
	//struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	//printf(">>%s\n", arg_2);

	if(!strcmp(arg_2,"input")){
		assembly = create_assembly_code("INPUT READ", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("INPUT LOAD", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

	}else if(!strcmp(arg_2,"output")){
		assembly = create_assembly_code("OUTPUT SEND", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

		assembly = create_assembly_code("OUTPUT PRINT", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);

	}else{
		assembly = create_assembly_code("CALL", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		if(get_type(arg_2) == 1){ /* int function */	
			t = strdup("&");
			assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
			add_assembly(assembly);

			assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
			add_assembly(assembly);
		}
	}
}

void assembly_ADD(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("+", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}

void assembly_SUB(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}

void assembly_MULT(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("*", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}


void assembly_DIV(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("/", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}

void assembly_EQ(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("XOR", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);		
}

void assembly_NEQ(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("DUP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("0"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("0"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("XOR", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("1"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);		
}

void assembly_LT(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("DUP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("0"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("0"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("--", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", strdup("1"), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);		
}

void assembly_GEQ(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", itoa(1,10), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);
	
	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}


void assembly_LEQ(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("PUSH", itoa(1,10), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("-", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);
	
	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}

void assembly_GT(char *arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_2[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	if(isdigit(arg_3[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");
		assembly = create_assembly_code("PUSH", strcat(t,arg_3), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	//assembly = create_assembly_code("SWAP", NULL, 0, assembly_code_line++, 1);
	//add_assembly(assembly);

	assembly = create_assembly_code("LEQ", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);

	t = strdup("&");
	assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);

	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
	
}

void assembly_LABEL(char* arg_1)
{
	struct memoryList *new_mem;
	new_mem = create_mem_list(arg_1, -1, 0, -1, 0, assembly_code_line);
	add_mem(new_mem);
}

void assembly_GOTO(char *arg_1)
{
	struct assembly_code *assembly;
	char *s, *t;
	
	assembly = create_assembly_code("CALL", strdup(arg_1), 0, assembly_code_line++, 1);
	add_assembly(assembly);
}

void assembly_IFF(char *arg_1, char* arg_2)
{
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_1[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

	assembly = create_assembly_code("IF", strdup(arg_2), 0, assembly_code_line++, 1);
	add_assembly(assembly);

}

void assembly_PARAM(char *arg_1)
{
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_1[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}

}

void assembly_ARG(char *arg_2)
{
	struct assembly_code *assembly;
	char *s, *t;
	
	t = strdup("&");	
	assembly = create_assembly_code("PUSH", strcat(t,arg_2), 0, assembly_code_line++, 1);
	add_assembly(assembly);
	
	assembly = create_assembly_code("!", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);
}

void assembly_RET(char *arg_1)
{
	struct assembly_code *assembly;
	char *s, *t;
	
	if(isdigit(arg_1[0])){
		assembly = create_assembly_code("PUSH", strdup(arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}else{
		t = strdup("&");	
		assembly = create_assembly_code("PUSH", strcat(t,arg_1), 0, assembly_code_line++, 1);
		add_assembly(assembly);
		
		assembly = create_assembly_code("@", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);
	}
}

void assembly_END(char *arg_1)
{
	struct assembly_code *assembly;
	
	if((strcmp(arg_1,"main"))){
		assembly = create_assembly_code("EXIT", NULL, 0, assembly_code_line++, 1);
		add_assembly(assembly);	
	}
}

void assembly_HALT()
{
	struct assembly_code *assembly;

	assembly = create_assembly_code("HALT", NULL, 0, assembly_code_line++, 1);
	add_assembly(assembly);	
}


void generate_assembly(char* operator, char* arg_1, char* arg_2, char* arg_3)
{
	if(!(strcmp(operator,"FUN"))){
//		printf("FUN\n");
		assembly_FUN(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"ALLOC"))){
//		printf("ALLOC\n");
		assembly_ALLOC(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"LOAD"))){
//		printf("LOAD\n");
		assembly_LOAD(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"STORE"))){
//		printf("STORE\n");
		assembly_STORE(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"ASSIGN"))){
//		printf("ASSIGN\n");
		assembly_ASSIGN(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"CALL"))){
//		printf("CALL\n");
		assembly_CALL(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"ADD"))){
//		printf("ADD\n");
		assembly_ADD(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"SUB"))){
//		printf("SUB\n");
		assembly_SUB(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"MULT"))){
//		printf("MULT\n");
		assembly_MULT(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"DIV"))){
//		printf("DIV\n");
		assembly_DIV(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"EQ"))){
//		printf("EQ\n");
		assembly_EQ(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"NEQ"))){
//		printf("NEQ\n");
		assembly_NEQ(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"LT"))){
//		printf("LT\n");
		assembly_LT(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"GEQ"))){
//		printf("GEQ\n");
		assembly_GEQ(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"LEQ"))){
//		printf("LEQ\n");
		assembly_LEQ(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"GT"))){
//		printf("GT\n");
		assembly_GT(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"LABEL"))){
//		printf("LABEL\n");
		assembly_LABEL(arg_1);
	}
	else if(!(strcmp(operator,"GOTO"))){
//		printf("GOTO\n");
		assembly_GOTO(arg_1);
	}
	else if(!(strcmp(operator,"IFF"))){
//		printf("IFF\n");
		assembly_IFF(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"PARAM"))){
//		printf("PARAM\n");
		assembly_PARAM(arg_1);
	}
	else if(!(strcmp(operator,"ARG"))){
//		printf("ARG\n");
		assembly_ARG(arg_2);
	}
	else if(!(strcmp(operator,"RET"))){
//		printf("RET\n");
		assembly_RET(arg_1);
	}
	else if(!(strcmp(operator,"END"))){
//		printf("END\n");
		assembly_END(arg_1);
	}
	else if(!(strcmp(operator,"HALT"))){
//		printf("HALT\n");
		assembly_HALT();
	}
}

FILE* assembler(char* intermediate_code_file_name)
{
	/* Open intermediate code input file */
	FILE *input_file;
	struct memoryList *new_mem;
	
	/* auxiliar variables to work */
	char quadruple[1000];
	int len = strlen(quadruple);
	char *trim_quadruple;
	char operator[100], arg_1[100], arg_2[100], arg_3[100];
	
	/* allocate space to the trim_quadruple */
	trim_quadruple = (char*)malloc((len-1)*sizeof(char));
	if((input_file = fopen(intermediate_code_file_name,"r")) == NULL)
	{
		printf("Error! opening file\n");
		exit(1);
	}

	/* allocation of temporary variables */
	for(int i = 0; i < 16; i++){
		char *s, *t;
		s = strdup("\0");
		s = itoa(i,10);
		t = strdup("$t");
		new_mem = create_mem_list(strcat(t,s), 1, 1, next_free_memory_pos, 2 * 1, -1);
		if(begin_memList == NULL){
			begin_memList = new_mem;
			end_memList = new_mem;
		}else{
			end_memList->next = new_mem;
			end_memList = new_mem;
		}
		next_free_memory_pos += 2;
	}

	/* get each quadruple, trim it and ask for generation of assembly and memory table */
	while (fgets(quadruple, sizeof(quadruple), input_file) != NULL) {
    	/* generate the length  of the quadruple */
    	//printf("%s", quadruple); 
    	//printf("quad: %s\n", quadruple);
    	len = strlen(quadruple);
    	strncpy(trim_quadruple,quadruple+1,len);
		
		/* ignore unecessary chars*/
		trim_quadruple[len-2] = '\0';
		len = strlen(trim_quadruple);
		
		//printf("%s\n", trim_quadruple);
		/* get operator */
		strncpy(operator,strtok (trim_quadruple,","),len);
		//printf("1. %s\n", operator);
		
		/* get the first argument */
		strncpy(arg_1, strtok(NULL, ","),len);
		//printf("2. %s\n", arg_1);
		
		/* get the second argument */
		strncpy(arg_2, strtok(NULL, ","),len);
		//printf("3. %s\n", arg_2);

		/* get the third argument */
		strncpy(arg_3, strtok(NULL, ",)"),len);
		//printf("4. %s\n", arg_3);
		/* assemble this quadruple and add it at memory, if it is necessary */
		generate_assembly(operator, arg_1, arg_2, arg_3);
		//int test;
		//scanf("%d",&test);
    }
    //fclose(input_file);
}

void printMemTab()
{
	printf("|-------------------------------------------------------------------|\n");
	printf("|                            Memory Table                           |\n");
	printf("|-------------------------------------------------------------------|\n");
	printf("|    label    |  type  |  scope   | first_pos  |  size  | inst_line |\n");
	printf("|-------------------------------------------------------------------|\n");
	struct memoryList *memlist;
	memlist = begin_memList;
	while(memlist != NULL)
	{
		if(memlist->mem.scope == 2)
		{
			printf("|-------------------------------------------------------------------|\n");
		}
		printf("| %11s |", memlist->mem.label);
		
		if(memlist->mem.type == -1) printf("   -    |");
		else if(memlist->mem.type == 0) printf("  VOID  |");
		else if(memlist->mem.type == 1) printf("  INT   |");
		else printf("  INT[] |");

		if(memlist->mem.scope == 0) printf(" LOCAL    |");
		else if(memlist->mem.scope == 1) printf(" GLOBAL   |");
		else printf(" FUNCTION |");

		if(memlist->mem.first_pos == -1) printf("          - |");
		else printf("%11d |",  memlist->mem.first_pos);

		if(memlist->mem.memory_size == 0) printf("      - |");
		else printf(" %6d |", memlist->mem.memory_size);

		if(memlist->mem.instruction_line == -1) printf("         - |\n");
		else printf(" %9d |\n", memlist->mem.instruction_line);

		memlist = memlist->next;

	}
	printf("|-------------------------------------------------------------------|\n");
	
}

void printAssemblyCode()
{
	struct assembly_code *assembly;
	assembly = begin_assembly;
	while(assembly != NULL)
	{
		//printf("%d\n",assembly->inst.type);
		if(assembly->inst.type == 1){
			printf("%s", assembly->inst.operation);
			if(assembly->inst.parameter != NULL){
				printf(" %s \n", assembly->inst.parameter);
			}else{
				printf("\n");
			}	
		}
		assembly = assembly->next;
	}
}
/*************************************************************************************/
/*
 * Binary Code Generator
 *
 */

char *scope_bin = "GLOBAL";

int search_address(char *label)
{
	struct memoryList *memlist;
	memlist = begin_memList;
	
	while(memlist != NULL)
	{
		if(!strcmp(memlist->mem.label,label) && memlist->mem.scope == 1){
			return memlist->mem.first_pos;
		}
		memlist = memlist->next;
		if(memlist->mem.scope != 1){
			break;
		}
	}
		
	while(memlist != NULL)
	{
		if(!strcmp(memlist->mem.label,scope_bin)){
		
			memlist = memlist->next;
			while(memlist != NULL)
			{
				if(!strcmp(memlist->mem.label,label)){
					return memlist->mem.first_pos;
				}
				memlist = memlist->next;
			}
		}
		else{
			memlist = memlist->next;
		}
	}
	
	

	return -1;
}

int search_label(char *label)
{
	struct memoryList *memlist;
	memlist = begin_memList;
	
	while(memlist != NULL)
	{
		if(!strcmp(memlist->mem.label,label)){
			return memlist->mem.instruction_line;
		}
		memlist = memlist->next;
	}
	return -1;
}

void binary_PUSH(struct assembly_instruction assembly)
{
	if(isdigit(assembly.parameter[0])){
		int value = string_to_int(assembly.parameter);			
		char *bin_value = itoa(value,2);
		printf("0");
		for(int i = 0; i < 15 - strlen(bin_value);i++) printf("0");
		printf("%s", bin_value);
	}else{
		char *label;
		label = strdup(assembly.parameter);
	    memmove(label, label+1, strlen(label));
		int add = search_address(label);
		printf("0");
		char *bin_value = itoa(add,2);
		for(int i = 0; i < 15 - strlen(bin_value);i++) printf("0");
		printf("%s", bin_value);
	}
}

char* string_repeat( int n, const char * s ) {
  size_t slen = strlen(s);
  char * dest = malloc(n*slen+1);
 
  int i; char * p;
  for ( i=0, p = dest; i < n; ++i, p += slen ) {
    memcpy(p, s, slen);
  }
  *p = '\0';
  return dest;
}

char* complement_2(char* bin)
{
	int n = strlen(bin);
	int carry = 1;
	char *res;
	char digit;
	res = (char*)malloc(n * sizeof(char));
	for(int i = n - 1; i >= 0; i--){
		digit = (bin[i] == '0')? 1:0;
		res[i] = digit^carry + 48;
		carry = digit*carry;
	}
	//printf("res:%s\n",res );
	return res;
}

void binary_IF(struct assembly_instruction assembly)
{
	char *label;
	label = strdup(assembly.parameter);
    int inst = search_label(label);
    int diff = inst - assembly.line;
	int comp_2 = (diff < 0)? 1:0;
	
	printf("10011");
	char *bin_value = itoa(abs(diff),2);

	if(comp_2 == 1){
		char *str_zeros;
		str_zeros = string_repeat(11-strlen(bin_value),"0");
		strcat(str_zeros,bin_value);
		
		bin_value = complement_2(str_zeros);
	}

	for(int i = 0; i < 11 - strlen(bin_value);i++) printf("0");
	printf("%s", bin_value);
}

void binary_CALL(struct assembly_instruction assembly)
{
	char *label;
	label = strdup(assembly.parameter);
    int inst = search_label(label);
    int diff = inst - assembly.line;
	int comp_2 = (diff < 0)? 1:0;
	
	printf("10100");
	char *bin_value = itoa(abs(diff),2);

	if(comp_2 == 1){
		char *str_zeros;
		str_zeros = string_repeat(11-strlen(bin_value),"0");
		strcat(str_zeros,bin_value);
		
		bin_value = complement_2(str_zeros);
	}

	for(int i = 0; i < 11 - strlen(bin_value);i++) printf("0");
	printf("%s", bin_value);
}

void binary_WRITE(struct assembly_instruction assembly)
{
	
	char *label;
	label = strdup(assembly.parameter);
    memmove(label, label+1, strlen(label));
	int add = search_address(label);
	printf("1");
	char *bin_value = itoa(add,2);
	for(int i = 0; i < 15 - strlen(bin_value);i++) printf("0");
	printf("%s", bin_value);
}


void generate_binary()
{
	struct assembly_code *assembly;
	assembly = begin_assembly;
	while(assembly != NULL)
	{
		if(!strcmp(assembly->inst.operation,"FUN")){
			//printf("-------- FUN %s --------\n",assembly->inst.parameter);
			scope_bin = strdup(assembly->inst.parameter);
		}
		if(!strcmp(assembly->inst.operation,"PUSH")){
			//printf("PUSH:\n\t");
			binary_PUSH(assembly->inst);
			printf("\n");
		}
		if(!(strcmp(assembly->inst.operation,"AND"))){
			//printf("AND:\n\t");
			printf("1000001000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"OR"))){
			//printf("OR:\n\t");
			printf("1000011000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"XOR"))){
			//printf("XOR:\n\t");
			printf("1000101000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"+"))){
			//printf("+:\n\t");
			printf("1000100000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"-"))){
			//printf("-:\n\t");
			printf("1000010000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"--"))){
			//printf("--:\n\t");
			printf("1000110000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"LR"))){
			//printf("LR:\n\t");
			printf("1000000000000001\n");
		}
		if(!(strcmp(assembly->inst.operation,"AL"))){
			//printf("AL:\n\t");
			printf("1000000000000010\n");
		}
		if(!(strcmp(assembly->inst.operation,"AR"))){
			//printf("AR:\n\t");
			printf("1000000000000011\n");
		}
		if(!(strcmp(assembly->inst.operation,"/"))){
			//printf("/:\n\t");
			printf("1000000100000100\n");
		}
		if(!(strcmp(assembly->inst.operation,"*"))){
			//printf("*:\n\t");
			printf("1000000100000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"LEQ"))){
			//printf("LEQ:\n\t");
			printf("1000000110000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"DUP"))){
			//printf("DUP:\n\t");
			printf("1000000001000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"OVER"))){
			//printf("OVER:\n\t");
			printf("1000111000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"EXIT"))){
			//printf("EXIT:\n\t");
			printf("1000000000100000\n");
		}
		if(!(strcmp(assembly->inst.operation,"R>"))){
			//printf("R>:\n\t");
			printf("1000000001000000\n");
		}
		if(!(strcmp(assembly->inst.operation,">R"))){
			//printf(">R:\n\t");
			printf("1000000001010000\n");
		}
		if(!(strcmp(assembly->inst.operation,"DROP"))){
			//printf("DROP:\n\t");
			printf("1110111000000001\n");
		}
		if(!(strcmp(assembly->inst.operation,"SWAP"))){
			//printf("SWAP:\n\t");
			printf("1000111001000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"NOP"))){
			//printf("NOP:\n\t");
			printf("1000000000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"HALT"))){
			//printf("HALT:\n\t");
			printf("1110000000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"INPUT READ"))){
			//printf("INPUT READ:\n\t");
			printf("1100000000000001\n");
		}
		if(!(strcmp(assembly->inst.operation,"INPUT LOAD"))){
			//printf("INPUT LOAD:\n\t");
			printf("1100000000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"OUTPUT SEND"))){
			//printf("OUTPUT SEND:\n\t");
			printf("0000000000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"OUTPUT PRINT"))){
			//printf("OUTPUT PRINT:\n\t");
			printf("1101000000000000\n");
		}

		/* Branch and memory instructions  */
		if(!(strcmp(assembly->inst.operation,"IF"))){
			//printf("IF:\n\t");
			binary_IF(assembly->inst);			
			printf("\n");
		}
		if(!(strcmp(assembly->inst.operation,"CALL"))){
			//printf("CALL:\n\t");
			binary_CALL(assembly->inst);			
			printf("\n");
		}
		if(!(strcmp(assembly->inst.operation,"!"))){
			//printf("!:\n\t");
			printf("1111000000000000\n");
		}
		if(!(strcmp(assembly->inst.operation,"@"))){
			//printf("@:\n\t");
			printf("1110100000000000\n");
		}

		assembly = assembly->next;
	}

}

int main(int argc, char **argv)
{
	//FILE *input_file;

	printf("%d\n",argc );
	printf("Running assembler for %s\n", argv[1]);
	
	assembler(argv[1]);	

	int out = open("memorytab.txt", O_WRONLY | O_TRUNC | O_CREAT,0600);
    if (-1 == out) { perror("opening memorytab.txt"); return 255; }
	int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
   	
   	printMemTab();

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

    out = open("assemblyCode.txt", O_WRONLY | O_TRUNC | O_CREAT,0600);
    if (-1 == out) { perror("opening assemblyCode.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
	printAssemblyCode();

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

    out = open("binaryCode.txt", O_WRONLY | O_TRUNC | O_CREAT,0600);
    if (-1 == out) { perror("opening binaryCode.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }

    generate_binary("assemblyCode.txt");

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

	return 0;
}
