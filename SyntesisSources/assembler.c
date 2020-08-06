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
#include "assembler.h" /* header of this file */

struct memoryList* create_mem_list(char *label, int type, int scope, int first_pos, int memory_size)
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

/*
 * In FUN, we should allocate a memory field to start the scope of the function 
 *
*/
void assembly_FUN(char* arg_1, char* arg_2)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	int type;
	type = (!strcmp(arg_1, "INT"))? 1:0;
	new_mem = create_mem_list(arg_2, type, 2, next_free_memory_pos, 2);
	
	if(begin_memList == NULL){
		begin_memList = new_mem;
		end_memList = new_mem;
	}else{
		end_memList->next = new_mem;
		end_memList = new_mem;
	}
	next_free_memory_pos += 2;

	assembly = create_assembly_code("FUN", NULL, 0, assembly_code_line++, 0);
	if(begin_assembly == NULL){
		begin_assembly = assembly;
		end_assembly = assembly;
	}else{
		end_assembly->next = assembly;
		end_assembly = assembly;
	}
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

void assembly_ALLOC(char* arg_1, char* arg_2, char* arg_3)
{
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	int type;
	int size;
	int scope;
	type = (!strcmp(arg_3, "-") | !strcmp(arg_3, "--"))? 1:2;
	size = (!strcmp(arg_3, "-") | !strcmp(arg_3, "--"))? 1:string_to_int(arg_3);
	scope = (!strcmp(arg_2, "GLOBAL"))? 1:0;
	//printf("%d - %s\n", size, arg_3);
	new_mem = create_mem_list(arg_1, type, scope, next_free_memory_pos, size * 2);

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

		assembly = create_assembly_code("ADD", NULL, 0, assembly_code_line++, 1);
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
	struct memoryList *new_mem;
	struct assembly_code *assembly;
	char *s, *t;
	printf(">>%s\n", arg_2);

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

void generate_assembly(char* operator, char* arg_1, char* arg_2, char* arg_3)
{
	if(!(strcmp(operator,"FUN"))){
		printf("FUN\n");
		assembly_FUN(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"ALLOC"))){
		printf("ALLOC\n");
		assembly_ALLOC(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"LOAD"))){
		printf("LOAD\n");
		assembly_LOAD(arg_1,arg_2,arg_3);
	}
	else if(!(strcmp(operator,"STORE"))){
		printf("STORE\n");
		assembly_STORE(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"ASSIGN"))){
		printf("ASSIGN\n");
		assembly_ASSIGN(arg_1,arg_2);
	}
	else if(!(strcmp(operator,"CALL"))){
		printf("CALL\n");
		assembly_CALL(arg_1,arg_2);
	}
}

char* itoa(int val, int base){
	if(val == 0) return "0";
	
	static char buf[32] = {0};
	
	int i = 30;
	
	for(; val && i ; --i, val /= base)
	
		buf[i] = "0123456789abcdef"[val % base];
	
	return &buf[i+1];
	
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
		new_mem = create_mem_list(strcat(t,s), 1, 1, next_free_memory_pos, 2 * 1);
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
	while (fgets(quadruple, sizeof(quadruple), input_file)) {
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
    }

    /* close file */
	fclose(input_file);
}

void printMemTab()
{
	printf("|-------------------------------------------------------|\n");
	printf("|                      Memory Table                     |\n");
	printf("|-------------------------------------------------------|\n");
	printf("|    label    |  type  |  scope   | first_pos  |  size  |\n");
	printf("|-------------------------------------------------------|\n");
	struct memoryList *memlist;
	memlist = begin_memList;
	while(memlist != NULL)
	{
		printf("| %11s |", memlist->mem.label);
		
		if(memlist->mem.type == 0) printf("  VOID  |");
		else if(memlist->mem.type == 1) printf("  INT   |");
		else printf("  INT[] |");

		if(memlist->mem.scope == 0) printf(" LOCAL    |");
		else if(memlist->mem.scope == 1) printf(" GLOBAL   |");
		else printf(" FUNCTION |");

		printf("%11d | %6d |\n",  memlist->mem.first_pos, memlist->mem.memory_size);
		memlist = memlist->next;
	}
	printf("|-------------------------------------------------------|\n");
	
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

int main(int argc, char **argv)
{
	assembler("../Outputs/InterCodeTest/interCode.txt");
	
	int out = open("memorytab.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening memorytab.txt"); return 255; }
	int save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
   
	printMemTab();

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

    out = open("assemblyCode.txt", O_RDWR|O_CREAT, 0600);
    if (-1 == out) { perror("opening assemblyCode.txt"); return 255; }
	save_out = dup(fileno(stdout));
    if (-1 == dup2(out, fileno(stdout))) { perror("cannot redirect stdout"); return 255; }
    
	printAssemblyCode();

	fflush(stdout); close(out);
    dup2(save_out, fileno(stdout));
    close(save_out);

	return 0;
}
