#include "syntax.tab.h"
#include "g--.h"

#define mips(xx) void xx##2mips(struct InterCodes* ir,FILE* f)
#define mip(xx) xx##2mips
#define leftName ir->code.u.assign.left->u.var_name
#define rightName ir->code.u.assign.right->u.var_name
#define leftKind ir->code.u.assign.left->kind
#define rightKind ir->code.u.assign.right->kind
#define op1 ir->code.u.binop.op1
#define op2 ir->code.u.binop.op2
#define resultName ir->code.u.binop.result->u.var_name

int* vector_var,* vector_temp, current_offset = -4, arg_offset = 8;
bool sync_fp_sp = false;

int GetOff(char* id, FILE* f){
    int i, offset;
    if(id[0]=='t'){
        sscanf(id,"t%d",&i);
        if(vector_temp[i] == 0){
            vector_temp[i] = current_offset;
            current_offset-=4;
            fprintf(f,"  addi $sp, $sp, -4\n");
        }
        offset = vector_temp[i];
    }else{
        sscanf(id,"v%d",&i);
        if(vector_var[i] == 0){
            vector_var[i] = current_offset;
            current_offset-=4;
            fprintf(f,"  addi $sp, $sp, -4\n");
        }
        offset = vector_var[i];
    }
    return offset;
}

void LReg(char* reg,char* id, FILE* f){
    int offset = GetOff(id,f);
    fprintf(f,"  lw %s, %d($fp)\n",reg,offset);
}

void SReg(char* reg,char* id, FILE* f){
    int offset = GetOff(id,f);
    fprintf(f,"  sw %s, %d($fp)\n",reg,offset);
}

mips(assign){
    if(leftName[1]=='0')
        return;
    if(leftKind == ADDRESS){
        LReg("$t0",leftName,f);
        LReg("$t1",rightName,f);
        fprintf(f,"  sw $t1, 0($t0)\n");
    }else{
        int offset;
        switch (rightKind)
        {
        case VARIABLE:
            LReg("$t0",rightName,f);
            break;
        case CONSTANT:
            fprintf(f,"  li $t0, %d\n",ir->code.u.assign.right->u.value);
            break;
        case ADDRESS:
            LReg("$t1",rightName,f);
            fprintf(f,"  lw $t0, 0($t1)\n");
            break;
        case REFER:
            offset = GetOff(rightName,f);
            fprintf(f,"  addi $t0, $fp, %d\n",offset);
            break;
        }
        SReg("$t0",leftName,f);
    }
}

mips(add){
    LReg("$t1",op1->u.var_name,f);
    if(op2->kind == CONSTANT){
        fprintf(f,"  addi $t0, $t1, %d\n",op2->u.value);
    }else{
        LReg("$t2",op2->u.var_name,f);
        fprintf(f,"  add $t0, $t1, $t2\n");
    }
    SReg("$t0",resultName,f);
}

mips(sub){
    if(op1->kind == CONSTANT){
        fprintf(f,"  move $t1, $0\n");
    }else{
        LReg("$t1",op1->u.var_name,f);
    }
    LReg("$t2",op2->u.var_name,f);
    fprintf(f,"  sub $t0, $t1, $t2\n");
    SReg("$t0",resultName,f);
}

mips(mul){
    LReg("$t1",op1->u.var_name,f);
    if(op2->kind == CONSTANT){
        fprintf(f,"  li $t2, %d\n",op2->u.value);
    }else{
        LReg("$t2",op2->u.var_name,f);
    }
    fprintf(f,"  mul $t0, $t1, $t2\n");
    SReg("$t0",resultName,f);
}

mips(div){
    LReg("$t1",op1->u.var_name,f);
    LReg("$t2",op2->u.var_name,f);
    fprintf(f,"  div $t1, $t2\n");
    fprintf(f,"  mflo $t0\n");
    SReg("$t0",resultName,f);
}

mips(label){
    fprintf(f,"%s:\n",ir->code.u.label);
}

mips(goto){
    fprintf(f,"  j %s\n",ir->code.u.label);
}

mips(read){
    fprintf(f,"  addi $sp, $sp, -4\n");
    fprintf(f,"  sw $ra, 0($sp)\n");
    fprintf(f,"  jal read\n");
    fprintf(f,"  lw $ra, 0($sp)\n");
    fprintf(f,"  addi $sp, $sp, 4\n");
    SReg("$v0",ir->code.u.label,f);
}

mips(write){
    LReg("$a0",ir->code.u.label,f);
    fprintf(f,"  addi $sp, $sp, -4\n");
    fprintf(f,"  sw $ra, 0($sp)\n");
    fprintf(f,"  jal write\n");
    fprintf(f,"  lw $ra, 0($sp)\n");
    fprintf(f,"  addi $sp, $sp, 4\n");
}

mips(arg){
    LReg("$t0",ir->code.u.label,f);
    if(!sync_fp_sp){
        fprintf(f,"  add $sp, $fp, %d\n",current_offset);
        sync_fp_sp = true;
    }
    fprintf(f,"  addi $sp, $sp, -4\n  sw $t0, 0($sp)\n");
}

mips(func){
    fprintf(f,"\n%s:\n",ir->code.u.label);
    fprintf(f,"  addi $sp, $sp, -4\n  sw $fp, 0($sp)\n  move $fp, $sp\n");
    current_offset = -4, arg_offset = 8;
}

mips(call){
    fprintf(f,"  addi $sp, $sp, -4\n  sw $ra, 0($sp)\n  jal %s\n  lw $ra, 0($sp)\n  addi $sp, $sp, 4\n  addi $sp, $fp, %d\n",rightName,current_offset+4);
    SReg("$v0",leftName,f);
    sync_fp_sp = false;
}

mips(ret){
    LReg("$v0",ir->code.u.label,f);
    fprintf(f,"  move $sp, $fp\n  lw $fp, 0($sp)\n  addi $sp, $sp, 4\n");
    fprintf(f,"  jr $ra\n");
}

mips(ifgoto){
    char* instr,* t = ir->code.u.ifgoto.op;
    if(t[0] == '=')
        instr = "beq";
    else if(t[0] == '!')
        instr = "bne";
    else if(t[0] == '>'){
        if(t[1] == '=')
            instr = "bge";
        else instr = "bgt";
    }else{
        if(t[1] == '=')
            instr = "ble";
        else instr = "blt";
    }
    LReg("$t0",ir->code.u.ifgoto.left->u.var_name,f);
    if(ir->code.u.ifgoto.right->kind == CONSTANT){
        fprintf(f,"  li $t1, %d\n",ir->code.u.ifgoto.right->u.value);
    }else{
        LReg("$t1",ir->code.u.ifgoto.right->u.var_name,f);
    }
    fprintf(f,"  %s $t0, $t1, %s\n",instr,ir->code.u.ifgoto.label);
}

mips(param){
    char*id = ir->code.u.label;
    int i, offset;
    if(id[0]=='t'){
        sscanf(id,"t%d",&i);
        vector_temp[i] = arg_offset;
    }else{
        sscanf(id,"v%d",&i);
        vector_var[i] = arg_offset;
    }
    arg_offset+=4;
}

mips(dec){
    char* id = ir->code.u.assign.left->u.var_name;
    int i, size = ir->code.u.assign.right->u.value;
    current_offset-=size;
    if(id[0]=='t'){
        sscanf(id,"t%d",&i);
        vector_temp[i] = current_offset+4;
    }else{
        sscanf(id,"v%d",&i);
        vector_var[i] = current_offset+4;
    }
    fprintf(f,"  addi $sp, $sp, %d\n",-size);
}

void (*ir2mips[])(struct InterCodes* ir,FILE* f) = {
    //todo
    /*ASSIGN, ADD, SUB, MUL, DIVV, LABEL, GOTO, RET,
   READ, WRITE, ARG,FUNC, PARAM, DEC,CALL,  IFGOTO*/
    mip(assign),mip(add),mip(sub),mip(mul),mip(div),mip(label),mip(goto),mip(ret),
    mip(read),mip(write),mip(arg),mip(func),mip(param),mip(dec),mip(call),mip(ifgoto)
};

void IR2Mips(struct InterCodes* ir,FILE* f){
    ir2mips[ir->code.kind](ir,f);
}

void MIPSinit(FILE* f,int varCount,int tempCount){
    vector_temp = (int*)malloc(tempCount*sizeof(int));
    vector_var = (int*)malloc(varCount*sizeof(int));
    fprintf(f,".data\n_prompt: .asciiz \"Enter an integer:\"\n_ret: .asciiz \"\\n\"\n.globl main\n.text\n");
    fprintf(f,"read:\n");
    fprintf(f,"  li $v0, 4\n");
    fprintf(f,"  la $a0, _prompt\n");
    fprintf(f,"  syscall\n");
    fprintf(f,"  li $v0, 5\n");
    fprintf(f,"  syscall\n");
    fprintf(f,"  jr $ra\n");
    fprintf(f,"\n");
    fprintf(f,"write:\n");
    fprintf(f,"  li $v0, 1\n");
    fprintf(f,"  syscall\n");
    fprintf(f,"  li $v0, 4\n");
    fprintf(f,"  la $a0, _ret\n");
    fprintf(f,"  syscall\n");
    fprintf(f,"  move $v0, $0\n");
    fprintf(f,"  jr $ra\n");
}