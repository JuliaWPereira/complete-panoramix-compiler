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
#include "assembler.h" /* header of this file */

FILE* assembler(char* intermediate_code_file_name)
{
	/* Open intermediate code input file */
	FILE *input_file;

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

	/* get each quadruple, trim it and ask for generation of assembly and memory table */
	while (fgets(quadruple, sizeof(quadruple), input_file)) {
    	/* generate the length  of the quadruple */
    	printf("%s", quadruple); 
    	len = strlen(quadruple);
    	strncpy(trim_quadruple,quadruple+1,len-3);
		
		/* ignore unecessary chars*/
		trim_quadruple[len-2] = '\0';
		len = strlen(trim_quadruple);
		
		/* get operator */
		strncpy(operator,strtok (trim_quadruple,","),len);
		printf("%s\n", operator);
		
		/* get the first argument */
		strncpy(arg_1, strtok(NULL, ","),len);
		printf("%s\n", arg_1);
		
		/* get the second argument */
		strncpy(arg_2, strtok(NULL, ","),len);
		printf("%s\n", arg_2);

		/* get the third argument */
		strncpy(arg_3, strtok(NULL, ","),len);
		printf("%s\n", arg_3);

    }

    /* close file */
	fclose(input_file);
}

int main(int argc, char **argv)
{
	assembler("../Outputs/InterCodeTest/interCode.txt");

	//FILE *file;
	//file = fopen("../Outputs/InterCodeTest/interCode.txt","r");

	/*char quadruple[1000];
	char op[7], arg1[20], arg2[20], arg3[20];
    if ((file = fopen("../Outputs/InterCodeTest/interCode.txt","r")) == NULL) {
        printf("Error! opening file");
        // Program exits if file pointer returns NULL.
        exit(1);
    }

    // reads text until newline is encountered
    fscanf(file, "%[^\n]", quadruple);
    printf("Data from the file:\n%s", quadruple);
	int len = strlen(quadruple);
	char *trim_quadruple;
	trim_quadruple = (char*)malloc((len-1)*sizeof(char));
	strncpy(trim_quadruple,quadruple+1,len-2);
	trim_quadruple[len-2] = '\0';
	
	printf("\n\n>>%s<<\n\n", trim_quadruple);
	char *parameters;
    parameters = strtok (trim_quadruple,",");
    while (parameters != NULL) {
        printf("%s\n", parameters);
        parameters = strtok (NULL, ",");
    }

	
    fclose(file);*/
	return 0;
}
