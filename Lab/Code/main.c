#include <stdio.h>
//#define YYDEBUG
extern int yydebug;

int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r");
    if (!f)
    {
    perror(argv[1]);
    return 1;
    }
    yyrestart(f);
    #ifdef YYDEBUG
    yydebug =1 ;
    #endif
    yyparse();
    return 0;
}