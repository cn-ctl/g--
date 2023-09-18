#include "g--.h"

static int set_size = 1;

struct Node* newNode(struct Node* next,int val){
    struct Node* temp = (struct Node*)malloc(sizeof(struct Node));
    temp->next = next;
    temp->val = val;
    return temp;
}

void pushSet(struct Node* set,int target){
    while(set->next!=NULL){
        if(set->val==target)
            break;
        set=set->next;
    }
    if(set->val!=target){
        set->next = newNode(NULL,target);
        set_size++;
    }
}

struct Node* makeSet() {
    struct Node* set = (struct Node*)malloc(sizeof(struct Node));
    set->next = NULL;
    set->val = 0;
    InterCode * vector = vIR->vector;
    int vector_size = vIR->count;
    for(int i = 0;i<vector_size;i++){
        if(vector[i]->kind==IFGOTO||vector[i]->kind==GOTO){
            char* label_togo = vector[i]->kind==GOTO?vector[i]->u.label:vector[i]->u.ifgoto.label;
            for(int j = 0;j<vector_size;j++){
                if(vector[j]->kind == LABEL && strcmp(vector[j]->u.label,label_togo)==0){
                    pushSet(set,j);
                    break;
                }
            }
            pushSet(set,i+1);
        }
    }
    return set;
}

int compra(const void* a, const void* b){
    return *(int*)a - *(int *)b;
}

void Set2BBVector(struct Node* set){
    int* vector = (int*)malloc(sizeof(int)*set_size);
    int i = 0;
    #ifdef DebugControllGragh
    printf("setsize:%d\n",set_size);
    #endif
    while(set != NULL){
        struct Node* temp = set;
        vector[i] = set->val;
        #ifdef DebugControllGragh
        printf("%d ",set->val);
        #endif
        set = set->next;
        free(temp);
        i++;
    }
    #ifdef DebugControllGragh
    printf("\n");
    #endif
    block_num = set_size;
    qsort(vector,set_size,sizeof(int),compra);
    basic_blocks = (struct _BasicBlock*)malloc(sizeof(struct _BasicBlock)*set_size);
    for(int i = 0;i<set_size;i++){
        basic_blocks[i].begin = vector[i];
        basic_blocks[i].end = i+1==set_size?vIR->count:vector[i+1];
        basic_blocks[i].predecessor = NULL;
        basic_blocks[i].sucessor = NULL;
    }
}

int findBlock(int target){
    for(int i = 0;i<block_num;i++){
        if(target>=basic_blocks[i].begin&&target<basic_blocks[i].end)
            return i;
    }
}

void makeControllGragh(){
    InterCode * vector = vIR->vector;
    int vector_size = vIR->count;
    for(int i = 0;i<block_num;i++){
        int end = basic_blocks[i].end-1;
        if(vector[end]->kind==IFGOTO){
            char* label_togo = vector[end]->u.ifgoto.label;
            for(int j = 0;j<vector_size;j++){
                if(vector[j]->kind == LABEL && strcmp(vector[j]->u.label,label_togo)==0){
                    int sucessor = findBlock(j);
                    basic_blocks[i].sucessor = newNode(basic_blocks[i].sucessor,sucessor);
                    basic_blocks[sucessor].predecessor = newNode(basic_blocks[sucessor].predecessor,i);
                    break;
                }
            }
            if(i+1<block_num){
                basic_blocks[i].sucessor = newNode(basic_blocks[i].sucessor,i+1);
                basic_blocks[i+1].predecessor = newNode(basic_blocks[i+1].predecessor,i);
            }
        }else if(vector[end]->kind==GOTO){
            char* label_togo = vector[end]->u.label;
            for(int j = 0;j<vector_size;j++){
                if(vector[j]->kind == LABEL && strcmp(vector[j]->u.label,label_togo)==0){
                    int sucessor = findBlock(j);
                    basic_blocks[i].sucessor = newNode(basic_blocks[i].sucessor,sucessor);
                    basic_blocks[sucessor].predecessor = newNode(basic_blocks[sucessor].predecessor,i);
                    break;
                }
            }
        }else if(vector[end]->kind != RET){
            if(i+1<block_num){
                basic_blocks[i].sucessor = newNode(basic_blocks[i].sucessor,i+1);
                basic_blocks[i+1].predecessor = newNode(basic_blocks[i+1].predecessor,i);
            }
        }
    }
}

void makeBB(){
    struct Node* set = makeSet();
    Set2BBVector(set);
    makeControllGragh();
    #ifdef DebugControllGragh
    for(int i = 0;i<block_num;i++){
        printf("block %d begin: %d end: %d\n",i,basic_blocks[i].begin,basic_blocks[i].end);
        struct Node* p = basic_blocks[i].predecessor;
        printf("predecessor:");
        while(p!=NULL){
            printf("%d ",p->val);
            p = p->next;
        }
        p = basic_blocks[i].sucessor;
        printf("\nsucessor:");
        while(p!=NULL){
            printf("%d ",p->val);
            p = p->next;
        }
        printf("\n");
    }
    #endif
}