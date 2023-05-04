#include <stdio.h>
//#define YYDEBUG
extern int yydebug;
extern struct YYNODE* root;
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
    typeCheck(root);
    translate(root,argv[2]);
    return 0;
}