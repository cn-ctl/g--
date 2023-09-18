#include "g--.h"
void op(Operand op,char* s){
    if(op->kind == VARIABLE){
        #ifdef DEBUGIR
            printf("opv\n");
        #endif
        sprintf(s,"%s",op->u.var_name==NULL?"(null)":op->u.var_name);
    }else if(op->kind == CONSTANT){
        #ifdef DEBUGIR
            printf("opc\n");
        #endif
        sprintf(s,"#%d",op->u.value);
    }else if(op->kind == ADDRESS){
        #ifdef DEBUGIR
            printf("opa\n");
        #endif
        sprintf(s,"*%s",op->u.var_name);
    }else{
        #ifdef DEBUGIR
            printf("opr\n");
        #endif
        sprintf(s,"&%s",op->u.var_name);
    }
}

char fours[] = "+-*/";
char fomat[][9] = {"LABEL","GOTO","RETURN","READ","WRITE","ARG"
    ,"FUNCTION", "PARAM"};

void printIR(InterCode ir,FILE* f){
  switch (ir->kind)
  {
  case ASSIGN:{
      #ifdef DEBUGIR
          printf("A\n");
      #endif
      char left[10], right[10];
      op(ir->u.assign.left,left);
      op(ir->u.assign.right,right);
      if(strcmp(left,"(null)")==0)
          return;
      fprintf(f,"%s := %s\n",left,right);
      break;
  }
  case ADD: case SUB: case MUL: case DIV:{
      #ifdef DEBUGIR
          printf("asmd\n");
      #endif
      char res[10], op1[10], op2[10];
      op(ir->u.binop.result,res);
      op(ir->u.binop.op1,op1);
      op(ir->u.binop.op2,op2);
      fprintf(f,"%s := %s %c %s\n",res,op1,fours[ir->kind-1],op2);
      break;
  }
    case GOTO:{
      #ifdef DEBUGIR
          printf("dasdsadsa\n");
      #endif
      fprintf(f,"%s %s\n",fomat[ir->kind-5],ir->u.label);
      break; 
  }
  case RET:case READ:case WRITE:
  case ARG: case PARAM:{
    #ifdef DEBUGIR
          printf("dasdsadsa\n");
      #endif
      char oneop[10];
      op(ir->u.oneOp,oneop);
      fprintf(f,"%s %s\n",fomat[ir->kind-5],oneop);
      break; 
  }
  case FUNC:{
    #ifdef DEBUGIR
          printf("dasdsadsa\n");
      #endif
      char oneop[10];
      op(ir->u.oneOp,oneop);
      fprintf(f,"%s %s :\n",fomat[ir->kind-5],oneop);
      break;     
  }
  case LABEL: {
      #ifdef DEBUGIR
          printf("fl\n");
      #endif
      fprintf(f,"%s %s :\n",fomat[ir->kind-5],ir->u.label);
      break; 
  }
  case DEC:{
      #ifdef DEBUGIR
          printf("d\n");
      #endif
      char left[10];
      op(ir->u.assign.left,left);
      fprintf(f,"DEC %s %d\n",left,ir->u.assign.right->u.value);
      break;
  }
  case CALL:{
      #ifdef DEBUGIR
          printf("c\n");
      #endif
      char left[10], right[10];
      op(ir->u.assign.left,left);
      op(ir->u.assign.right,right);
      fprintf(f,"%s := CALL %s\n",left,right);
      break;
  }
  case IFGOTO:{
      #ifdef DEBUGIR
          printf("ig\n");
      #endif
      char left[10], right[10];
      op(ir->u.ifgoto.left,left);
      op(ir->u.ifgoto.right,right);
      fprintf(f,"IF %s %s %s GOTO %s\n",left,ir->u.ifgoto.op,right,ir->u.ifgoto.label);
      break;
  }
  default:
      break;
  }
}

vectorIR newVectorIR(){
  vectorIR ret = (vectorIR)malloc(sizeof(struct _vectorIR));
  ret->size = 64;
  ret->count = 0;
  ret->vector = (InterCode*)malloc(sizeof(InterCode)*64);
  return ret;
}

int main(int argc, char** argv)
{
    if (argc <= 1) return 1;
    FILE* f = fopen(argv[1], "r")
        ,*fout = fopen(argv[2], "w");;
    if (!f)
    {
        perror(argv[1]);
        return 1;
    }
    vIR = newVectorIR();
    yyrestart(f);
    yyparse();
    #ifdef testIRinput
    for(int i = 0;i<vIR->count;i++){
        printf("%d ",i);
        printIR(vIR->vector[i],stdout);
    }
    #endif
    makeBB();
    #ifdef Debug
        printf("makeBB\n");
    #endif
    liveVariavleAnalysis();
    #ifdef Debug
        printf("liveVariavleAnalysis\n");
    #endif
    #ifdef DebugStdout
    DAG(stdout);
    #else
    DAG(fout);
    #endif
    #ifdef Debug
        printf("DAG\n");
    #endif
    return 0;
}