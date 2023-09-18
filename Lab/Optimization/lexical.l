%{
     #include "syntax.tab.h"
     #include <stdlib.h>
     #include <string.h>
     //#define TEST_lexical
     #define newOp (Operand)malloc(sizeof(struct Operand_))
     char* newID(char* id,int size){
          char* ret = (char*)malloc(sizeof(char)*size);
          strcpy(ret,id);
          return ret;
     }

     Operand newOpV(char* var_name){
          Operand ret = newOp;
          ret->kind = VARIABLE;
          if(var_name == NULL)
               var_name = "t0";
          ret->u.var_name = var_name;
          return ret;
     }

     Operand newOpC(int value){
          Operand ret = newOp;
          ret->kind = CONSTANT;
          ret->u.value = value;
          return ret;
     }

     Operand newOpA(char* var_name){
          Operand ret = newOp;
          ret->kind = ADDRESS;
          ret->u.var_name = var_name;
          return ret;
     }

     Operand newOpR(char* var_name){
          Operand ret = newOp;
          ret->kind = REFER;
          ret->u.var_name = var_name;
          return ret;
     }
%}

delim [ \t]
letter [_a-zA-Z]
digit [0-9]
%%
{delim}+ {}
\r|\n {}
#{digit}+ {
     #ifdef TEST_lexical
     printf("CONSTANT:%s\n",yytext);
     #endif
     yylval = (void*)newOpC(atoi(yytext+1));
     return _CONSTANT;
}
&(t|v){digit}+ {
     #ifdef TEST_lexical
     printf("REFER:%s\n",yytext);
     #endif
     yylval = (void*)newOpR(newID(yytext+1,10));
     return _REFER;
}
\*(t|v){digit}+ {
     #ifdef TEST_lexical
     printf("ADDRESS:%s\n",yytext);
     #endif
     yylval = (void*)newOpA(newID(yytext+1,10));
     return _ADDRESS;
}
(t|v){digit}+ {
     #ifdef TEST_lexical
     printf("VARIABLE:%s\n",yytext);
     #endif
     yylval = (void*)newOpV(newID(yytext,10));
     return _VARIABLE;
}
{digit}+ {
     #ifdef TEST_lexical
     printf("SIZE:%s\n",yytext);
     #endif
     yylval = (void*)newOpC(atoi(yytext));
     return SIZE;
}
((label)|l){digit}+ {
     #ifdef TEST_lexical
     printf("label:%s\n",yytext);
     #endif
     yylval = (void*)newID(yytext,10);
     return LABELNAME;
}
DEC {
     #ifdef TEST_lexical
     printf("DEC\n");
     #endif
     return _DEC;
}
ARG {
     #ifdef TEST_lexical
     printf("ARG\n");
     #endif
     return _ARG;
}
RETURN {
     #ifdef TEST_lexical
     printf("RETURN\n");
     #endif
     return RETURN;
}
CALL {
     #ifdef TEST_lexical
     printf("CALL\n");
     #endif
     return _CALL;     
}
PARAM {
     #ifdef TEST_lexical
     printf("PARAM\n");
     #endif
     return _PARAM;    
}
READ {
     #ifdef TEST_lexical
     printf("READ\n");
     #endif
     return _READ;   
}
WRITE {
     #ifdef TEST_lexical
     printf("WRITE\n");
     #endif
     return _WRITE;       
}
IF {
     #ifdef TEST_lexical
     printf("IF\n");
     #endif
     return IF;
}
GOTO {
     #ifdef TEST_lexical
     printf("GOTO\n");
     #endif
     return _GOTO;
}
":" {
     #ifdef TEST_lexical 
     printf("COLON\n");
     #endif
     return COLON;
}
":=" {
     #ifdef TEST_lexical
     printf("ASSIGN\n");
     #endif
     return _ASSIGN;
}
">"|"<"|">="|"<="|"=="|"!=" {
     #ifdef TEST_lexical
     printf("RELOP:%s\n",yytext);
     #endif
     yylval = (void*)newID(yytext,3);
     return RELOP;
}
"+" {
     #ifdef TEST_lexical
     printf("ADD\n");
     #endif
     return _ADD;
}
"-" {
     #ifdef TEST_lexical
     printf("SUB\n");
     #endif
     return _SUB;
}
"*" {
     #ifdef TEST_lexical
     printf("MUL\n");
     #endif
     return _MUL;
}
"/" {
     #ifdef TEST_lexical
     printf("DIV\n");
     #endif
     return _DIV;
}
LABEL {
     #ifdef TEST_lexical
     printf("LABEL\n");
     #endif
     return _LABEL;
}
FUNCTION {
     #ifdef TEST_lexical
     printf("FUNCTION\n");
     #endif
     return FUNCTION;
}
{letter}({letter}|{digit})* {
     #ifdef TEST_lexical
     printf("ID:%s\n",yytext);
     #endif
     yylval = (void*)newOpV(newID(yytext,20));
     return ID;
}
%%