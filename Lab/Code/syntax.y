%locations
%{  
    typedef struct YYNODE{
        int YYTYPE;
        int TG;
        int first_line;
        union {
            float val_float;
            int val_int;
            char* val_type;
            char* val_ID;
            char* val_relop;
        };
        struct YYNODE* next, * sons;
    }yynode;
    #define YY_TREE_ACTION($$,k,$1,yytype) \ 
        $$ = (struct YYNODE*) malloc(sizeof(yynode)); \ 
        $$->first_line = k.first_line; \ 
        $$->sons = $1; \ 
        $$->YYTYPE = YYSYMBOL_##yytype ;  \
        $$->TG = 1;
    #define YYSTYPE struct YYNODE*
    #include "lex.yy.c"
    #include <stdio.h>
    #define YYDEBUG 1
    #define caseToken(token) case YYSYMBOL_##token : return #token ; break;
    void PrintTree(struct YYNODE* root,int depth);
    char* enum2s(int val);
    #define true 1
    #define false 0
    int toP = true;
        
%}

%token INT
%token FLOAT
%token ID
%token SEMI
%token COMMA
%token ASSIGNOP
%token RELOP
%token PLUS MINUS STAR DIV
%token AND OR 
%token DOT
%token NOT
%token TYPE
%token LP RP 
%token LB RB
%token LC RC
%token STRUCT
%token RETURN
%token IF
%token ELSE
%token WHILE

%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT SIG_MINUS
%left LP RP LB RB DOT

%%
Program : ExtDefList {YY_TREE_ACTION($$,@$,$1,Program)if(toP)PrintTree($$,0);}
    ;
ExtDefList : ExtDef ExtDefList {YY_TREE_ACTION($$,@$,$1,ExtDefList) if($1)$1->next = $2;}
    |   {$$ = NULL;}
    ;
ExtDef : Specifier ExtDecList SEMI {YY_TREE_ACTION($$,@$,$1,ExtDef) $1->next = $2;$2->next = $3;}
    | Specifier SEMI {YY_TREE_ACTION($$,@$,$1,ExtDef) $1->next = $2;}
    | Specifier FunDec CompSt {YY_TREE_ACTION($$,@$,$1,ExtDef) $1->next = $2;$2->next = $3;}
    | error {$$ =  NULL;toP = false;}
    | error SEMI{$$ =  NULL;toP = false;}
    ;
ExtDecList : VarDec {YY_TREE_ACTION($$,@$,$1,ExtDecList)}
    | VarDec COMMA ExtDecList {YY_TREE_ACTION($$,@$,$1,ExtDecList) $1->next = $2;$2->next = $3;}
    ;
Specifier : TYPE {YY_TREE_ACTION($$,@$,$1,Specifier)}
    | StructSpecifier {YY_TREE_ACTION($$,@$,$1,Specifier)}
    ;
StructSpecifier : STRUCT OptTag LC DefList RC {
    YY_TREE_ACTION($$,@$,$1,StructSpecifier)
    if($2){
        $1->next = $2;
        $2->next = $3;
    }else{
        $1->next = $3;
    }
    if($4){
        $3->next = $4;
        $4->next = $5;
    }else{
        $3->next = $5;
    }
}
    | STRUCT Tag {YY_TREE_ACTION($$,@$,$1,StructSpecifier) $1->next = $2;}
    ;
OptTag : ID {YY_TREE_ACTION($$,@$,$1,OptTag)}
    | {$$ =  NULL;}
    ;
Tag : ID {YY_TREE_ACTION($$,@$,$1,Tag)}
    ;
VarDec : ID {YY_TREE_ACTION($$,@$,$1,VarDec)}
    | VarDec LB INT RB {YY_TREE_ACTION($$,@$,$1,VarDec) $1->next = $2;$2->next = $3;$3->next = $4;}
    ;
FunDec : ID LP VarList RP {YY_TREE_ACTION($$,@$,$1,FunDec) $1->next = $2;$2->next = $3;$3->next = $4;}
    | ID LP RP {YY_TREE_ACTION($$,@$,$1,FunDec) $1->next = $2;$2->next = $3;}
    ;
VarList : ParamDec COMMA VarList {YY_TREE_ACTION($$,@$,$1,VarList) $1->next = $2;$2->next = $3;}
    | ParamDec {YY_TREE_ACTION($$,@$,$1,VarList)}
    ;
ParamDec : Specifier VarDec {YY_TREE_ACTION($$,@$,$1,ParamDec) $1->next = $2;}
    ;
CompSt : LC DefList StmtList RC {YY_TREE_ACTION($$,@$,$1,CompSt) 
    if($2){
        $1->next = $2;
        if($3){$2->next = $3; $3->next = $4;}
        else $2->next = $4;
    }else {
        if($3){$1->next = $3;$3->next = $4;}
        else $1->next = $4;
    }
}
    | error RC {$$ =  NULL;toP = false;}
    ;
StmtList : Stmt StmtList {YY_TREE_ACTION($$,@$,$1,StmtList) if($1)$1->next = $2;}
    |   {$$ =  NULL;}
    ;
Stmt : Exp SEMI {YY_TREE_ACTION($$,@$,$1,Stmt) if($1)$1->next=$2;}
    | CompSt {YY_TREE_ACTION($$,@$,$1,Stmt)}
    | RETURN Exp SEMI {YY_TREE_ACTION($$,@$,$1,Stmt)$1->next=$2;if($2)$2->next=$3;}
    | IF LP Exp RP Stmt{YY_TREE_ACTION($$,@$,$1,Stmt)$1->next=$2;$2->next=$3;if($3)$3->next=$4;$4->next=$5;}
    | IF LP Exp RP Stmt ELSE Stmt{YY_TREE_ACTION($$,@$,$1,Stmt)$1->next=$2;$2->next=$3;if($3)$3->next=$4;$4->next=$5;if($5)$5->next=$6;$6->next=$7;}
    | WHILE LP Exp RP Stmt{YY_TREE_ACTION($$,@$,$1,Stmt)$1->next=$2;$2->next=$3;if($3)$3->next=$4;$4->next=$5;}
    | error SEMI{$$ =  NULL;toP = false;}
    | error {$$ =  NULL;toP = false;}
    ;
DefList : Def DefList {YY_TREE_ACTION($$,@$,$1,DefList) $1->next=$2;}
    | {$$ =  NULL;}
    | error {$$ = NULL;toP = 0;}
    ;
Def : Specifier DecList SEMI {
    YY_TREE_ACTION($$,@$,$1,Def)
    if($2){
        $1->next=$2;$2->next = $3;
    }else $1->next = $3;
}
    ;
DecList : Dec {YY_TREE_ACTION($$,@$,$1,DecList)}
    | Dec COMMA DecList {YY_TREE_ACTION($$,@$,$1,DecList)$1->next=$2;$2->next=$3;}
    | error {$$ = NULL;toP = 0;}
    ;
Dec : VarDec {YY_TREE_ACTION($$,@$,$1,Dec)}
    | VarDec ASSIGNOP Exp{YY_TREE_ACTION($$,@$,$1,Dec)$1->next=$2;$2->next=$3;}
    ;
Exp : Exp ASSIGNOP Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp AND Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp OR Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp RELOP Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp PLUS Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp MINUS Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp STAR Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | Exp DIV Exp {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | LP Exp RP {YY_TREE_ACTION($$,@$,$1,Exp) $1->next=$2;if($2)$2->next=$3;}
    | MINUS Exp %prec SIG_MINUS {YY_TREE_ACTION($$,@$,$1,Exp) $1->next = $2;}
    | NOT Exp {YY_TREE_ACTION($$,@$,$1,Exp) $1->next = $2;}
    | ID LP Args RP {YY_TREE_ACTION($$,@$,$1,Exp) $1->next = $2;$2->next=$3;if($3)$3->next=$4;}
    | ID LP RP {YY_TREE_ACTION($$,@$,$1,Exp) $1->next=$2;$2->next=$3;}
    | Exp LB Exp RB {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next = $2;$2->next=$3;if($3)$3->next=$4;}
    | Exp DOT ID {YY_TREE_ACTION($$,@$,$1,Exp) if($1)$1->next=$2;$2->next=$3;}
    | ID {YY_TREE_ACTION($$,@$,$1,Exp)}
    | INT {YY_TREE_ACTION($$,@$,$1,Exp)}
    | FLOAT {YY_TREE_ACTION($$,@$,$1,Exp)}
    | error {$$ = NULL;toP = false;}
    ;
Args : Exp COMMA Args {YY_TREE_ACTION($$,@$,$1,Args) if($1)$1->next=$2;$2->next=$3;}
    | Exp {YY_TREE_ACTION($$,@$,$1,Args)}
    ;
%%
yyerror(char* msg) {
    fprintf(stdout, "Error type B at Line %d: %s at \'%s\'\n", yylineno ,msg,yytext);
}

char* enum2s(int val){
    switch(val){
        case INT:return "INT";break;
        case FLOAT:return "FLOAT";break;
        case ID:return "ID";break;
        case SEMI:return "SEMI";break;
        case COMMA:return "COMMA";break;
        case ASSIGNOP:return "ASSIGNOP";break;
        case RELOP:return "RELOP";break;
        case PLUS :return "PLUS";break;
        case MINUS :return "MINUS";break;
        case STAR :return "STAR";break;
        case DIV:return "DIV";break;
        case AND :return "AND";break;
        case OR :return "OR";break;
        case DOT:return "DOT";break;
        case NOT:return "NOT";break;
        case TYPE:return "TYPE";break;
        case LP :return "LP";break;
        case RP :return "RP";break;
        case LB :return "LB";break;
        case RB:return "RB";break;
        case LC :return "LC";break;
        case RC:return "RC";break;
        case STRUCT:return "STRUCT";break;
        case RETURN:return "RETURN";break;
        case IF:return "IF";break;
        case ELSE:return "ELSE";break;
        case WHILE:return "WHILE";break;
        caseToken(Program)
        caseToken(ExtDefList)
        caseToken(ExtDef)
        caseToken(ExtDecList)
        caseToken(Specifier)
        caseToken(StructSpecifier)
        caseToken(OptTag)
        caseToken(Tag)
        caseToken(VarDec)
        caseToken(FunDec)
        caseToken(VarList)
        caseToken(ParamDec)
        caseToken(CompSt)
        caseToken(StmtList)
        caseToken(Stmt)
        caseToken(DefList)
        caseToken(Def)
        caseToken(DecList)
        caseToken(Dec)
        caseToken(Exp)
        caseToken(Args)
    }
}

void PrintTree(struct YYNODE* root,int depth){
    if(root == NULL)
        return;
    for(int i = 0;i<depth;i++)
        printf("  ");
    if(root->TG){
        printf("%s (%d)\n",enum2s(root->YYTYPE),root->first_line);
    }
    else {
        switch(root->YYTYPE){
            case INT:printf("%s: %d\n",enum2s(root->YYTYPE),root->val_int);break;
            case FLOAT:printf("%s: %f\n",enum2s(root->YYTYPE),root->val_float);break;
            case TYPE:printf("%s: %s\n",enum2s(root->YYTYPE),root->val_type);break;
            case ID:printf("%s: %s\n",enum2s(root->YYTYPE),root->val_ID);break;
            default:printf("%s\n",enum2s(root->YYTYPE));break;
        }
    }
    struct YYNODE* son = root->sons;
    while(son!=NULL){
        PrintTree(son,depth+1);
        son=son->next;
    }
}