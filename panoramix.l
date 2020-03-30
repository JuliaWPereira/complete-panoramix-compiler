/**********************************************
 *
 *   Panoramix Compiler - Scanner
 *   
 *   @version 1.1
 *   @edited November 29, 2019
 *   @author Júlia Wotzasek Pereira
 *
**********************************************/

/* Options used:
 *  - yywrap: Do not continue to the next file
 *  - nodefault: Not to add a default rule and
 *      rather to report an error if the
 *      input rules don't cover all possible
 *      input.
 *  - yylineno: Define a integer variable called
 *      yylineno and maintain the current 
 *      number of it. Every time the Scanner
 *      reads a newline character, it increments
 *      yylineno, and if the scanner backs up
 *      over a newline, it decrements it.
 *      WARNING: you should initialize yyline to
 *      1 at the beginning of each file
 *  - batch: Forces the scanner to look ahead
 *      always. It is a good ideia because it is 
 *      intend to always read from a file.
*/

%option noyywrap nodefault yylineno batch

/* Start states:
 *  - COMMENT: Used to skip comments
*/

%x COMMENT

%{
  #include "AST.h"
  #include "panoramix.tab.h"
%}

%%

 /* Comments: With the state comment created,
  * I can open and close comments easily,
  * just using a machine state. Notice that
  * all contents of the comment is skipped.
  * If the comment has not been closed, it 
  * sends a warning.
 */

"/*" {BEGIN(COMMENT);}
<COMMENT>"*/" {BEGIN(INITIAL);}
<COMMENT>([^*]|\n)+|.
<COMMENT><<EOF>> { printf("%d: Unterminated comment\n", yylineno); return 0; }

 /* single character special symbols */
"+"  {return '+';} 
"-"  {return '-';}
"*"  {return '*';}
"/"  {return '/';}
"="  {return '=';}
";"  {return ';';}
","  {return ',';}
"("  {return '(';}
")"  {return ')';}
"["  {return '[';}
"]"  {return ']';}
"{"  {return '{';}
"}"  {return '}'; }

 /* Comparision operations. CMP is the token to comparations */
">"   {yylval.fn = 1; return CMP;}
"<"   {yylval.fn = 2; return CMP;}
"!="  {yylval.fn = 3; return CMP;}
"=="  {yylval.fn = 4; return CMP;}
">="  {yylval.fn = 5; return CMP;}
"<="  {yylval.fn = 6; return CMP;}

 /* keywords */
"else" {return ELSE;}
"if" {return IF;}
"int" {return INT;}
"return" {return RETURN;}
"void" {return VOID;}
"while" {return WHILE;}

 /* identifier and Numbers */
[a-zA-Z]+[0-9][0-9a-zA-Z]* {printf("%d: error: lexical error: %s \n",yylineno,yytext);}
[a-zA-Z]+ {yylval.s = createsymbol(yytext); return ID;}
([0-9]+[a-zA-Z]+)+ {printf("%d: error: lexical error: %s \n",yylineno, yytext);}
[0-9]+ {yylval.i = atol(yytext); return NUM;}

[ \t] {/* ignore whitespaces */}
\n  {/* ignore */}

. {printf("%d: error: lexical error: %c \n",yylineno,*yytext);}

%%


