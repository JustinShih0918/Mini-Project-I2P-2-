#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codeGen.h"

int hasVariable(BTNode* root){
    if(root!=NULL){
        if(root->data == ID) return 1;
        else{
            if(hasVariable(root->left) || hasVariable(root->right)) return 1;
            else return 0;
        }
    }
    else return 0;
}

int evaluateTree(BTNode *root,int reg_index) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                printf("MOV r%d [%d]\n",reg_index,getMemoryAddress(root->lexeme));
                break;
            case INT:
                retval = atoi(root->lexeme);
                printf("MOV r%d %d\n",reg_index,retval);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right,reg_index);
                retval = setval(root->left->lexeme, rv);
                printf("MOV [%d] r%d\n",getMemoryAddress(root->left->lexeme),reg_index);
                break;
            case ADDSUB:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                if(strcmp(root->lexeme,"+") == 0){
                    printf("ADD r%d r%d\n",reg_index,reg_index+1);
                    retval = lv + rv;
                }
                else if(strcmp(root->lexeme,"-") == 0) {
                    printf("SUB r%d r%d\n",reg_index,reg_index+1);
                    retval = lv - rv;
                }
                break;
            case MULDIV:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                if(strcmp(root->lexeme, "*") == 0){
                    printf("MUL r%d r%d\n",reg_index,reg_index+1);
                    retval = lv * rv;
                }
                else if(strcmp(root->lexeme, "/") == 0) {
                    if(rv == 0 && !hasVariable(root->right)) {error(DIVZERO);}
                    else{
                        printf("DIV r%d r%d\n",reg_index,reg_index+1);
                        retval = lv / rv;
                    }
                }
                break;
            case INCDEC:
                rv = evaluateTree(root->right,reg_index);
                printf("MOV r%d 1\n",reg_index+1);
                if(strcmp(root->lexeme,"++") == 0){
                    printf("ADD r%d r%d",reg_index,reg_index+1);
                    retval = setval(root->right->lexeme, rv+1);
                    printf("MOV [%d] r%d\n",getMemoryAddress(root->right->lexeme),reg_index);
                }
                else if(strcmp(root->lexeme,"--") == 0){
                    printf("SUB r%d r%d",reg_index,reg_index+1);
                    retval = setval(root->right->lexeme, getval(root->right->lexeme)-1);
                    printf("MOV [%d] r%d\n",getMemoryAddress(root->right->lexeme),reg_index);
                }
                break;
            case AND:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                retval = lv & rv;
                printf("AND r%d r%d\n",reg_index,reg_index+1);
                break;
            case OR:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                retval = lv | rv;
                printf("OR r%d r%d",reg_index,reg_index+1);
                break;
            case XOR:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                retval = lv^rv;
                printf("XOR r%d r%d",reg_index,reg_index+1);
                break;
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                if(strcmp(root->lexeme,"+=") == 0){
                    retval = setval(root->left->lexeme,lv+rv);
                    printf("ADD r%d r%d",reg_index,reg_index+1);
                }
                else if(strcmp(root->lexeme,"-=") == 0){
                    retval = setval(root->left->lexeme,lv-rv);
                    printf("SUB r%d r%d",reg_index,reg_index+1);
                }
                printf("MOV [%d] r%d",getMemoryAddress(root->left->lexeme),reg_index);
                break;
            default:
                retval = 0;
        }
    }
    return retval;
}

void printPrefix(BTNode *root) {
    if (root != NULL) {
        printf("%s ", root->lexeme);
        printPrefix(root->left);
        printPrefix(root->right);
    }
}
