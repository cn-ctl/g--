%locations
%{  
    #include"g--.h"
    #define YYSTYPE void*
    #include "lex.yy.c"
    #include <stdio.h>
    //#define YYDEBUG 1
    #define NEWIRCODE(_type) InterCode temp = (InterCode)malloc(sizeof(struct InterCode_)); \
        temp->kind = _type; 
    void pushVectorIR(vectorIR vector,InterCode t){
        if(vector->count>=vector->size){
            InterCode* newVector = (InterCode*)malloc(sizeof(InterCode)*vector->size*2);
            for(int i = 0;i<vector->size;i++){
            newVector[i] = vector->vector[i];
            }
            free(vector->vector);
            vector->vector = newVector;
            vector->size*=2;
        }
        vector->vector[vector->count]=t;
        vector->count++;
    }    
%}

%token _VARIABLE
%token _CONSTANT
%token _REFER
%token _ADDRESS
%token _ASSIGN
%token _ADD _SUB _MUL _DIV
%token _DEC
%token _ARG
%token _CALL
%token _PARAM
%token _READ
%token _WRITE
%token _GOTO
%token _LABEL
%token RETURN
%token FUNCTION
%token ID
%token RELOP
%token IF
%token COLON
%token SIZE
%token LABELNAME

%%
Program : IRList {}
    ;
IRList : IR {pushVectorIR(vIR,$1);} IRList  
    |   {}
    ;
IR : _LABEL LABELNAME COLON {NEWIRCODE(LABEL);$$ = temp;temp->u.label = $2;}
    | FUNCTION ID COLON {NEWIRCODE(FUNC);$$ = temp;temp->u.label = $2;}
    | OPERAND _ASSIGN OPERAND _ADD OPERAND {NEWIRCODE(ADD);$$ = temp;temp->u.binop.result = $1;temp->u.binop.op1 = $3;temp->u.binop.op2 = $5;}
    | OPERAND _ASSIGN OPERAND _SUB OPERAND {NEWIRCODE(SUB);$$ = temp;temp->u.binop.result = $1;temp->u.binop.op1 = $3;temp->u.binop.op2 = $5;}
    | OPERAND _ASSIGN OPERAND _MUL OPERAND {NEWIRCODE(MUL);$$ = temp;temp->u.binop.result = $1;temp->u.binop.op1 = $3;temp->u.binop.op2 = $5;}
    | OPERAND _ASSIGN OPERAND _DIV OPERAND {NEWIRCODE(DIV);$$ = temp;temp->u.binop.result = $1;temp->u.binop.op1 = $3;temp->u.binop.op2 = $5;}
    | OPERAND _ASSIGN OPERAND {NEWIRCODE(ASSIGN);$$ = temp;temp->u.assign.left = $1;temp->u.assign.right = $3;}
    | _GOTO LABELNAME {NEWIRCODE(GOTO);$$ = temp;temp->u.label = $2;}
    | IF OPERAND RELOP OPERAND _GOTO LABELNAME {NEWIRCODE(IFGOTO);$$ = temp;temp->u.ifgoto.left = $2;temp->u.ifgoto.op = $3;
        temp->u.ifgoto.right = $4;temp->u.ifgoto.label = $6;}
    | RETURN OPERAND {NEWIRCODE(RET);$$ = temp;temp->u.oneOp = $2;}
    | _DEC OPERAND SIZE {NEWIRCODE(DEC);$$ = temp;temp->u.assign.left = $2;temp->u.assign.right = $3;}
    | _ARG OPERAND {NEWIRCODE(ARG);$$ = temp;temp->u.oneOp = $2;}
    | _PARAM OPERAND {NEWIRCODE(PARAM);$$ = temp;temp->u.oneOp = $2;}
    | OPERAND _ASSIGN _CALL ID {NEWIRCODE(CALL);$$ = temp;temp->u.assign.left = $1;temp->u.assign.right = $4;}
    | _READ OPERAND {NEWIRCODE(READ);$$ = temp;temp->u.oneOp = $2;}
    | _WRITE OPERAND {NEWIRCODE(WRITE);$$ = temp;temp->u.oneOp = $2;}
    ;
OPERAND : _VARIABLE {$$ = $1;}
    | _CONSTANT {$$ = $1;}
    | _REFER {$$ = $1;}
    | _ADDRESS {$$ = $1;}
    ;
%%