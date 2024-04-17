#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "codeGen.c"
#include "lex.c"

int sbcount = 0;
Symbol table[TBLSIZE];

void initTable(void) {
    strcpy(table[0].name, "x");
    table[0].val = 0;
    strcpy(table[1].name, "y");
    table[1].val = 0;
    strcpy(table[2].name, "z");
    table[2].val = 0;
    sbcount = 3;
}

int getval(char *str) {
    int i = 0; 

    for (i = 0; i < sbcount; i++)
        if (strcmp(str, table[i].name) == 0)
            return table[i].val;

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = 0;
    sbcount++;
    return 0;
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE)
        error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode*)malloc(sizeof(BTNode));
    strcpy(node->lexeme, lexe);
    node->data = tok;
    node->val = 0;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void freeTree(BTNode *root) {
    if (root != NULL) {
        freeTree(root->left);
        freeTree(root->right);
        free(root);
    }
}

// statement := ENDFILE | END | assign_expr END
void statement(void){
    BTNode *retp = NULL;
    if(match(ENDFILE)) exit(0);
    else if(match(END)){
        printf(">> ");
        advance();
    }
    else{
        retp = assign_expr();
        if(match(END)){
            printf("%d\n", evaluateTree(retp));
            printf("Prefix traversal: ");
            printPrefix(retp);
            printf("\n");
            freeTree(retp);
            printf(">> ");
            advance();
        }
        else{
            error(SYNTAXERR);
        }
    }
}

// assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr 
BTNode *assign_expr(void){
    BTNode *rept = NULL;
    BTNode *left = or_expr();

    if(match(ASSIGN) || match(ADDSUB_ASSIGN)){
        if(left->data != ID){error(SYNTAXERR);}
        else{
            if(match(ASSIGN)){
                rept = makeNode(ASSIGN,getLexeme());
                advance();
                rept->left = left;
                rept->right = assign_expr();
            }
            else if(match(ADDSUB_ASSIGN)){
                rept = makeNode(ADDSUB_ASSIGN,getLexeme());
                advance();
                rept->left = left;
                rept->right = assign_expr();
            }
            else error(SYNTAXERR);
        }
    }
    else rept = left;

    return rept;
}

// or_expr := xor_expr or_expr_tail 
BTNode *or_expr(void){
    BTNode *rept = xor_expr();
    return or_expr_tail(rept);
}

// or_expr_tail := OR xor_expr or_expr_tail | NiL 
BTNode *or_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(OR)){
        node = makeNode(OR,getLexeme());
        advance();
        node->left = left;
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    else return left;
}

// xor_expr := and_expr xor_expr_tail 
BTNode *xor_expr(void){
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

// xor_expr_tail := XOR and_expr xor_expr_tail | NiL 
BTNode *xor_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(XOR)){
        node = makeNode(XOR,getLexeme());
        advance();
        node->left = left;
        node->right = and_expr();
        return xor_expr_tail(node);
    }
    else return left;
}

// and_expr := addsub_expr and_expr_tail 
BTNode *and_expr(void){
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

// and_expr_tail := AND addsub_expr and_expr_tail | NiL 
BTNode *and_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(AND)){
        node = makeNode(AND,getLexeme());
        advance();
        node->left = left;
        node->right = addsub_expr();
        return and_expr_tail(node);
    }
    else return left;
}

// addsub_expr := muldiv_expr addsub_expr_tail 
BTNode *addsub_expr(void){
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL 
BTNode *addsub_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(ADDSUB)){
        node = makeNode(ADDSUB,getLexeme());
        advance();
        node->left = left;
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    else return left;
}

// muldiv_expr := unary_expr muldiv_expr_tail 
BTNode *muldiv_expr(void){
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL 
BTNode *muldiv_expr_tail(BTNode *left){
    BTNode *node = NULL;
    if(match(MULDIV)){
        node = makeNode(MULDIV,getLexeme());
        advance();
        node->left = left;
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    else return left;
}

// unary_expr := ADDSUB unary_expr | factor 
BTNode *unary_expr(void){
    BTNode *rept = NULL;
    if(match(ADDSUB)){
        rept->left = makeNode(INT,"0");
        rept = makeNode(ADDSUB,getLexeme());
        advance();
        rept->right = unary_expr();
    }
    else rept = factor();
    

    return rept;
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void){
    BTNode *rept = NULL;
    BTNode *left = NULL;

    if(match(INT)){
        rept = makeNode(INT,getLexeme());
        advance();
    }
    else if(match(ID)){
        rept = makeNode(ID,getLexeme());
        advance();
    }
    else if(match(INCDEC)){
        rept = makeNode(INCDEC,getLexeme());
        advance();
        if(match(ID)){
            left = makeNode(ID,getLexeme());
            advance();
            rept->left = left;
            rept->right = makeNode(ID,"1");
        }
        else err(SYNTAXERR); 
    }
    else if(match(LPAREN)){
        advance();
        rept = assign_expr();
        if(match(RPAREN)) advance();
        else error(MISPAREN);
    }

    return rept;
}
