#ifndef H_GMINUES
#define H_GMINUES
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

//#define DebugTotal

#ifdef DebugTotal
#define DebugMakeDAG

#define DebugLiveVariable

#define DebugControllGragh

#define testIRinput
#define DebugStdout
#define Debug
#endif
typedef int bool;
#define true 1
#define false 0

typedef struct Operand_* Operand;
struct Operand_ {
  enum { VARIABLE, CONSTANT, ADDRESS, REFER } kind;
  union {
    char* var_name;
    int value;
  } u;
};

typedef struct InterCode_* InterCode;
struct InterCode_
{
  enum { ASSIGN, ADD, SUB, MUL, DIV, LABEL, GOTO, RET,
   READ, WRITE, ARG,FUNC, PARAM, DEC,CALL,  IFGOTO  } kind;
  union {
    struct { Operand right, left; } assign;
    struct { Operand result, op1, op2; } binop;
    char* label;
    Operand oneOp;
    struct { Operand left,right;char* op,*label;}ifgoto;
  } u;
};

typedef struct _vectorIR* vectorIR;
struct _vectorIR
{
  InterCode* vector;
  int size;
  int count;
}*vIR;

struct Node{
  int val;
  struct Node* next;
};
struct Node* newNode(struct Node* next,int val);
void pushSet(struct Node* set,int target);
struct _BasicBlock
{
  int begin, end;
  struct Node* sucessor,* predecessor;
}*basic_blocks;
int block_num;

struct NodeVar{
  char* var;
  struct NodeVar* next;
}**def,**use,**in,**out;

void op(Operand op,char* s);
void printIR(InterCode ir,FILE* f);
Operand newOpA(char* var_name);
#endif