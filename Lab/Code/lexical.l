%option yylineno
%{
     #include "syntax.tab.h"
     #include <stdlib.h>
     #include <string.h>
     //#define TEST_lexical
     int o_atoi(char* text);
     int h_atoi(char* text);
     int yycolumn = 1;
     #define YY_USER_ACTION \
          yylloc.first_line = yylloc.last_line = yylineno; \
          yylloc.first_column = yycolumn; \
          yycolumn += yyleng; \
          yylloc.last_column = yycolumn - 1;
     
     #define YY_RET_VAL(type) \ 
          yylval = (struct YYNODE*) malloc(sizeof(yynode)); \ 
          yylval -> YYTYPE = type ; \ 
          yylval -> TG = 0;
     extern int toP;
%}

delim [ \t]
letter [_a-zA-Z]
digit [0-9]
%%
{delim}+ {}
\r|\n {yycolumn = 1;}
0[0-7]+ {
     #ifdef TEST_lexical
     printf("octal INT:%s\n",yytext);
     #else
     YY_RET_VAL(INT)
     yylval->val_int = o_atoi(yytext);
     return INT;
     #endif
}
0{digit}+ {
     YY_RET_VAL(INT)
     yylval->val_int = 0;
     printf("Error type A at Line %d: Illegal octal number \'%s\'\n",yylineno,yytext);
     toP=0;
     return INT;
}
0(x|X)[0-9a-fA-F]+ {
     #ifdef TEST_lexical
     printf("hexadecimal INT:%s\n",yytext);
     #endif
     YY_RET_VAL(INT)
     yylval->val_int = h_atoi(yytext);
     return INT;     
}
[1-9]{digit}*|0 {
     #ifdef TEST_lexical 
     printf("INT:%s\n",yytext);
     #endif
     YY_RET_VAL(INT)
     yylval->val_int = atoi(yytext);
     return INT;
}
([1-9]{digit}*|0)\.({digit}+) {
	#ifdef TEST_lexical
	printf("FLOAT:%s\n",yytext);
	#endif
     YY_RET_VAL(FLOAT)
     yylval -> val_float = (float)atof(yytext);
     return FLOAT;
}
int|float {
     #ifdef TEST_lexical
     printf("TYPE:%s\n",yytext);
     #endif
     YY_RET_VAL(TYPE)
     yylval -> val_type = (char*)malloc(sizeof(char)*yyleng+1);
     strcpy(yylval->val_type,yytext);
     return TYPE;
}
struct {
     #ifdef TEST_lexical
     printf("STRUCT\n");
     #endif
     YY_RET_VAL(STRUCT)
     return STRUCT;
}
return {
     #ifdef TEST_lexical
     printf("RETURN\n");
     #endif
     YY_RET_VAL(RETURN)
     return RETURN;
}
        
if {
     #ifdef TEST_lexical
     printf("IF\n");
     #endif
     YY_RET_VAL(IF)
     return IF;
}
else {
     #ifdef TEST_lexical
     printf("ELSE\n");
     #endif
     YY_RET_VAL(ELSE)
     return ELSE;
}
while {
     #ifdef TEST_lexical
     printf("WHILE\n");
     #endif
     YY_RET_VAL(WHILE)
     return WHILE;
}
{letter}({letter}|{digit})* {
	#ifdef TEST_lexical
	printf("ID:%s\n",yytext);
	#endif
     YY_RET_VAL(ID)
     yylval->val_ID = (char*)malloc(sizeof(char)*yyleng+1);
     strcpy(yylval->val_ID,yytext);
     return ID;
}    
";" {
     #ifdef TEST_lexical 
     printf("SEMI\n");
     #endif
     YY_RET_VAL(SEMI)
     return SEMI;
}
"," {
     #ifdef TEST_lexical
     printf("COMMA\n");
     #endif
     YY_RET_VAL(COMMA)
     return COMMA;
}
"=" {
     #ifdef TEST_lexical
     printf("ASSIGNOP\n");
     #endif
     YY_RET_VAL(ASSIGNOP)
     return ASSIGNOP;
}
">"|"<"|">="|"<="|"=="|"!=" {
     #ifdef TEST_lexical
     printf("RELOP:%s\n",yytext);
     #endif
     YY_RET_VAL(RELOP)
     yylval->val_relop = yytext;
     return RELOP;
}
"+" {
     #ifdef TEST_lexical
     printf("PLUS\n");
     #endif
     YY_RET_VAL(PLUS)
     return PLUS;
}
"-" {
     #ifdef TEST_lexical
     printf("MINUS\n");
     #endif
     YY_RET_VAL(MINUS)
     return MINUS;
}
"*" {
     #ifdef TEST_lexical
     printf("STAR\n");
     #endif
     YY_RET_VAL(STAR)
     return STAR;
}
"/" {
     #ifdef TEST_lexical
     printf("DIV\n");
     #endif
     YY_RET_VAL(DIV)
     return DIV;
}
"&&" {
     #ifdef TEST_lexical
     printf("AND\n");
     #endif
     YY_RET_VAL(AND)
     return AND;
}
"||" {
     #ifdef TEST_lexical
     printf("OR\n");
     #else
     YY_RET_VAL(OR)
     return OR;
     #endif
}
"." {
     #ifdef TEST_lexical
     printf("DOT\n");
     #endif
     YY_RET_VAL(DOT)
     return DOT;
     }
"!" {
     #ifdef TEST_lexical
     printf("NOT\n");
     #endif
     YY_RET_VAL(NOT)
     return NOT;
}
"(" {
     #ifdef TEST_lexical
     printf("LP\n");
     #endif
     YY_RET_VAL(LP)
     return LP;
}
")" {
     #ifdef TEST_lexical
     printf("RP\n");
     #endif
     YY_RET_VAL(RP)
     return RP;
}
"[" {
     #ifdef TEST_lexical
     printf("LB\n");
     #endif
     YY_RET_VAL(LB)
     return LB;
}
"]" {
     #ifdef TEST_lexical
     printf("RB\n");
     #endif
     YY_RET_VAL(RB)
     return RB;
}
"{" {
     #ifdef TEST_lexical
     printf("LC\n");
     #endif
     YY_RET_VAL(LC)
     return LC;
}
"}" {
     #ifdef TEST_lexical
     printf("RC\n");
     #endif
     YY_RET_VAL(RC)
     return RC;
}
. {
   printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext);
   toP = 0;
   }
%%

#ifdef TEST_lexical
int main(int argc, char** argv){
  if (argc > 1){
    if(!(yyin = fopen(argv[1],"r"))){
      perror(argv[1]);
      return 1;
    }
  }
  yylex();
  return 0;
}
#endif     

int o_atoi(char* text){
     int i = 1;
     int ret = 0;
     while(text[i]!='\0'){
          ret *= 8;
          ret += text[i]-'0';
          i++;
     }
     return ret;
}
                             
int h_atoi(char* text){
     int i = 2;
     int ret = 0;
     while(text[i]!='\0'){
          ret *= 16;
          if(text[i]<='9'&&text[i]>='0')
               ret += text[i]-'0';
          else if(text[i]<='f'&&text[i]>='a')
               ret += text[i]-'a'+10;
          else if(text[i]<='F'&&text[i]>='A')
               ret += text[i]-'A'+10;
          i++;
     }
     return ret;
}                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
