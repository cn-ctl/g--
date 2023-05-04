#include "syntax.tab.h"
#include "g--.h"

#define new(obj) (struct obj *)malloc(sizeof(struct obj))
#define newOp (Operand)malloc(sizeof(struct Operand_))
//#define DEBUGIR
bool tranlateError = false;

varIDnode varIDlist[0x3fff];
fieldIDnode fieldIDlist[0x3fff];
arrayIDnode arrayIDlist[0x3fff];//array+struct name
int varCount = 1, tempCount = 1, labelCount = 1, funcCount = 1;

static unsigned int hash(char* name){
    unsigned int val = 0, i;
    for(;*name;++name){
        val = (val<<2)+*name;
        if(i=val&~0x3fff) val = (val ^ (i>>12)) & 0x3fff;
    }
    return val;
}

char* lookUp(char* src){
    if(strcmp(src,"read")==0)
        return "read";
    if(strcmp(src,"write")==0)
        return "write";
    int index = hash(src);
    varIDnode t = varIDlist[index];
    char* ret;
    while(t!=NULL){
        if(strcmp(t->name,src)==0){
            ret = t->nickName;
            break;
        }
        t=t->next;
    }
    if(t == NULL){
        char* newName = ret = (char*)malloc(sizeof(char)*10);
        sprintf(newName,"v%d",varCount++);
        varIDnode newNode  = (varIDnode)malloc(sizeof(struct varIDnode_));
        newNode->name = src;
        newNode->nickName = newName;
        newNode->next = varIDlist[index];
        varIDlist[index] = newNode;
    }
    return ret;
}

char* addFunc(char* src){
    int index = hash(src);
    varIDnode t = varIDlist[index];
    char* ret;
    char* newName = ret = (char*)malloc(sizeof(char)*10);
    sprintf(newName,"f%d",funcCount++);
    varIDnode newNode  = (varIDnode)malloc(sizeof(struct varIDnode_));
    newNode->name = src;
    newNode->nickName = newName;
    newNode->next = varIDlist[index];
    varIDlist[index] = newNode;
    return ret;
}

void addField(char* src,int offset){
    int index = hash(src);
    fieldIDnode t = fieldIDlist[index];
    fieldIDnode newNode  = (fieldIDnode)malloc(sizeof(struct fieldIDnode_));
    newNode->name = src;
    newNode->position = offset;
    newNode->next = fieldIDlist[index];
    fieldIDlist[index] = newNode;
}

void addArray(char* src,int elemsize){
    int index = hash(src);
    arrayIDnode t = arrayIDlist[index];
    arrayIDnode newNode  = (arrayIDnode)malloc(sizeof(struct arrayIDnode_));
    newNode->name = src;
    newNode->elem_size = elemsize;
    newNode->next = arrayIDlist[index];
    arrayIDlist[index] = newNode;
}

int lookUpArray(char* src){
    int index = hash(src);
    arrayIDnode t = arrayIDlist[index];
    int ret = 4;
    while(t!=NULL){
        if(strcmp(t->name,src)==0){
            ret = t->elem_size;
            break;
        }
        t=t->next;
    }
    return ret;
}

int lookUpField(char* src){
    int index = hash(src);
    fieldIDnode t = fieldIDlist[index];
    int ret = 4;
    while(t!=NULL){
        if(strcmp(t->name,src)==0){
            ret = t->position;
            break;
        }
        t=t->next;
    }
    return ret;
}

Operand newOpV(char* var_name){
    Operand ret = newOp;
    ret->kind = VARIABLE;
    ret->u.var_name = var_name;
    return ret;
}

Operand newOpC(int value){
    Operand ret = newOp;
    ret->kind = CONSTANT;
    ret->u.value = value;
    return ret;
}

Operand newOpA(char* var_name){
    Operand ret = newOp;
    ret->kind = ADDRESS;
    ret->u.var_name = var_name;
    return ret;
}

Operand newOpR(char* var_name){
    Operand ret = newOp;
    ret->kind = REFER;
    ret->u.var_name = var_name;
    return ret;
}

char* newTemp(){
    char* newName = (char*)malloc(sizeof(char)*10);
    sprintf(newName,"t%d",tempCount++);
    return newName;
}

char* newLabel(){
    char* newName = (char*)malloc(sizeof(char)*10);
    sprintf(newName,"l%d",labelCount++);
    return newName;
}

struct InterCodes* newNode(){
    struct InterCodes* ret = (struct InterCodes*)malloc(sizeof(struct InterCodes));
    ret->next = ret;
    ret->prev = ret;
    return ret;
}

struct InterCodes* newNodeAVC(char* v,int c){
    struct InterCodes* ret = newNode();
    ret->code.kind = ASSIGN;
    ret->code.u.assign.left = newOpV(v);
    ret->code.u.assign.right = newOpC(c);
    return ret;
}

struct InterCodes* newNodeAVV(char* v1,char* v2){
    struct InterCodes* ret = newNode();
    ret->code.kind = ASSIGN;
    ret->code.u.assign.left = newOpV(v1);
    ret->code.u.assign.right = newOpV(v2);
    return ret;
}

struct InterCodes* newNodeAVA(char* v1,char* v2){
    struct InterCodes* ret = newNode();
    ret->code.kind = ASSIGN;
    ret->code.u.assign.left = newOpV(v1);
    ret->code.u.assign.right = newOpA(v2);
    return ret;
}

struct InterCodes* newNodeAVR(char* v1,char* v2){
    struct InterCodes* ret = newNode();
    ret->code.kind = ASSIGN;
    ret->code.u.assign.left = newOpV(v1);
    ret->code.u.assign.right = newOpR(v2);
    return ret;
}

struct InterCodes* newNodeAAV(char* v1,char* v2){
    struct InterCodes* ret = newNode();
    ret->code.kind = ASSIGN;
    ret->code.u.assign.left = newOpA(v1);
    ret->code.u.assign.right = newOpV(v2);
    return ret;
}

struct InterCodes* newNodeBiVC(int kind,char* res,char* v,int c){
    struct InterCodes* ret = newNode();
    ret->code.kind = kind;
    ret->code.u.binop.result = newOpV(res);
    ret->code.u.binop.op1 = newOpV(v);
    ret->code.u.binop.op2 = newOpC(c);
    return ret;
}

struct InterCodes* newNodeBiCV(int kind,char* res,int c,char* v){
    struct InterCodes* ret = newNode();
    ret->code.kind = kind;
    ret->code.u.binop.result = newOpV(res);
    ret->code.u.binop.op1 = newOpC(c);
    ret->code.u.binop.op2 = newOpV(v);
    return ret;
}

struct InterCodes* newNodeBiVV(int kind,char* res,char* v1,char* v2){
    struct InterCodes* ret = newNode();
    ret->code.kind = kind;
    ret->code.u.binop.result = newOpV(res);
    ret->code.u.binop.op1 = newOpV(v1);
    ret->code.u.binop.op2 = newOpV(v2);
    return ret;
}

struct InterCodes* newNodeDec(char* v,int c){
    struct InterCodes* ret = newNode();
    ret->code.kind = DEC;
    ret->code.u.assign.left = newOpV(v);
    ret->code.u.assign.right = newOpC(c);
    return ret;
}

struct InterCodes* newNodeL(char* label){
    struct InterCodes* ret = newNode();
    ret->code.kind = LABEL;
    ret->code.u.label = label;
    return ret;
}

struct InterCodes* newNodeF(char* fun){
    struct InterCodes* ret = newNode();
    ret->code.kind = FUNC;
    ret->code.u.label = fun;
    return ret;
}

struct InterCodes* newNodeGo(char* label){
    struct InterCodes* ret = newNode();
    ret->code.kind = GOTO;
    ret->code.u.label = label;
    return ret;
}

struct InterCodes* newNodeIfGoVV(char* op,char* left,char* right,char* label){
    struct InterCodes* ret = newNode();
    ret->code.kind = IFGOTO;
    ret->code.u.ifgoto.label = label;
    ret->code.u.ifgoto.op = op;
    ret->code.u.ifgoto.left = newOpV(left);
    ret->code.u.ifgoto.right = newOpV(right);
    return ret;
}

struct InterCodes* newNodeIfGoVC(char* op,char* left,int right,char* label){
    struct InterCodes* ret = newNode();
    ret->code.kind = IFGOTO;
    ret->code.u.ifgoto.label = label;
    ret->code.u.ifgoto.op = op;
    ret->code.u.ifgoto.left = newOpV(left);
    ret->code.u.ifgoto.right = newOpC(right);
    return ret;
}

struct InterCodes* newNodeRe(char* val){
    struct InterCodes* ret = newNode();
    ret->code.kind = RET;
    ret->code.u.label = val;
    return ret;
}

struct InterCodes* newNodeRead(char* val){
    struct InterCodes* ret = newNode();
    ret->code.kind = READ;
    ret->code.u.label = val;
    return ret;
}

struct InterCodes* newNodeWrite(char* val){
    struct InterCodes* ret = newNode();
    ret->code.kind = WRITE;
    ret->code.u.label = val;
    return ret;
}

struct InterCodes* newNodeCall(char* place,char* funcName){
    struct InterCodes* ret = newNode();
    ret->code.kind = CALL;
    ret->code.u.assign.left = newOpV(place);
    ret->code.u.assign.right = newOpV(funcName);
    return ret;
}

struct InterCodes* newNodeArg(char* val){
    struct InterCodes* ret = newNode();
    ret->code.kind = ARG;
    ret->code.u.label = val;
    return ret;
}

struct InterCodes* newNodeParam(char* val){
    struct InterCodes* ret = newNode();
    ret->code.kind = PARAM;
    ret->code.u.label = val;
    return ret;
}

struct InterCodes* catNode(struct InterCodes* src, struct InterCodes* dst){
    if(src == NULL)
        return dst;
    if(dst == NULL)
        return src;
    struct InterCodes* srcEnd = src->prev, *dstEnd = dst->prev;
    src->prev = dstEnd;
    srcEnd->next = dst;
    dst->prev = srcEnd;
    dstEnd->next = src;
    return src;
}

struct ArgList* newArgList(char* name,struct ArgList* next){
    struct ArgList* ret = new(ArgList);
    ret->name = name;
    ret->next = next;
    return ret;
}

int sizeElem(struct YYNODE* Exp){
    struct YYNODE* son = Exp->sons;
    int ret = 4;
    if(son->next){
        ret = lookUpArray(son->next->next->val_ID);
    }else{
        ret = lookUpArray(son->val_ID);
    }
    return ret;
}

struct InterCodes* translate_StmtList(struct YYNODE* StmtList);
struct InterCodes* translate_Exp(struct YYNODE* Exp,char* place);
struct InterCodes* translate_Stmt(struct YYNODE* Stmt);
struct InterCodes* translate_CompSt(struct YYNODE* CompSt);
struct InterCodes* translate_Def(struct YYNODE* Def);
struct InterCodes* translate_Dec(struct YYNODE* Dec,int size);
struct InterCodes* translate_Default(struct YYNODE* Default);
struct InterCodes* translate_ExtDef(struct YYNODE* Def);
struct InterCodes* translate_Fun(struct YYNODE* Fun);
struct InterCodes* translate_Param(struct YYNODE* Param);
int translate_Specifier(struct YYNODE* Spe);
int translate_StrDef(struct YYNODE* StrD,int offset);
int translate_StrDec(struct YYNODE* StrDec,int offset,int size);
struct InterCodes* translate_Cond(struct YYNODE* Cond,char* labeltrue,char* labelfalse);
struct InterCodes* translate_Args(struct YYNODE* Args,struct ArgList* arg_list);

struct InterCodes* lookUpExp(struct YYNODE* Exp,char* place){
    #ifdef DEBUGIR
        printf("lookUpExp\n");
    #endif
    //translate the left value Exp
    //only 3 posibal situation
    struct InterCodes* ret;
    struct YYNODE* son = Exp->sons;
    if(son->next == NULL){
        char* varname = lookUp(son->val_ID);
        struct InterCodes* code1 = ret = newNodeAVV(place,varname);
    }else{
        char* t1 = newTemp();
        struct InterCodes* code1 = lookUpExp(son,t1);
        if(son->next->YYTYPE == DOT){
            son = son->next->next;
            int offset = lookUpField(son->val_ID);
            struct InterCodes* code2 = newNodeBiVC(ADD,place,t1,offset);
            ret = catNode(code1,code2);
        }else{
            int elemsize = sizeElem(son);
            son = son->next->next;
            char* t2 = newTemp(),*t3 = newTemp();
            struct InterCodes* code2 = translate_Exp(son,t2)
                ,*code3 = newNodeBiVC(MUL,t3,t2,elemsize)
                ,*code4 = newNodeBiVV(ADD,place,t1,t3);
            ret = catNode(code1,catNode(code2,catNode(code3,code4)));
        }
    }
    return ret;
}

struct InterCodes* translate_CompSt(struct YYNODE* CompSt){
    #ifdef DEBUGIR
        printf("CompSt\n");
    #endif
    struct InterCodes* ret = NULL;
    struct YYNODE* son = CompSt->sons->next;
    struct InterCodes* code1 = NULL,*code2 = NULL;
    if(son->YYTYPE == YYSYMBOL_DefList){
        code1 =translate_Def(son);
        code2 = translate_StmtList(son->next);
    }else if(son->YYTYPE == YYSYMBOL_StmtList){
        code2 = translate_StmtList(son);
    }
    ret = catNode(code1,code2);
    return ret;
}

struct InterCodes* translate_StmtList(struct YYNODE* StmtList){
    #ifdef DEBUGIR
        printf("StmtList\n");
    #endif
    if(StmtList == NULL)
        return NULL;
    struct YYNODE* son = StmtList->sons;
    struct InterCodes* code1 = translate_Stmt(son),
        *code2 = translate_StmtList(son->next),
        *ret = catNode(code1,code2);
    return ret;
}

struct InterCodes* translate_Def(struct YYNODE* Def){
    #ifdef DEBUGIR
        printf("Def\n");
    #endif
    if(Def == NULL)
        return NULL;
    struct YYNODE* son = Def->sons;
    struct InterCodes* ret = NULL;
    son = son->sons;
    int size = translate_Specifier(son);
    son = son->next;
    struct InterCodes* code1 = translate_Dec(son,size)
        ,*code2 = translate_Def(Def->sons->next);
    ret = catNode(code1, code2);
    
    return ret;
}

struct InterCodes* translate_Dec(struct YYNODE* Dec,int size){
    #ifdef DEBUGIR
        printf("Dec\n");
    #endif
    struct YYNODE* son = Dec->sons->sons;//VarDec
    struct InterCodes* ret = NULL;
    char* id;
    if(son->sons->YYTYPE == ID){//singlevar
        id = lookUp(son->sons->val_ID);
        if(size!=4){
            char* t1 = newTemp();
            struct InterCodes* code1 = newNodeDec(t1,size)
                ,*code2 = newNodeAVR(id,t1);
            ret = newNodeDec(id,size);
        }
    }else{//array
        id = lookUp(son->sons->sons->val_ID);
        int num = son->sons->next->next->val_int;
        char* t1 = newTemp();
        struct InterCodes* code1 = newNodeDec(t1,size*num)
            ,*code2 = newNodeAVR(id,t1);
        addArray(son->sons->sons->val_ID,size);
        ret = catNode(code1,code2);
    }
    if(son->next){
        char* t1 = newTemp();
        struct InterCodes* code1 = translate_Exp(son->next->next,t1)
            ,*code2 = newNodeAVV(id,t1);
        ret = catNode(ret,catNode(code1,code2));
    }
    if(Dec->sons->next){
        ret = catNode(ret,translate_Dec(Dec->sons->next->next,size));
    }
    return ret;
}

struct InterCodes* translate_Default(struct YYNODE* Default){
    #ifdef DEBUGIR
        printf("IR\n");
    #endif
    struct InterCodes* ret = NULL;
    struct YYNODE* son = Default->sons;
    while(son!=NULL){
        switch (son->YYTYPE)
        {
        case YYSYMBOL_ExtDef:{
            ret = catNode(ret,translate_ExtDef(son));
            break;
        }
        default:{
            ret = catNode(ret,translate_Default(son));
            break;
        }
        }
        son = son->next;
    }
    return ret;
}

struct InterCodes* translate_ExtDef(struct YYNODE* Def){
    #ifdef DEBUGIR
        printf("Def\n");
    #endif
    struct YYNODE* son = Def->sons;
    struct InterCodes* ret = NULL;
    int size = translate_Specifier(son);
    son = son->next;
    if(son->YYTYPE == YYSYMBOL_FunDec){
        if(size != 4){
            tranlateError = true;
        }
        struct InterCodes* code1 = translate_Fun(son)
            ,*code2 = translate_CompSt(son=son->next);
        ret = catNode(code1,code2);
    }else if(son->YYTYPE == YYSYMBOL_ExtDecList){
        tranlateError = true;
    }
    return ret;
}

struct InterCodes* translate_Fun(struct YYNODE* Fun){
    #ifdef DEBUGIR
        printf("Fun\n");
    #endif
    struct YYNODE* son = Fun->sons;
    char* id = son->val_ID;
    if(strcmp(id,"main") != 0){
        id = addFunc(id);
    }
    struct InterCodes* ret = newNodeF(id);
    son = son->next->next;
    if(son->YYTYPE != RP){
        ret = catNode(ret, translate_Param(son));
    }
    return ret;
}

struct InterCodes* translate_Param(struct YYNODE* Param){
    #ifdef DEBUGIR
        printf("Param\n");
    #endif
    struct YYNODE* son = Param->sons,*next = son->next;//ParamDec;
    struct InterCodes* ret;
    son = son->sons->next;//VarDec
    if(son->sons->YYTYPE!=ID){
        tranlateError = true;
    }else{
        char* id = lookUp(son->sons->val_ID);
        ret = newNodeParam(id);
        lookUp(id);
    }
    if(next){
        ret = catNode(ret,translate_Param(next->next));
    }
    return ret;
}

int translate_Specifier(struct YYNODE* Spe){
    #ifdef DEBUGIR
        printf("Specifier\n");
    #endif
    struct YYNODE* son = Spe->sons;
    int ret = 4;
    if(son->YYTYPE != TYPE){
        Spe = son;
        son = son->sons->next;
        if(son->YYTYPE == YYSYMBOL_Tag){
            ret = lookUpArray(son->sons->val_ID);
        }else{
            char* str_name = NULL;
            if(son->sons){
                str_name = son->sons->val_ID;
            }
            son = son->next->next;
            int size = ret = translate_StrDef(son,0);
            if(str_name){
                addArray(str_name,size);
            }
        }
    }
    return ret;
}

int translate_StrDef(struct YYNODE* StrD,int offset){
    #ifdef DEBUGIR
        printf("StrDef\n");
    #endif
    int ret = offset;
    if(StrD == NULL)
        return ret;
    struct YYNODE* son = StrD->sons;
    if (StrD->YYTYPE == YYSYMBOL_Def){
        int size = translate_Specifier(son);
        son = son->next;
        ret = translate_StrDec(son,offset,size);
    }
    else{
        ret = translate_StrDef(son,offset);
        ret = translate_StrDef(son->next,ret);
    }
    return ret;
}

int translate_StrDec(struct YYNODE* StrDec,int offset,int size){ 
    #ifdef DEBUGIR
        printf("StrDec\n");
    #endif
    int ret = offset;
    struct YYNODE* son = StrDec->sons,*next = son->next;//Dec
    son = son->sons;//VarDec
    if(son->next){
        tranlateError = true;
    }
    son = son->sons;
    if(son->YYTYPE == ID){//singlevar
        addField(son->val_ID,offset);
        ret += size;
    }else{//array
        int num = son->next->next->val_int;
        addField(son->sons->val_ID,offset);
        addArray(son->sons->val_ID,size);
        ret += size*num;
    }
    if(next){
        ret = translate_StrDec(next->next,ret,size);
    }
    return ret;
}

struct InterCodes* translate_Cond(struct YYNODE* Cond,char* labeltrue,char* labelfalse){    
    #ifdef DEBUGIR
        printf("Cond\n");
    #endif
    struct InterCodes* ret;
    struct YYNODE* son = Cond->sons;
    if(son->YYTYPE == NOT){
        ret = translate_Cond(son->next,labelfalse,labeltrue);
    }else{
        switch (son->next->YYTYPE)
        {
        case RELOP:{
            char* t1 = newTemp(),*t2 = newTemp();
            struct InterCodes* code1 = translate_Exp(son,t1)
                ,*code2 = translate_Exp(son->next->next,t2)
                ,*code3 = newNodeIfGoVV(son->next->val_relop,t1,t2,labeltrue);
            ret = catNode(code1,catNode(code2,catNode(code3,newNodeGo(labelfalse))));
            break;
        }
        case AND:{
            char* label1 = newLabel();
            struct InterCodes* code1 = translate_Cond(son,label1,labelfalse)
                ,*code2 = translate_Cond(son->next->next,labeltrue,labelfalse);
            ret = catNode(code1,catNode(newNodeL(label1),code2));
            break;
        }
        case OR:{
            char* label1 = newLabel();
            struct InterCodes* code1 = translate_Cond(son,labeltrue,label1)
                ,*code2 = translate_Cond(son->next->next,labeltrue,labelfalse);
            ret = catNode(code1,catNode(newNodeL(label1),code2));
            break;
        }
        default:{
            char* t1 = newTemp();
            struct InterCodes* code1 = translate_Exp(Cond,t1)
                ,*code2 = newNodeIfGoVC("!=",t1,0,labeltrue);
            ret = catNode(code1,catNode(code2,newNodeGo(labelfalse)));
            break;
        }
        }
    }
    return ret;
}

struct InterCodes* translate_Args(struct YYNODE* Args,struct ArgList* arg_list){
    #ifdef DEBUGIR
        printf("Args\n");
    #endif
    struct InterCodes* ret;
    struct YYNODE* son = Args->sons;
    char* t1 = newTemp();
    struct InterCodes* code1 = translate_Exp(son,t1);
    arg_list->next = newArgList(t1,arg_list->next);
    if(son->next){
        struct InterCodes* code2 = translate_Args(son->next->next,arg_list);
        ret = catNode(code1,code2);
    }else{
        ret = code1;
    }
    return ret;
}

struct InterCodes* translate_Exp(struct YYNODE* Exp,char* place){
    #ifdef DEBUGIR
        printf("Exp\n");
    #endif
    struct InterCodes* ret;
    struct YYNODE* son = Exp->sons;
    switch (son->YYTYPE)
    {
    case INT:{
        struct InterCodes* code = ret = newNodeAVC(place,son->val_int);
        break;
        }
    case ID:{
        if(son->next==NULL){
            char* var_name = lookUp(son->val_ID);
            struct InterCodes* code = ret = newNodeAVV(place,var_name);
        }else{
            switch (son->next->YYTYPE)
            {
            case LP:{
                char* func_name = lookUp(son->val_ID);
                son=son->next->next;
                if(son->next==NULL){
                    if(strcmp(func_name,"read") == 0){
                        ret = newNodeRead(place);
                    }else ret = newNodeCall(place,func_name);
                }else{
                    struct ArgList* arg_list = newArgList(NULL,NULL);
                    struct InterCodes* code1 = translate_Args(son,arg_list);
                    if(strcmp(func_name,"write")==0){
                        ret = catNode(code1,catNode(newNodeWrite(arg_list->next->name),newNodeAVC(place,0)));
                    }else{
                        struct InterCodes* code2=NULL;
                        struct ArgList* bef = arg_list;
                        arg_list = arg_list->next;
                        free(bef);
                        while(arg_list){
                            if(code2){
                                catNode(code2,newNodeArg(arg_list->name));
                            }else {
                                code2 = newNodeArg(arg_list->name);
                            }
                            bef = arg_list;
                            arg_list = arg_list->next;
                            free(bef);
                        }
                        ret = catNode(code1,catNode(code2,newNodeCall(place,func_name)));
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        break;
        }
    case MINUS:{
        char* t1 = newTemp();
        struct InterCodes* code1 = translate_Exp(son->next,t1),
            *code2 = newNodeBiCV(SUB,place,0,t1);
        ret = catNode(code1,code2);
        break;
        }
    case LP:{
        char* t1 = newTemp();
        struct InterCodes* code1 = translate_Exp(son->next,t1),
            *code2 = newNodeAVV(place,t1);
        ret = catNode(code1,code2);
        break;
    }
    default:{
        switch (son->next->YYTYPE)
        {
        case ASSIGNOP:{
            if(son->sons->YYTYPE == ID){
                char* t1 = newTemp();
                char* var_name = lookUp(son->sons->val_ID);
                struct InterCodes* code1 = translate_Exp(son->next->next,t1)
                    ,*codet1 = newNodeAVV(var_name,t1)
                    ,*codet2 = newNodeAVV(place,var_name)
                    ,*code2 = catNode(codet1,codet2);
                ret = catNode(code1,code2);
            }else{
                char* t1 = newTemp(),*t2 = newTemp();
                struct InterCodes* code1 = lookUpExp(son,t1)
                    ,*code2 = translate_Exp(son->next->next,t2)
                    ,*codet1 = newNodeAAV(t1,t2)
                    ,*codet2 = newNodeAVV(place,t2)
                    ,*code3 = catNode(codet1,codet2);
                ret = catNode(code1,catNode(code2,code3));
            }
            break;
            }
        case PLUS:case MINUS:case STAR:case DIV:{
            char* t1 = newTemp(), *t2 = newTemp();
            struct InterCodes* code1 = translate_Exp(son,t1)
                ,*code2 = translate_Exp(son->next->next,t2)
                ,*code3 = newNodeBiVV(son->next->YYTYPE-264,place,t1,t2);
            ret = catNode(code1,catNode(code2,code3));
            break;
        }
        case LB:{
            char* t1 = newTemp();
            struct InterCodes* code1 = lookUpExp(Exp,t1)
                ,*code2;
            if(sizeElem(Exp->sons) == 4){
                code2 = newNodeAVA(place,t1);
            }else {
                code2 = newNodeAVV(place,t1);
            }
            ret = catNode(code1,code2);
            break;
        }
        case DOT:{
            char* t1 = newTemp();
            struct InterCodes* code1 = lookUpExp(Exp,t1)
                ,*code2 = newNodeAVA(place,t1);
            ret = catNode(code1,code2);
            break;
        }
        default:{
            char* label1 = newLabel(), *label2 = newLabel();
            struct InterCodes* code0 = newNodeAVC(place,0)
                ,*code1 = translate_Cond(Exp,label1,label2)
                ,*codet2 = newNodeAVC(place,1)
                ,*code2 = catNode(newNodeL(label1),codet2);

            ret = catNode(code0,catNode(code1,catNode(code2,newNodeL(label2))));
            break;
        }
        }
        break;
        }
    }
    return ret;
}

struct InterCodes* translate_Stmt(struct YYNODE* Stmt){
    #ifdef DEBUGIR
        printf("Stmt\n");
    #endif
    struct InterCodes* ret;
    struct YYNODE* son = Stmt->sons;  
    switch (son->YYTYPE)
    {
    case YYSYMBOL_Exp:{
        ret = translate_Exp(son,NULL);
        break;
        }
    case YYSYMBOL_CompSt:{
        ret = translate_CompSt(son);
        break;
    }
    case RETURN:{
        char* t1 = newTemp();
        struct InterCodes* code1 = translate_Exp(son->next,t1);
        ret = catNode(code1,newNodeRe(t1));
        break;
    }
    case IF:{
        char* label1 = newLabel(), *label2 = newLabel();
        struct InterCodes* code1 = translate_Cond(son = son->next->next,label1,label2)
            ,*code2 = translate_Stmt(son = son->next->next);
        if(son->next){
            char* label3 = newLabel();
            struct InterCodes* code3 = translate_Stmt(son->next->next);
            ret = catNode(code1,catNode(newNodeL(label1),catNode(code2,
                catNode(newNodeGo(label3),catNode(newNodeL(label2),
                catNode(code3,newNodeL(label3)))))));
        }else{
            ret = catNode(code1,catNode(newNodeL(label1),catNode(code2,newNodeL(label2))));
        }
        break;
    }
    case WHILE:{
        char* label1 = newLabel(), *label2 = newLabel(), *label3 = newLabel();
        struct InterCodes* code1 = translate_Cond(son = son->next->next,label2,label3)
            ,*code2 = translate_Stmt(son->next->next);
        ret = catNode(newNodeL(label1),catNode(code1,catNode(newNodeL(label2),
            catNode(code2,catNode(newNodeGo(label1),newNodeL(label3))))));
        break;
    }
    default:
        break;
    } 
    return ret;
}

void op(Operand op,char* s){
    if(op->kind == VARIABLE){
        sprintf(s,"%s",op->u.var_name);
    }else if(op->kind == CONSTANT){
        sprintf(s,"#%d",op->u.value);
    }else if(op->kind == ADDRESS){
        sprintf(s,"*%s",op->u.var_name);
    }else{
        sprintf(s,"&%s",op->u.var_name);
    }
}

char fours[] = "+-*/";
char fomat[][9] = {"LABEL","GOTO","RETURN","READ","WRITE","ARG"
    ,"FUNCTION", "PARAM"};
void printIR(struct InterCodes* ir,FILE* f){
    switch (ir->code.kind)
    {
    case ASSIGN:{
        char left[10], right[10];
        op(ir->code.u.assign.left,left);
        op(ir->code.u.assign.right,right);
        if(strcmp(left,"(null)")==0)
            return;
        fprintf(f,"%s := %s\n",left,right);
        break;
    }
    case ADD: case SUB: case MUL: case DIVV:{
        char res[10], op1[10], op2[10];
        op(ir->code.u.binop.result,res);
        op(ir->code.u.binop.op1,op1);
        op(ir->code.u.binop.op2,op2);
        fprintf(f,"%s := %s %c %s\n",res,op1,fours[ir->code.kind-1],op2);
        break;
    }
     case GOTO:case RET:case READ:case WRITE:
    case ARG: case PARAM:{
        fprintf(f,"%s %s\n",fomat[ir->code.kind-5],ir->code.u.label);
        break; 
    }
    case LABEL: case FUNC:{
        fprintf(f,"%s %s :\n",fomat[ir->code.kind-5],ir->code.u.label);
        break; 
    }
    case DEC:{
        char left[10];
        op(ir->code.u.assign.left,left);
        fprintf(f,"DEC %s %d\n",left,ir->code.u.assign.right->u.value);
        break;
    }
    case CALL:{
        char left[10], right[10];
        op(ir->code.u.assign.left,left);
        op(ir->code.u.assign.right,right);
        fprintf(f,"%s := CALL %s\n",left,right);
        break;
    }
    case IFGOTO:{
        char left[10], right[10];
        op(ir->code.u.ifgoto.left,left);
        op(ir->code.u.ifgoto.right,right);
        fprintf(f,"IF %s %s %s GOTO %s\n",left,ir->code.u.ifgoto.op,right,ir->code.u.ifgoto.label);
        break;
    }
    default:
        break;
    }
}

void translate(struct YYNODE* root,char* file){
    struct InterCodes* IR = newNode();
    IR->code.kind = -1;
    IR = catNode(translate_Default(root),IR);
    #ifdef DEBUGIR
    printf("finish\n");
    #endif
    if(!tranlateError){
        FILE* f = fopen(file, "w");
        while(IR->code.kind!=-1){
            printIR(IR,f);
            IR=IR->next;
        }
    }else{
        printf("Cannot translate: Code contains variables or parameters of structure type.\n");
    }
}