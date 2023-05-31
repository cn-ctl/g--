#include "syntax.tab.h"
#include "g--.h"

char* reg(char* id){
    //todo
}

#define mips(xx) void xx##2mips(struct InterCodes* ir,FILE* f)
#define leftName ir->code.u.assign.left->u.var_name
#define rightName ir->code.u.assign.right->u.var_name
#define leftKind ir->code.u.assign.left->kind
#define rightKind ir->code.u.assign.right->kind
#define op1 ir->code.u.binop.op1
#define op2 ir->code.u.binop.op2
#define resultName ir->code.u.binop.result->u.var_name

char fours[] = "+-*/";
char fomat[][9] = {"LABEL","GOTO","RETURN","READ","WRITE","ARG"
    ,"FUNCTION", "PARAM"};

void (*ir2mips[])(struct InterCodes* ir,FILE* f) = {
    //todo
}

void LReg(char* reg,char* id){
    //todo
}

void SReg(char* reg,char* id){
    //todo
}

mips(assign){
    if(leftKind == ADDRESS){
        LReg("$t0",leftName);
        LReg("$t1",rightName);
        fprintf(f,"sw $t1, 0($t0)\n")
    }else{
        switch (rightKind)
        {
        case VARIABLE:
            LReg("$t0",rightName);
            break;
        case CONSTANT:
            fprintf(f,"li $t0, %d\n",ir->code.u.assign.right->u.value);
            break;
        case ADDRESS:
            LReg("$t1",rightName);
            fprintf(f,"sw $t0, 0($t1)\n");
            break;
        }
        SReg("$t0",leftName);
    }
}

mips(add){
    LReg("$t1",op1->u.var_name);
    if(op2->kind == CONSTANT){
        fprintf(f,"addi $t0, $t1, %d\n",op2->u.value);
    }else{
        LReg("$t2",op2->u.var_name);
        fprintf(f,"add $t0, $t1, $t2\n");
    }
    SReg("$t0",resultName);
}

mips(sub){
    if(op1->kind == CONSTANT){
        fprintf(f,"move $t1, $0\n");
    }else{
        LReg(f,"$t1",op1->u.var_name);
    }
    LReg("$t2",op2->u.var_name);
    fprintf(f,"sub $t0, $t1, $t2\n");
    SReg("$t0",resultName);
}

mips(mul){
    LReg("$t1",op1->u.var_name);
    if(op2->kind == CONSTANT){
        fprintf(f,"li $t2, %d\n",op2->u.value);
    }else{
        LReg("$t2",op2->u.var_name);
    }
    fprintf(f,"mul $t0, $t1, $t2\n");
    SReg("$t0",resultName);
}

mips(div){
    LReg("$t1",op1->u.var_name);
    LReg("$t2",op2->u.var_name);
    fprintf(f,"div $t1, $t2\n");
    fprintf(f,"mflo $t0\n");
    SReg("$t0",resultName);
}

mips(label){
    fprintf(f,"%s:\n",ir->code.u.label);
}

mips(goto){
    fprintf(f,"j %s\n",ir->code.u.label);
}

mips(call){
    //todo
    fprintf(f,"jal %s\n",rightName);
    SReg("$v0",leftName);
}

mips(ret){
    //todo
    LReg("$v0",ir->->code.u.label);
    fprintf(f,"jr $ra\n");
}

mips(ifgoto){
    
}

void IR2Mips(struct InterCodes* ir,FILE* f){
    ir2mips[ir->code.kind](ir,f);
}