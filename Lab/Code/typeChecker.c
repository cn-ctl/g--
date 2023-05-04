#include "syntax.tab.h"
#include "g--.h"
//#define DEBUG
IDnode IDlist[0x3fff],decFuncList;

bool hasError = false;

static unsigned int hash(char* name){
    unsigned int val = 0, i;
    for(;*name;++name){
        val = (val<<2)+*name;
        if(i=val&~0x3fff) val = (val ^ (i>>12)) & 0x3fff;
    }
    return val;
}

bool checkAssign(Type left,Type right){
    if(left == NULL||right == NULL||left->kind!=right->kind){
        return false;
    }
    if(left->kind==BASIC){
        return left->basic == right->basic;
    }else if(left->kind == STRUCTURE){
        return strcmp(left->struc.name,right->struc.name) == 0;
    }else{
        while(left->kind==ARRAY&&right->kind==ARRAY){
            left = left->array.elem;
            right = right->array.elem;
        }
        return checkAssign(left,right);
    }
}

bool hasID(char* name,unsigned int index,Type* type,bool fun){
    //printf("finding %s\n",name);
    IDnode t = IDlist[index];
    while(t!=NULL){
        if(fun&&t->type->kind!=FUNCTION){
            t=t->next;
            continue;
        }
        if(strcmp(t->name,name) == 0){
            *type = t->type;
            return true;
        }
        t=t->next;
    }
    return false;
}

bool addID(char* name,unsigned int index,Type type){
    //printf("adding %s\n",name);
    if(type->kind !=FUNCTION && type->dec){
        type->dec = 0;
        return true;
    }
    if(type->kind == FUNCTION&&type->dec){
        Type funcType;
        if(hasID(name,index,&funcType,true)){
            if(!checkAssign(funcType->func.ret,type->func.ret))
                return false;
            ParaList x = funcType->func.para, y = type->func.para;
            while(x&&y){
                if(checkAssign(x->type,y->type)){
                    x = x->next;y=y->next;
                }else return false;
            }
            return x==NULL&&y==NULL;
        }else
        {       
            if(decFuncList){
                IDnode t = decFuncList;
                while(t->next){
                    if(strcmp(name,t->name) == 0){
                        if(!checkAssign(t->type->func.ret,type->func.ret))
                            return false;
                        ParaList x = t->type->func.para, y = type->func.para;
                        while(x&&y){
                            if(checkAssign(x->type,y->type)){
                                x = x->next;y=y->next;
                            }else return false;
                        }
                        return x==NULL&&y==NULL;
                    }
                    t = t->next;
                }
                if(strcmp(name,t->name) == 0){
                    ParaList x = t->type->func.para, y = type->func.para;
                    if(!checkAssign(t->type->func.ret,type->func.ret))
                        return false;
                    while(x&&y){
                        if(checkAssign(x->type,y->type)){
                            x = x->next;y=y->next;
                        }else return false;
                    }
                    return x==NULL&&y==NULL;
                }else{
                    IDnode newID = (IDnode)malloc(sizeof(struct IDnode_));
                    newID->name=name;
                    newID->next = NULL;
                    newID->type = type;
                    t->next = newID;
                }
            }else{
                IDnode newID = (IDnode)malloc(sizeof(struct IDnode_));
                newID->name=name;
                newID->next = NULL;
                newID->type = type;
                decFuncList = newID;
            }
        }
    }else{
        IDnode t;
        if(type->kind == FUNCTION){
            t = decFuncList;
            while(t){
                if(strcmp(name,t->name) == 0){
                    if(!checkAssign(t->type->func.ret,type->func.ret))
                        {type->dec = true;return false;}
                    ParaList x = t->type->func.para, y = type->func.para;
                    while(x&&y){
                        if(checkAssign(x->type,y->type)){
                            x = x->next;y=y->next;
                        }else {type->dec = true;return false;}
                    }
                    t->type->dec = 0;
                    break;
                }
                t = t->next;
            }
        }
        t = IDlist[index];
        if(t){
            if((type->kind<=2&&t->type->kind<=2||type->kind==t->type->kind)&&strcmp(t->name,name)==0)
                return false;
            while(t->next){
                t = t->next;
                if((type->kind<=2&&t->type->kind<=2||type->kind==t->type->kind)&&strcmp(t->name,name)==0)
                    return false;
            }
            IDnode newID = (IDnode)malloc(sizeof(struct IDnode_));
            newID->name=name;
            newID->next = NULL;
            newID->type = type;
            t->next = newID;
        }else{
            IDnode newID = (IDnode)malloc(sizeof(struct IDnode_));
            newID->name=name;
            newID->next = NULL;
            newID->type = type;
            IDlist[index] = newID;
        }
        return true;
    }
}

Type makeArray(Type type,int size){
    Type ret = (Type)malloc(sizeof(struct Type_));
    ret->kind = ARRAY;
    ret->array.elem = type;
    ret->dec = type->dec;
    type->dec = 0;
    ret->belongingStruct = type->belongingStruct;
    if(type->kind!=ARRAY){
        ret->array.size = 4*size;
    }else{
        ret->array.size = type->array.size*size;
    }
    return ret;
}

Type copUP(Type dest,Type src){
    if(dest == NULL)
        return src;
}

Type intType,floatType;
int stackLoad = 0;
void typeChecker(struct YYNODE* root){
    if(!root)
        return;
    #ifdef DEBUG
        printf("%d %d\n",root->YYTYPE,stackLoad++);
    #endif
    struct YYNODE* son = root->sons;
    switch (root->YYTYPE)
    {
    case YYSYMBOL_ExtDef:case YYSYMBOL_Def:
        typeChecker(son);
        Type specifierType = son->type;
        int specifierKind = son->kind;
        if(root->type&&root->type->kind==STRUCTURE&&son->type)
            son->type->belongingStruct = root->type;
        son = son->next;
        if(son->YYTYPE == YYSYMBOL_FunDec&&son->next&&son->next->YYTYPE == SEMI)
            specifierType->dec = root->first_line;
        else specifierType->dec = 0;
        son->type = specifierType;
        son->kind = specifierKind;
        while(son){
            if(son->YYTYPE == YYSYMBOL_CompSt)
                son->returnType = specifierType;
            typeChecker(son);

            son = son->next;
        }
        specifierType->belongingStruct = NULL;
        break;
    case YYSYMBOL_Specifier:
        typeChecker(son);
        root->type = son->type;
        root->kind = son->kind;
        son = son->next;
        break;
    case YYSYMBOL_ExtDecList:case YYSYMBOL_DecList:case YYSYMBOL_DefList:case YYSYMBOL_CompSt: case YYSYMBOL_StmtList: case YYSYMBOL_VarList:
        while(son){
            son->type = root->type;
            son->kind = root->kind;
            son->returnType = root->returnType;
            typeChecker(son);
            son = son->next;
        }
        break;
    case YYSYMBOL_Stmt:
        if(son->YYTYPE==RETURN){
            son = son->next;
            typeChecker(son);
            if(!checkAssign(son->type,root->returnType)){
                hasError = true;
                printf("Error type 8 at Line %d: Type mismatched for return.\n",root->first_line);
            }
            son = son->next->next;
            break;
        }else{
            while(son){
                son->type = root->type;
                son->kind = root->kind;
                son->returnType = root->returnType;
                typeChecker(son);
                son = son->next;
            }
        }
        break;
    case YYSYMBOL_Dec:
        son->type = root->type;
        son->kind = root->kind;
        typeChecker(son);
        if(son->next){
            if(root->type&&root->type->belongingStruct){
                hasError = true;
                printf("Error type 15 at Line %d: initailing field while defining structure.\n",root->first_line);
                son = son->next->next->next;
                break;
            }
            Type left = son->type;
            son = son->next->next;
            typeChecker(son);
            Type right = son->type;
            if(!checkAssign(left,right)){
                hasError = true;
                printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->first_line);
            }
        }
        son=son->next;
        break;
    case YYSYMBOL_FunDec:
        root->kind = function;
        Type temp = (Type)malloc(sizeof(struct Type_));
        temp->kind=FUNCTION;
        temp->func.argIndex = 0;
        temp->func.ret=root->type;
        temp->func.para = NULL;
        temp->dec = root->type->dec;
        temp->belongingStruct = NULL;
        root->type = temp;
        struct YYNODE* finalDeal = son;
        son->type = root->type;
        son->kind = root->kind;
        son = son->next;
        while(son){
            son->type = root->type;
            son->kind = root->kind;
            typeChecker(son);
            son = son->next;
        }
        typeChecker(finalDeal);
        break;
    case YYSYMBOL_ParamDec:
        typeChecker(son);
        son->next->type = son->type;
        son->type->dec = root->type->dec;
        son->next->kind = son->kind;
        son = son->next;
        typeChecker(son);
        ParaList para = (ParaList)malloc(sizeof(struct ParaList_));
        para->name = son->val_ID;
        para->type = son->type;
        para->next = NULL;
        if(root->type->func.para){
            ParaList t = root->type->func.para;
            while(t->next)
                t=t->next;
            t->next = para;
        }else root->type->func.para = para;
        son = son->next;
        break;
    case YYSYMBOL_VarDec:
        if(son->YYTYPE==YYSYMBOL_VarDec){
            int size = son->next->next->val_int;
            root->type = makeArray(root->type,size);
        }
        
        son->kind = variable;
        son->type = root->type;
        typeChecker(son);
        root->type = son->type;
        root->val_ID = son->val_ID;
        while(son)
            son = son->next;
        root->kind = variable;
        break;
    case TYPE:
        root->type = strcmp(root->val_type,"int")==0?intType:floatType;
        root->kind = variable;
        break;
    case YYSYMBOL_StructSpecifier:
        root->kind = structrue;
        son = son->next;
        if(son->YYTYPE!=YYSYMBOL_Tag){
            root->type = (Type)malloc(sizeof(struct Type_));
            root->type->kind = STRUCTURE;
            root->type->struc.structure = NULL;
            root->type->belongingStruct = NULL;
            root->type->dec = 0;
            while(son!=NULL){
                son->type = root->type;
                son->kind = root->kind;
                typeChecker(son);
                son=son->next;
            }
        }else{
            son->kind = root->kind;
            typeChecker(son);
            root->type = son->type;
            son = son->next;
        }
        break;
    case YYSYMBOL_OptTag:
        if(son){
            son->type = root->type;
            son->kind = root->kind;
            typeChecker(son);
            son->type->struc.name = son->val_ID;
            son=son->next;
        }
        break;
    case YYSYMBOL_Tag:
        son->kind = structrue;
        typeChecker(son);
        root->type = son->type;
        son = son->next;
        break;
    case YYSYMBOL_Exp:
        son->kind = variable;
        if(son->next){
            Type leftType, rightType;
            #ifdef DEBUG
            printf("exp:%d\n",son->next->YYTYPE);
            #endif
            switch (son->next->YYTYPE)
            {
            case ASSIGNOP:
                typeChecker(son);
                if(!son->type||!son->left){
                    hasError = true;
                    printf("Error type 6 at Line %d: The left-hand side of an assignment must be a variable.\n",root->first_line);
                    son = son->next->next;
                    break;;
                }
                leftType = son->type;
                son = son->next->next;
                typeChecker(son);
                rightType = son->type;
                if(!checkAssign(leftType,rightType)){
                    hasError = true;
                    printf("Error type 5 at Line %d: Type mismatched for assignment.\n",root->first_line);
                }
                root->type = rightType;
                root->left = true;
                break;
            case AND:case OR:
                typeChecker(son);
                leftType = son->type;
                son = son->next->next;
                typeChecker(son);
                rightType = son->type;
                if(!leftType||!rightType||leftType->kind!=BASIC||leftType->basic!=INT||rightType->kind!=BASIC||rightType->basic!=INT){
                    hasError = true;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->first_line);
                }
                root->type = son->type;
                root->kind = num;
                root->left = 0;
                break;
            case PLUS:case MINUS:case STAR:case DIV:
                typeChecker(son);
                leftType = son->type;
                son = son->next->next;
                typeChecker(son);
                rightType = son->type;
                if(!leftType||!rightType||leftType->kind!=BASIC||rightType->kind!=BASIC||rightType->basic!=leftType->basic){
                    hasError = true;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->first_line);
                }
                root->type = son->type;
                root->kind = num;
                root->left = 0;
                break;
            case YYSYMBOL_Exp:
                typeChecker(son->next);
                
                if(son->YYTYPE == NOT&&(!son->next->type||son->next->type->kind!=BASIC||son->next->type->basic!=INT)){{
                    hasError = true;
                    son=son->next;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->first_line);
                    break;
                }
                }else if(!son->next->type||son->YYTYPE == MINUS&&son->next->type->kind!=BASIC){
                    hasError = true;
                    son=son->next;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->first_line);
                    break;
                }
                root->type = son->next->type;
                root->kind = num;
                root->left = 0;
                son=son->next;
                break;
            case RELOP:
                typeChecker(son);
                leftType = son->type;
                if(!leftType){
                    son = son->next->next;
                    break;
                }
                son = son->next->next;
                typeChecker(son);
                rightType = son->type;
                if(!rightType)
                    break;
                if(leftType->kind!=BASIC||rightType->kind!=BASIC||rightType->basic!=leftType->basic){
                    hasError = true;
                    printf("Error type 7 at Line %d: Type mismatched for operands.\n",root->first_line);
                }
                root->type = intType;
                root->kind = num;
                root->left = 0;
                break;
            case DOT:
                typeChecker(son);
                if(!son->type||son->type->kind != STRUCTURE){
                    hasError = true;
                    printf("Error type 13 at Line %d: Illegal use of \".\".\n", root->first_line);
                    while(son->next)
                        son=son->next;
                    break;
                }
                FieldList thisField = son->type->struc.structure;
                son = son->next->next;
                while(thisField){
                    if(strcmp(son->val_ID,thisField->name) == 0){
                        root->type = thisField->type;
                        root->left = 1;
                        break;
                    }
                    thisField = thisField->next;
                }
                if(!thisField){
                    hasError = true;
                    printf("Error type 14 at Line %d: Non-existent field \"%s\".\n",root->first_line,son->val_ID);
                }
                break;
            case LP:
                son->kind = function;
                root->left = 0;
                typeChecker(son);
                if(!son->type||son->type->kind != FUNCTION){
                    hasError = true;
                    printf("Error type 11 at Line %d: \"%s\" is not a function.\n",root->first_line,son->val_ID);
                    son = son->next->next;
                    break;
                }
                root->type = son->type->func.ret;
                leftType = son->type;
                son = son->next->next;
                son->type=leftType;
                int tempIndex = son->type->func.argIndex;
                son->type->func.argIndex = 0;
                typeChecker(son);
                ParaList matchArg = son->type->func.para;
                for(int i = 0;i<son->type->func.argIndex&&matchArg;i++){
                    matchArg = matchArg->next;
                }
                son->type->func.argIndex = tempIndex;
                if(matchArg&&matchArg->next!=NULL){
                    hasError = true;
                    printf("Error type 9 at Line %d: lack of args\n",root->first_line);
                }
                break;
            case LB:
                typeChecker(son);
                leftType = son->type;
                if(!leftType||leftType->kind!=ARRAY){
                    hasError = true;
                    printf("Error type 10 at Line %d: %s is not an array.\n",root->first_line,son->val_ID);
                    while(son->next)
                        son=son->next;
                    break ;
                }
                root->type = leftType->array.elem;
                root->left = 1;
                son = son->next->next;
                typeChecker(son);
                if(!son->type||son->type->kind!=BASIC||son->type->basic!=INT){
                    hasError = true;
                    printf("Error type 12 at Line %d: num in [] is not an integer.\n",root->first_line);
                }
                break;
            default:
                break;
            }
            son = son->next;
        }else{
            son->kind = variable;
            typeChecker(son);
            root->type = son->type;
            root->left = son->left;
            if(son->YYTYPE == ID)
                root->val_ID = son->val_ID;
            else if(son->YYTYPE == INT)
                root->val_int = son->val_int;
            else root->val_float = son->val_float;
            son = son->next;
        }
        
        break;
    case YYSYMBOL_Args:
        typeChecker(son);
        ParaList paraT = root->type->func.para;
        for(int i = 0;i < root->type->func.argIndex&&paraT;i++){
            paraT = paraT->next;
        }
        if(!paraT){
            hasError = true;
            printf("Error type 9 at Line %d: too many args\n",root->first_line);
        }
        else if(!checkAssign(son->type,paraT->type)){
            hasError = true;
            printf("Error type 9 at Line %d: Function is not applicable for arguments at the %d arg\n",root->first_line,root->type->func.argIndex);
        }
        if(son->next){
            son = son->next->next;
            root->type->func.argIndex++;
            son ->type = root->type;
            typeChecker(son);
            son = son->next;
        }else son = son->next;
        break;
    case ID:
        root->left = 1;
        if(root->type == NULL){
            if(!hasID(root->val_ID,hash(root->val_ID),&root->type,root->kind==function)){
                hasError = true;
                printf("Error type %d at Line %d: Undefined ID %s.\n", root->kind,root->first_line, root->val_ID);
                root->type = intType;
            }
        }else{
            if(root->type->belongingStruct){
                Type t = root->type->belongingStruct;
                FieldList fieldOfS = t->struc.structure;
                FieldList thisField = (FieldList)malloc(sizeof(struct FieldList_));
                thisField->name = root->val_ID;
                thisField->type = root->type;
                thisField->next = NULL;
                if(fieldOfS){
                    while(fieldOfS->next!=NULL){
                        if(strcmp(fieldOfS->name,thisField->name) == 0){
                            hasError = true;
                            printf("Error type 15 at Line %d: Redefined field \"%s\".\n",root->first_line,thisField->name);
                            free(thisField);
                            #ifdef DEBUG
                            printf("q%d %d\n",root->YYTYPE,--stackLoad);
                            #endif
                            return;
                        }
                        fieldOfS = fieldOfS->next;
                    }
                    if(strcmp(fieldOfS->name,thisField->name) == 0){
                        hasError = true;
                        printf("Error type 15 at Line %d: Redefined field \"%s\".\n",root->first_line,thisField->name);
                        free(thisField);
                        break;
                    }
                    fieldOfS->next = thisField;
                }else{
                    t->struc.structure = thisField;
                }
            }
            else if(!addID(root->val_ID,hash(root->val_ID),root->type)){
                if(root->type->kind==FUNCTION&&root->type->dec){
                    hasError = true;
                    printf("Error type 19 at Line %d: Inconsistent declaration of function \"%s\".\n",root->first_line, root->val_ID);
                }else
                {
                    int errorType = root->kind<=2?(root->kind+2):16;
                    hasError = true;
                    printf("Error type %d at Line %d: Redefined ID %s.\n", errorType,root->first_line, root->val_ID);
                }
            }
        }
        break;
    case INT: root->type = intType;break;
    case FLOAT: root->type = floatType;break;
    default:break;
    }
    while(son!=NULL){
        typeChecker(son);
        son=son->next;
    }
    #ifdef DEBUG
    printf("q%d %d\n",root->YYTYPE,--stackLoad);
    #endif
}

bool typeCheck(struct YYNODE* root){
    intType = (Type)malloc(sizeof(struct Type_));
    intType->kind=BASIC;
    intType->basic=INT;
    intType->belongingStruct =NULL;
    floatType = (Type)malloc(sizeof(struct Type_));
    floatType->kind=BASIC;
    floatType->basic=FLOAT;
    floatType->belongingStruct =NULL;

    //forIR,add2func
    Type readType = (Type)malloc(sizeof(struct Type_));
    readType->kind=FUNCTION;
    readType->func.ret = intType;
    readType->func.para = NULL;
    readType->func.argIndex = 0;
    readType->func.paraNum = 0;
    readType->dec = 0;
    readType->belongingStruct = NULL;
    static char readID[5] = "read";
    addID(readID,hash(readID), readType);

    Type writeType = (Type)malloc(sizeof(struct Type_));
    writeType->kind=FUNCTION;
    writeType->func.ret = intType;
    ParaList para = (ParaList)malloc(sizeof(struct ParaList_));
    para->type = intType;
    para->name = "writeArg";
    para->next = NULL;
    writeType->func.para = para;
    writeType->func.argIndex = 0;
    writeType->func.paraNum = 1;
    writeType->dec = 0;
    writeType->belongingStruct = NULL;
    static char writeID[6] = "write";
    addID(writeID,hash(writeID), writeType);
    /////////

    typeChecker(root);
    IDnode t = decFuncList;
    while(t){
        if(t->type->dec){
            hasError = true;
            printf("Error type 18 at Line %d: Undefined function \"%s\".\n",t->type->dec,t->name);
        }
        t = t->next;
    }
    return !hasError;
}