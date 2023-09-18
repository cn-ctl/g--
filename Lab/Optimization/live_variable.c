#include"g--.h"
struct NodeVar* newNodeVar(struct NodeVar* next,char* var){
    struct NodeVar* temp = (struct NodeVar*)malloc(sizeof(struct NodeVar));
    temp->next = next;
    temp->var = var;
    return temp;
}

bool pushSetVar(struct NodeVar* set,char* target){
    if(target == NULL)
        return false;
    while(set->next!=NULL){
        if(set->var != NULL && strcmp(set->var,target) == 0)
            break;
        set=set->next;
    }
    if(set->var == NULL || strcmp(set->var,target) != 0){
        set->next = newNodeVar(set->next,target);
        return true;
    }
    return false;
}

void popSetVar(struct NodeVar* set,char* target){
    if(target == NULL)
        return;
    struct NodeVar* pre = set;
    set = set->next;
    while(set != NULL){
        if(set->var != NULL && strcmp(set->var,target) == 0){
            pre->next = set->next;
            free(set);
            return;
        }
        pre = set;
        set=set->next;
    }
}

void useOP(struct NodeVar* set_def,struct NodeVar* set_use,Operand op){
    if(op->kind != CONSTANT){
        popSetVar(set_def,op->u.var_name);
        pushSetVar(set_use,op->u.var_name);
    }
}

void defOP(struct NodeVar* set_def,struct NodeVar* set_use,Operand op){
    if(op->kind == VARIABLE){
        popSetVar(set_use,op->u.var_name);
        pushSetVar(set_def,op->u.var_name);
    }else{
        popSetVar(set_def,op->u.var_name);
        pushSetVar(set_use,op->u.var_name);
    }
}

void makeDefUse(){
    def = (struct NodeVar**)malloc(sizeof(struct NodeVar*)*block_num);
    use = (struct NodeVar**)malloc(sizeof(struct NodeVar*)*block_num);
    for(int i = 0;i<block_num;i++){
        def[i] = newNodeVar(NULL,NULL);
        use[i] = newNodeVar(NULL,NULL);
        for(int j = basic_blocks[i].end-1;j>=basic_blocks[i].begin;j--){
            InterCode ir = vIR->vector[j];
            switch (ir->kind)
            {
            case IFGOTO:{
                useOP(def[i],use[i],ir->u.ifgoto.left);
                useOP(def[i],use[i],ir->u.ifgoto.right);
                break;
            }
            case DEC:case CALL:{
                defOP(def[i],use[i],ir->u.assign.left);
                break;
            }
            case PARAM:{
                useOP(def[i],use[i],ir->u.oneOp);
                break;
            }
            case ARG:case READ:{
                defOP(def[i],use[i],ir->u.oneOp);
                break;
            }
            case WRITE:case RET:{
                useOP(def[i],use[i],ir->u.oneOp);
                break;
            }
            case ADD:case SUB:case MUL:case DIV:{
                defOP(def[i],use[i],ir->u.binop.result);
                useOP(def[i],use[i],ir->u.binop.op1);
                useOP(def[i],use[i],ir->u.binop.op2);
                break;
            }
            case ASSIGN:{
                defOP(def[i],use[i],ir->u.assign.left);
                useOP(def[i],use[i],ir->u.assign.right);
                break;
            }
            default:
                break;
            }
        }
    }
}

struct NodeVar* diffSetVar(struct NodeVar* src,struct NodeVar* dest){
    struct NodeVar* ret = newNodeVar(NULL,NULL);
    while(src != NULL){
        if(src->var!=NULL)
            ret->next = newNodeVar(ret->next,src->var);
        src = src->next;
    }
    while(dest!=NULL){
        popSetVar(ret,dest->var);
        dest = dest->next;
    }
    return ret;
}

struct NodeVar* unionSetVar(struct NodeVar* src,struct NodeVar* dest){
    struct NodeVar* ret = newNodeVar(NULL,NULL);
    while(src != NULL){
        if(src->var!=NULL)
            ret->next = newNodeVar(ret->next,src->var);
        src = src->next;
    }
    while(dest!=NULL){
        pushSetVar(ret,dest->var);
        dest = dest->next;
    }
    return ret;
}

void freeSetVar(struct NodeVar* set){
    if(set == NULL)
        return;
    freeSetVar(set->next);
    free(set);
}

bool catSetVar(struct NodeVar* src,struct NodeVar* dest){
    bool ret = false;
    while(src!=NULL){
        ret = ret | pushSetVar(dest,src->var);
        src = src->next;
    }
    return ret;
}

static void propogate(struct Node* worklist,int current){
    struct NodeVar *temp = diffSetVar(out[current],def[current]),*new_in = unionSetVar(use[current],temp);
    freeSetVar(temp);
    if(catSetVar(new_in,in[current])){
        struct Node* p = basic_blocks[current].predecessor;
        while(p!=NULL){
            catSetVar(new_in,out[p->val]);
            pushSet(worklist,p->val);
            p=p->next;
        }
    }
    freeSetVar(new_in);
}

void makeInOut(){
    in = (struct NodeVar**)malloc(sizeof(struct NodeVar*)*block_num);
    out = (struct NodeVar**)malloc(sizeof(struct NodeVar*)*block_num);
    for(int i = 0;i<block_num;i++){
        in[i] = newNodeVar(NULL,NULL);
        out[i] = newNodeVar(NULL,NULL);
    }
}

void liveVariavleAnalysis(){
    makeDefUse();
    struct Node* worklist = newNode(NULL,-1);
    for(int i = 0;i<block_num;i++){
        worklist->next = newNode(worklist->next,i);
    }

    #ifdef DebugLiveVariable
    for(int i = 0;i<block_num;i++){
        printf("block %d:\n",i);
        printf("def: ");
        struct NodeVar* p = def[i];
        while(p != NULL){
            if(p->var != NULL)
                printf("%s ",p->var);
            p = p->next;
        }
        printf("\nuse: ");
        p = use[i];
        while(p != NULL){
            if(p->var != NULL)
                printf("%s ",p->var);
            p = p->next;
        }
        printf("\n");
    }
    #endif 
    makeInOut();
    while(worklist->next!=NULL){
        struct Node* to_free = worklist->next;
        int current = to_free->val;
        worklist->next = to_free->next;
        free(to_free);
        propogate(worklist,current);
    }
    for(int i = 0;i<block_num;i++){
        freeSetVar(def[i]);
        freeSetVar(use[i]);
    }
    free(def);
    free(use);
    #ifdef DebugLiveVariable
    for(int i = 0;i<block_num;i++){
        printf("block %d:\n",i);
        struct NodeVar* p = def[i];
        printf("in: ");
        p = in[i];
        while(p != NULL){
            if(p->var != NULL)
                printf("%s ",p->var);
            p = p->next;
        }
        printf("\nout: ");
        p = out[i];
        while(p != NULL){
            if(p->var != NULL)
                printf("%s ",p->var);
            p = p->next;
        }
        printf("\n");
    }
    #endif 
}