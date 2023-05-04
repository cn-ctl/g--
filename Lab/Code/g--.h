#ifndef H_GMINUES
#define H_GMINUES
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_INT = 3,                        /* INT  */
  YYSYMBOL_FLOAT = 4,                      /* FLOAT  */
  YYSYMBOL_ID = 5,                         /* ID  */
  YYSYMBOL_SEMI = 6,                       /* SEMI  */
  YYSYMBOL_COMMA = 7,                      /* COMMA  */
  YYSYMBOL_ASSIGNOP = 8,                   /* ASSIGNOP  */
  YYSYMBOL_RELOP = 9,                      /* RELOP  */
  YYSYMBOL_PLUS = 10,                      /* PLUS  */
  YYSYMBOL_MINUS = 11,                     /* MINUS  */
  YYSYMBOL_STAR = 12,                      /* STAR  */
  YYSYMBOL_DIV = 13,                       /* DIV  */
  YYSYMBOL_AND = 14,                       /* AND  */
  YYSYMBOL_OR = 15,                        /* OR  */
  YYSYMBOL_DOT = 16,                       /* DOT  */
  YYSYMBOL_NOT = 17,                       /* NOT  */
  YYSYMBOL_TYPE = 18,                      /* TYPE  */
  YYSYMBOL_LP = 19,                        /* LP  */
  YYSYMBOL_RP = 20,                        /* RP  */
  YYSYMBOL_LB = 21,                        /* LB  */
  YYSYMBOL_RB = 22,                        /* RB  */
  YYSYMBOL_LC = 23,                        /* LC  */
  YYSYMBOL_RC = 24,                        /* RC  */
  YYSYMBOL_STRUCT = 25,                    /* STRUCT  */
  YYSYMBOL_RETURN = 26,                    /* RETURN  */
  YYSYMBOL_IF = 27,                        /* IF  */
  YYSYMBOL_ELSE = 28,                      /* ELSE  */
  YYSYMBOL_WHILE = 29,                     /* WHILE  */
  YYSYMBOL_SIG_MINUS = 30,                 /* SIG_MINUS  */
  YYSYMBOL_YYACCEPT = 31,                  /* $accept  */
  YYSYMBOL_Program = 32,                   /* Program  */
  YYSYMBOL_ExtDefList = 33,                /* ExtDefList  */
  YYSYMBOL_ExtDef = 34,                    /* ExtDef  */
  YYSYMBOL_ExtDecList = 35,                /* ExtDecList  */
  YYSYMBOL_Specifier = 36,                 /* Specifier  */
  YYSYMBOL_StructSpecifier = 37,           /* StructSpecifier  */
  YYSYMBOL_OptTag = 38,                    /* OptTag  */
  YYSYMBOL_Tag = 39,                       /* Tag  */
  YYSYMBOL_VarDec = 40,                    /* VarDec  */
  YYSYMBOL_FunDec = 41,                    /* FunDec  */
  YYSYMBOL_VarList = 42,                   /* VarList  */
  YYSYMBOL_ParamDec = 43,                  /* ParamDec  */
  YYSYMBOL_CompSt = 44,                    /* CompSt  */
  YYSYMBOL_StmtList = 45,                  /* StmtList  */
  YYSYMBOL_Stmt = 46,                      /* Stmt  */
  YYSYMBOL_DefList = 47,                   /* DefList  */
  YYSYMBOL_Def = 48,                       /* Def  */
  YYSYMBOL_DecList = 49,                   /* DecList  */
  YYSYMBOL_Dec = 50,                       /* Dec  */
  YYSYMBOL_Exp = 51,                       /* Exp  */
  YYSYMBOL_Args = 52                       /* Args  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;

//DS4typeCheck
typedef struct Type_* Type;
typedef struct FieldList_* FieldList;
typedef struct ParaList_* ParaList;
typedef struct IDnode_* IDnode;
typedef int bool;
#define true 1
#define false 0
struct Type_
{
    enum { BASIC, ARRAY, STRUCTURE, FUNCTION} kind;
    union{
      int basic;
      struct { Type elem; int size;} array;
      struct {char* name; FieldList structure;} struc;
      struct {Type ret;ParaList para;int argIndex;int paraNum;} func;
    };
    int dec;
    Type belongingStruct;
};

struct FieldList_
{
    char* name;
    Type type;
    FieldList next;
};

struct ParaList_
{
    char* name;
    Type type;
    ParaList next;
};

struct IDnode_
{
  char* name;
  Type type;
  IDnode next;
};
//DS4IR
typedef struct Operand_* Operand;
struct Operand_ {
  enum { VARIABLE, CONSTANT, ADDRESS, REFER } kind;
  union {
    char* var_name;
    int value;
  } u;
};

typedef struct varIDnode_* varIDnode;
struct varIDnode_
{
  char* name;
  char* nickName;
  
  varIDnode next;
};

typedef struct fieldIDnode_* fieldIDnode;
struct fieldIDnode_
{
  char* name;

  int position;//field position
  
  fieldIDnode next;
};

typedef struct arrayIDnode_* arrayIDnode;
struct arrayIDnode_
{
  char* name;

  int elem_size;//elem size of array
  
  arrayIDnode next;
};

struct ArgList
{
  char* name;
  struct ArgList* next;
};

struct InterCode
{
  enum { ASSIGN, ADD, SUB, MUL, DIVV, LABEL, GOTO, RET,
   READ, WRITE, ARG,FUNC, PARAM, DEC,CALL,  IFGOTO  } kind;
  union {
    struct { Operand right, left; } assign;
    struct { Operand result, op1, op2; } binop;
    char* label;
    struct { Operand left,right;char* op,*label;}ifgoto;
  } u;
};

struct InterCodes { struct InterCode code; struct InterCodes *prev, *next; };


//YYNODE
typedef struct YYNODE{
  int YYTYPE;
  int TG;// 标记该node是词法还是语法单元
  int first_line;
  int left;//标记左值还是右值
  union {
      float val_float;
      int val_int;
      char* val_type;
      struct{
        char* val_ID;
        enum{variable = 1,function = 2,num,structrue = 17} kind;
      };
      char* val_relop;
  };
  struct YYNODE* next, * sons;

  Type type;
  Type returnType;
}yynode;


#endif