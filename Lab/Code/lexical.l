%{
  //#include "syntax.tab.h"
  #define TEST_lexical
%}
delim [ \r\t\n]
letter [_a-zA-Z]
digit [0-9]
%%
{delim}+ {}
[1-9]{digit}*|0 {
                  #ifdef TEST_lexical 
                  printf("INT:%s\n",yytext);
                  #endif
                 }
([1-9]{digit}*|0).({digit}*[1-9]) {
				   #ifdef TEST_lexical
				   printf("FLOAT:%s\n",yytext);
				   #endif
				   }
int|float {
           #ifdef TEST_lexical
           printf("TYPE:%s\n",yytext);
           #endif
           }
struct {
        #ifdef TEST_lexical
        printf("STRUCT\n");
        #endif
        }
return {
        #ifdef TEST_lexical
        printf("RETURN\n");
        #endif
        }
        
if {
    #ifdef TEST_lexical
    printf("IF\n");
    #endif
    }
else {
      #ifdef TEST_lexical
      printf("ELSE\n");
      #endif
      }
while {
       #ifdef TEST_lexical
       printf("WHILE\n");
       #endif
       }
{letter}({letter}|{digit})* {
			          #ifdef TEST_lexical
			          printf("ID:%s\n",yytext);
			          #endif
		               }    
";" {
     #ifdef TEST_lexical 
     printf("SEMI\n");
     #endif
     }
"," {
     #ifdef TEST_lexical
     printf("COMMA\n");
     #endif
     }
"=" {
     #ifdef TEST_lexical
     printf("ASSIGNOP\n");
     #endif
     }
">"|"<"|">="|"<="|"=="|"!=" {
                             #ifdef TEST_lexical
                             printf("RELOP:%s\n",yytext);
                             #endif
                             }
"+" {
     #ifdef TEST_lexical
     printf("PLUS\n");
     #endif
     }
"-" {
     #ifdef TEST_lexical
     printf("MINUS\n");
     #endif
     }
"*" {
     #ifdef TEST_lexical
     printf("STAR\n");
     #endif
     }
"/" {
     #ifdef TEST_lexical
     printf("DIV\n");
     #endif
     }
"&&" {
      #ifdef TEST_lexical
      printf("AND\n");
      #endif
      }
"||" {
      #ifdef TEST_lexical
      printf("OR\n");
      #endif
      }
"." {
     #ifdef TEST_lexical
     printf("DOT\n");
     #endif
     }
"!" {
     #ifdef TEST_lexical
     printf("NOT\n");
     #endif
     }
"(" {
     #ifdef TEST_lexical
     printf("LP\n");
     #endif
     }
")" {
     #ifdef TEST_lexical
     printf("RP\n");
     #endif
     }
"[" {
     #ifdef TEST_lexical
     printf("LB\n");
     #endif
     }
"]" {
     #ifdef TEST_lexical
     printf("RB\n");
     #endif
     }
"{" {
     #ifdef TEST_lexical
     printf("LC\n");
     #endif
     }
"}" {
     #ifdef TEST_lexical
     printf("RC\n");
     #endif
     }
. {
   printf("Error type A at Line %d: Mysterious characters \'%s\'\n",yylineno,yytext);
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
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
                             
