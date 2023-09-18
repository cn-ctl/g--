#include"g--.h"

typedef struct _DAGOp* DAGOp;
struct _DAGOp{
    Operand op;
    int no;
};
struct OpList{
    DAGOp op;
    struct OpList* next;
};
typedef struct _DAGNode* DAGNode;
struct _DAGNode
{
    int kind;
    struct OpList *Bounded;
    union{
        struct{
            DAGOp op1;
            DAGOp op2;
        }Biop;
        DAGOp op;
    }u;
    InterCode copy;
    int ingree;
    bool visited;
};

typedef struct _vectorDAG* vectorDAG;
struct _vectorDAG
{
  DAGNode* vector;
  int size;
  int count;
};

typedef struct _mapNode* mapNode;
struct _mapNode{
    char* key;
    int value;
    mapNode next;
}map = {NULL,-1,NULL};

struct OpList *newOpList(struct OpList *next,DAGOp op){
    struct OpList *ret = (struct OpList *)malloc(sizeof(struct OpList));
    ret->next = next;
    ret->op = op;
    return ret;
}

DAGOp newDAGOp(Operand op,int no){
    DAGOp ret = (DAGOp)malloc(sizeof(struct _DAGOp));
    ret->op = op;
    ret->no = no;
    return ret;
}

DAGNode newDAGNode(int kind){
    DAGNode ret = (DAGNode)malloc(sizeof(struct _DAGNode));
    ret->kind = kind;
    ret->ingree = 0;
    ret->visited = false;
    return ret;
}

vectorDAG newVectorDAG(){
  vectorDAG ret = (vectorDAG)malloc(sizeof(struct _vectorDAG));
  ret->size = 64;
  ret->count = 0;
  ret->vector = (DAGNode*)malloc(sizeof(DAGNode)*64);
  return ret;
}

void pushVectorDAG(vectorDAG vector,DAGNode t){
    if(vector->count>=vector->size){
        DAGNode* newVector = (DAGNode*)malloc(sizeof(DAGNode)*vector->size*2);
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

void putMap(char* key,int value){
    if(map.next == NULL){
        mapNode t = map.next = (mapNode)malloc(sizeof(struct _mapNode));
        t->key = key;
        t->value = value;
        t->next = NULL;
    }else{
        mapNode p = map.next;
        while(p->next != NULL){
            if(strcmp(key,p->key) == 0){
                p->value = value;
                return;
            }
            p = p->next;
        }
        if(strcmp(key,p->key) == 0){
            p->value = value;
        }else{
            mapNode t = p->next = (mapNode)malloc(sizeof(struct _mapNode));
            t->key = key;
            t->value = value;
            t->next = NULL;
        }
    }
}

int inMap(char* key){
    mapNode p = map.next;
    while(p != NULL){
        if(strcmp(key,p->key) == 0){
            return p ->value;
        }
        p = p->next;
    }
    return -1;
}

DAGOp searchOp(vectorDAG vector,Operand op,bool limit_A_R){
    if(op->kind == CONSTANT){
        return newDAGOp(op,-1);
    }
    int var_at = inMap(op->u.var_name);
    if(op->kind==REFER)
        return newDAGOp(op,var_at);
    while(var_at !=-1&&op->kind != CONSTANT && op->kind != REFER){
        DAGNode node = vector->vector[var_at];
        if(node->kind == ASSIGN){
            Operand deeper_op = node->u.op->op;
            if(deeper_op->kind == CONSTANT){
                if(op->kind != VARIABLE)
                    break;
                else return newDAGOp(deeper_op,-1);
            }else if(deeper_op->kind != ADDRESS){
                int deep_op_at = inMap(deeper_op->u.var_name);
                if(node->u.op->no == deep_op_at){
                    if(op->kind == ADDRESS){
                        if(deeper_op->kind == VARIABLE){
                            var_at = deep_op_at;
                            op = newOpA(deeper_op->u.var_name);                            
                        }else break;
                    }else{
                        if(limit_A_R && (deeper_op->kind == ADDRESS||deeper_op->kind == REFER))
                            break;
                        var_at = deep_op_at;
                        op = deeper_op;
                    }
                }else{
                    break;
                }
            }else{
                break;
            }
        }else{
            break;
        }
    }
    if(var_at!=-1){
        vector->vector[var_at]->ingree++;
    }
    return newDAGOp(op,var_at);
}

bool compareOpDAG(DAGOp op1,DAGOp op2){
    if(op1->no!=op2->no || op1->op->kind!=op2->op->kind)
        return false;
    if(op1->no == -1){
        if(op1->op->kind == CONSTANT){
            return op1->op->u.value == op2->op->u.value;
        }
        else{
            if(strcmp(op1->op->u.var_name,op2->op->u.var_name) == 0)
                return true;
            else return false;
        }
    }
    return true;
}

int searchCommonNode(vectorDAG vector,DAGNode node){
    for(int i = 0;i<vector->count;i++){
        if(vector->vector[i]->kind == node->kind){
            if(node->kind == ADD || node->kind == MUL){
                if(compareOpDAG(vector->vector[i]->u.Biop.op1,node->u.Biop.op1)){
                    if(compareOpDAG(vector->vector[i]->u.Biop.op2,node->u.Biop.op2)){
                        return i;
                    }
                }else if(compareOpDAG(vector->vector[i]->u.Biop.op1,node->u.Biop.op2)){
                    if(compareOpDAG(vector->vector[i]->u.Biop.op2,node->u.Biop.op1)){
                        return i;
                    }
                }
            }else{
                if(compareOpDAG(vector->vector[i]->u.Biop.op1,node->u.Biop.op1)){
                    if(compareOpDAG(vector->vector[i]->u.Biop.op2,node->u.Biop.op2)){
                        return i;
                    }
                }
            }
            
        }
    }
    return -1;
}

void ir2DAGNode(vectorDAG vector,InterCode ir){
    DAGNode DAG_node = newDAGNode(ir->kind);
    switch (ir->kind)
    {
    /*ASSIGN;ADD,SUB,MUL,DIV;WRITE,RET,PARAM;READ,CALL;IFGOTO;FUNC,ARG,LABEL,GOTO,DEC;*/
    case ASSIGN:{
        DAG_node->Bounded = newOpList(DAG_node->Bounded,newDAGOp(ir->u.assign.left,-1));
        if(ir->u.assign.left->kind!=ADDRESS)
            putMap(ir->u.assign.left->u.var_name,vector->count);
        else DAG_node->Bounded->op->no=inMap(ir->u.assign.left->u.var_name);
        DAG_node->u.op = searchOp(vector,ir->u.assign.right,false);
        ir->u.assign.right = DAG_node->u.op->op;
        #ifdef DebugMakeDAG
        char temp[10];
        op(DAG_node->u.op->op,temp);
        printf("%s at %d\n",temp,DAG_node->u.op->no);
        #endif
        break;
    }
    case ADD:case SUB:case MUL:case DIV:{
        DAG_node->Bounded = newOpList(DAG_node->Bounded,newDAGOp(ir->u.binop.result,-1));
        DAG_node->u.Biop.op1 = searchOp(vector,ir->u.binop.op1,false);
        DAG_node->u.Biop.op2 = searchOp(vector,ir->u.binop.op2,false);
        ir->u.binop.op1 = DAG_node->u.Biop.op1->op;
        ir->u.binop.op2 = DAG_node->u.Biop.op2->op;
        if(ir->u.binop.result->kind!=ADDRESS)
            putMap(ir->u.binop.result->u.var_name,vector->count);
        else DAG_node->Bounded->op->no=inMap(ir->u.binop.result->u.var_name);
        int same_node_index = searchCommonNode(vector,DAG_node);
        if(same_node_index != -1){
            if(DAG_node->u.Biop.op1->no!=-1){
                vector->vector[DAG_node->u.Biop.op1->no]->ingree--;
            }
            if(DAG_node->u.Biop.op2->no!=-1){
                vector->vector[DAG_node->u.Biop.op2->no]->ingree--;
            }
            DAG_node->kind = ASSIGN;
            free(DAG_node->u.Biop.op1);
            free(DAG_node->u.Biop.op2);
            DAG_node->u.op = newDAGOp(vector->vector[same_node_index]->Bounded->op->op,same_node_index);
            ir->kind=ASSIGN;
            ir->u.assign.left = ir->u.binop.result;
            ir->u.assign.right = DAG_node->u.op->op;
            #ifdef DebugMakeDAG
            printf("direct to common node %d\n",same_node_index);
            char temp[10];
            op(DAG_node->u.op->op,temp);
            printf("%s at %d\n",temp,DAG_node->u.op->no);
            #endif
        }else{
            #ifdef DebugMakeDAG
            char temp1[10],temp2[10];
            op(DAG_node->u.Biop.op1->op,temp1);
            op(DAG_node->u.Biop.op2->op,temp2);
            printf("%s at %d %s at %d\n",temp1,DAG_node->u.Biop.op1->no,temp2,DAG_node->u.Biop.op1->no);
            #endif
        }
        break;
    }
    case RET:case WRITE:case ARG:{
        DAG_node->u.op = searchOp(vector,ir->u.oneOp,true);
        ir->u.oneOp = DAG_node->u.op->op;
        #ifdef DebugMakeDAG
        char temp[10];
        op(DAG_node->u.op->op,temp);
        printf("%s at %d\n",temp,DAG_node->u.op->no);
        #endif
        break;
    }
    case READ:case CALL:{
        Operand op = ir->kind == CALL?ir->u.assign.left:ir->u.oneOp;
        DAG_node->Bounded = newOpList(DAG_node->Bounded,newDAGOp(op,-1));
        if(op->kind!=ADDRESS)
            putMap(op->u.var_name,vector->count);
        #ifdef DebugMakeDAG
        printf("read or call\n");
        #endif
        break;
    }
    case IFGOTO:{
        DAG_node->u.Biop.op1 = searchOp(vector,ir->u.ifgoto.left,true);
        DAG_node->u.Biop.op2 = searchOp(vector,ir->u.ifgoto.right,true);
        ir->u.ifgoto.left = DAG_node->u.Biop.op1->op;
        ir->u.ifgoto.right = DAG_node->u.Biop.op2->op;
        #ifdef DebugMakeDAG
        char temp1[10],temp2[10];
        op(DAG_node->u.Biop.op1->op,temp1);
        op(DAG_node->u.Biop.op2->op,temp2);
        printf("%s at %d %s at %d\n",temp1,DAG_node->u.Biop.op1->no,temp2,DAG_node->u.Biop.op1->no);
        #endif
        break;
    }
    default:{
        #ifdef DebugMakeDAG
        printf("copy\n");
        #endif
        break;
    }
    }
    DAG_node->copy = ir;
    pushVectorDAG(vector,DAG_node);
}

void freeMap(){
    while(map.next!=NULL){
        mapNode p = map.next;
        map.next=p->next;
        free(p);
    }
};

void freeVectorDAG(vectorDAG vector){
    for(int i = 0;i<vector->count;i++){
        free(vector->vector[i]);
    }
    free(vector);
}

void dfsDAG(vectorDAG vector,int index){
    if(index == -1)
        return;
    DAGNode t = vector->vector[index];
    if(t->visited)
        return;
    t->visited = true;
    switch (t->kind)
    /*ASSIGN;ADD,SUB,MUL,DIV;WRITE,RET,PARAM;IFGOTO;READ,CALL;FUNC,ARG,LABEL,GOTO,DEC;*/
    {
    case ARG:case WRITE:case RET:case ASSIGN:{
        if(t->Bounded!=NULL&&t->Bounded->op->op->kind == ADDRESS)
            dfsDAG(vector,t->Bounded->op->no);
        dfsDAG(vector,t->u.op->no);
        break;
    }
    case IFGOTO:case ADD:case SUB:case MUL:case DIV:{
        if(t->Bounded!=NULL&&t->Bounded->op->op->kind == ADDRESS)
            dfsDAG(vector,t->Bounded->op->no);
        dfsDAG(vector,t->u.Biop.op1->no);
        dfsDAG(vector,t->u.Biop.op2->no);
        break;
    }
    default:
        break;
    }
}

bool inVarList(struct NodeVar* List,char* var){
    while(List!=NULL){
        if(List->var!=NULL&&strcmp(List->var,var) == 0)
            return true;
        List = List->next;
    }
    return false;
}

void deadCode(vectorDAG vector,struct NodeVar* use){
    int end = vector->count;
    for(int i = end-1;i>=0;i--){
        DAGNode t = vector->vector[i];
        if(t->visited)
            continue;
        t->visited = true;
        switch (t->kind)
        /*ASSIGN;ADD,SUB,MUL,DIV;WRITE,RET,PARAM;IFGOTO;READ,CALL;FUNC,ARG,LABEL,GOTO,DEC;*/
        {
        case ARG:case WRITE:case RET:{
            #ifdef DebugMakeDAG
            printf("must stay\n");
            #endif
            dfsDAG(vector,t->u.op->no);
            break;
        }
        case IFGOTO:{
            dfsDAG(vector,t->u.Biop.op1->no);
            dfsDAG(vector,t->u.Biop.op2->no);
            break;
        }
        case ASSIGN:{
            if(t->Bounded->op->op->kind == ADDRESS || inVarList(use,t->Bounded->op->op->u.var_name)){
                if(t->Bounded->op->op->kind == ADDRESS)
                    dfsDAG(vector,t->Bounded->op->no);
                dfsDAG(vector,t->u.op->no);
            }else{
                t->kind = -1;
            }
            break;
        }
        case ADD:case SUB:case MUL:case DIV:{
            if(t->Bounded->op->op->kind == ADDRESS || inVarList(use,t->Bounded->op->op->u.var_name)){
                if(t->Bounded->op->op->kind == ADDRESS)
                    dfsDAG(vector,t->Bounded->op->no);
                dfsDAG(vector,t->u.Biop.op1->no);
                dfsDAG(vector,t->u.Biop.op2->no);
            }else{
                t->kind = -1;
            }
            break;
        }
        default:
            break;
        }
    }
}

void translateDAG(vectorDAG vector,FILE* f){
    for(int i = 0;i<vector->count;i++){
        DAGNode t = vector->vector[i];
        if(t->kind!=-1){
            printIR(t->copy,f);
        }
    }
}

void DAG(FILE* f){
    for(int i = 0;i<block_num;i++){
        vectorDAG vector = newVectorDAG();
        #ifdef DebugMakeDAG
        printf("making BB %d\n",i);
        #endif
        for(int j = basic_blocks[i].begin;j<basic_blocks[i].end;j++){
            #ifdef DebugMakeDAG
            printIR(vIR->vector[j],stdout);
            #endif
            ir2DAGNode(vector,vIR->vector[j]);
        }
        deadCode(vector,out[i]);
        translateDAG(vector,f);
        freeVectorDAG(vector);
        freeMap();
    }
}