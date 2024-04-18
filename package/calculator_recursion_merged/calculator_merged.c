#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// for lex
#define MAXLEN 256

// Token types

typedef enum {
    UNKNOWN, END, ENDFILE, 
    INT, ID,// int , variable
    ADDSUB, MULDIV, // + or -, * or /
    ASSIGN, // =
    LPAREN, RPAREN, // (,)
    INCDEC, // ++ or --
    AND, OR, XOR, // &, |, ^
    ADDSUB_ASSIGN // += or -=
} TokenSet;
// things that missing: INCDEC(f), AND(f), OR(f), XOR(f), ADDSUB_ASSIGN(f)

// Test if a token matches the current token 
int match(TokenSet token);

// Get the next token
void advance(void);

// Get the lexeme of the current token
char *getLexeme(void);


// for parser
#define TBLSIZE 64

// Set PRINTERR to 1 to print error message while calling error()
// Make sure you set PRINTERR to 0 before you submit your code
#define PRINTERR 1

// Call this macro to print error message and exit the program
// This will also print where you called it in your program
#define error(errorNum) { \
    if (PRINTERR) \
        fprintf(stderr, "error() called at %s:%d: ", __FILE__, __LINE__); \
    err(errorNum); \
}

// Error types
typedef enum {
    UNDEFINED, MISPAREN, NOTNUMID, NOTFOUND, RUNOUT, NOTLVAL, DIVZERO, SYNTAXERR
} ErrorType;

// Structure of the symbol table
typedef struct {
    int val;
    char name[MAXLEN];
} Symbol;

// Structure of a tree node
typedef struct _Node {
    TokenSet data;
    int val;
    char lexeme[MAXLEN];
    struct _Node *left; 
    struct _Node *right;
} BTNode;

// The symbol table
Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
void initTable(void);

// Get the value of a variable
int getval(char *str);

// Set the value of a variable
int setval(char *str, int val);

int getMemoryAddress(char *str);

// Make a new node according to token type and lexeme
BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
void freeTree(BTNode *root);

//new
void statement(void);
BTNode *assign_expr(void);
BTNode *or_expr(void);
BTNode *or_expr_tail(BTNode *left);
BTNode *xor_expr(void);
BTNode *xor_expr_tail(BTNode *left);
BTNode *and_expr(void);
BTNode *and_expr_tail(BTNode *left);
BTNode *addsub_expr(void);
BTNode *addsub_expr_tail(BTNode *left);
BTNode *muldiv_expr(void);
BTNode *muldiv_expr_tail(BTNode *left);
BTNode *unary_expr(void);
BTNode *factor(void);
// the grammer
// statement := END FILE | END | assign_expr END
// assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr 
// or_expr := xor_expr or_expr_tail 
// or_expr_tail := OR xor_expr or_expr_tail | NiL 
// xor_expr := and_expr xor_expr_tail 
// xor_expr_tail := XOR and_expr xor_expr_tail | NiL 
// and_expr := addsub_expr and_expr_tail 
// and_expr_tail := AND addsub_expr and_expr_tail | NiL 
// addsub_expr := muldiv_expr addsub_expr_tail 
// addsub_expr_tail := ADDSUB muldiv_expr addsub_expr_tail | NiL 
// muldiv_expr := unary_expr muldiv_expr_tail 
// muldiv_expr_tail := MULDIV unary_expr muldiv_expr_tail | NiL 
// unary_expr := ADDSUB unary_expr | factor 
// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN

// Print error message and exit the program
void err(ErrorType errorNum);


// for codeGen
// Evaluate the syntax tree
int evaluateTree(BTNode *root,int reg_index);

// Print the syntax tree in prefix
void printPrefix(BTNode *root);


/*============================================================================================
lex implementation
============================================================================================*/

static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void)
{
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) { //INT

        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin);
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') { //ADDSUB, INCDEC, ADDSUB_ASSIGN
        lexeme[0] = c;
        c = fgetc(stdin);
        if(c == lexeme[0]){ //INCDEC
            lexeme[1] = c;
            lexeme[2] = '\0';
            return INCDEC;
        }
        else if(c == '='){ //ADDSUN_ASSIGN
            lexeme[1] = c;
            lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }
        else{
            ungetc(c,stdin); // ADDSUB
            lexeme[1] = '\0';
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') { // MULDIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') { //end
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') { // assign
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') { //Lparen
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') { // Rparen
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c) || c == '_') { // ID
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while ((isalpha(c) || c == '_' || isdigit(c))&& i<MAXLEN)
        {
            lexeme[i] = c;
            i++;
            c = fgetc(stdin);
        }
        ungetc(c,stdin);
        lexeme[i] = '\0';
        return ID;
    } else if(c == '&'){ // and
        lexeme[0] = c;
        lexeme[1] = '\0';
        return AND;
    } else if(c == '|'){ // or
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    } else if(c == '^'){ // xor
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }
    else if (c == EOF) { // end of file
        return ENDFILE;
    } else {
        return UNKNOWN;
    }
}

void advance(void) {
    curToken = getToken();
}

int match(TokenSet token) {
    if (curToken == UNKNOWN)
        advance();
    return token == curToken;
}

char *getLexeme(void) {
    return lexeme;
}

/*============================================================================================
parser implementation
============================================================================================*/


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

    if (sbcount >= TBLSIZE) error(RUNOUT);

    error(NOTFOUND);
}

int setval(char *str, int val) {
    int i = 0;

    for (i = 0; i < sbcount; i++) {
        if (strcmp(str, table[i].name) == 0) {
            table[i].val = val;
            return val;
        }
    }

    if (sbcount >= TBLSIZE) error(RUNOUT);
    
    strcpy(table[sbcount].name, str);
    table[sbcount].val = val;
    sbcount++;
    return val;
}

int getMemoryAddress(char *str){
    for(int i = 0;i<sbcount;i++){
        if(strcmp(str,table[i].name) == 0){
            return i*4;
        }
    }
    return -1;
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
    if(match(ENDFILE)){
        printf("MOV r0 [0]\n"
               "MOV r1 [4]\n"
               "MOV r2 [8]\n");
        printf("EXIT 0\n");
        exit(0);
    }
    else if(match(END)){
        advance();
    }
    else{
        retp = assign_expr();
        if(match(END)){
            evaluateTree(retp,0);
            freeTree(retp);
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
        if(left->data != ID) {error(SYNTAXERR);}
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
        rept = makeNode(ADDSUB,getLexeme());
        rept->left = makeNode(INT,"0");
        advance();
        rept->right = unary_expr();
        return rept;
    }
    else rept = factor();
    
    return rept;
}

// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void){
    BTNode *rept = NULL;

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
            rept->left = makeNode(ID,getLexeme());
            advance();
        }
        else error(SYNTAXERR); 
    }
    else if(match(LPAREN)){
        advance();
        rept = assign_expr();
        if(match(RPAREN)) advance();
        else error(MISPAREN);
    }
    else error(NOTNUMID);

    return rept;
}

void err(ErrorType errorNum) {
    printf("EXIT 1\n");
    if (PRINTERR) {
        fprintf(stderr, "error: ");
        switch (errorNum) {
            case MISPAREN:
                fprintf(stderr, "mismatched parenthesis\n");
                break;
            case NOTNUMID:
                fprintf(stderr, "number or identifier expected\n");
                break;
            case NOTFOUND:
                fprintf(stderr, "variable not defined\n");
                break;
            case RUNOUT:
                fprintf(stderr, "out of memory\n");
                break;
            case NOTLVAL:
                fprintf(stderr, "lvalue required as an operand\n");
                break;
            case DIVZERO:
                fprintf(stderr, "divide by constant zero\n");
                break;
            case SYNTAXERR:
                fprintf(stderr, "syntax error\n");
                break;
            default:
                fprintf(stderr, "undefined error\n");
                break;
        }
    }
    exit(0);
}


/*============================================================================================
codeGen implementation
============================================================================================*/

int hasVariable(BTNode* root){
    if(root == NULL) return 0;
    if(root->data == ID) return 1;
    return (hasVariable(root->right)||hasVariable(root->left));
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
                if(strcmp(root->lexeme, "*") == 0){
                    lv = evaluateTree(root->left,reg_index);
                    rv = evaluateTree(root->right,reg_index+1);
                    printf("MUL r%d r%d\n",reg_index,reg_index+1);
                    retval = lv * rv;
                }
                else if(strcmp(root->lexeme, "/") == 0) {
                    lv = evaluateTree(root->left,reg_index);
                    rv = evaluateTree(root->right,reg_index+1);
                    if(rv == 0 && !hasVariable(root->right)) error(DIVZERO);
                    if(rv!=0) retval = lv / rv;
                    printf("DIV r%d r%d\n",reg_index,reg_index+1);
                }
                break;
            case INCDEC:
                lv = evaluateTree(root->left,reg_index);
                printf("MOV r%d 1\n",reg_index+1);
                if(strcmp(root->lexeme,"++") == 0){
                    printf("ADD r%d r%d\n",reg_index,reg_index+1);
                    printf("MOV [%d] r%d\n",getMemoryAddress(root->left->lexeme),reg_index);
                    retval = setval(root->left->lexeme, lv+1);
                }
                else if(strcmp(root->lexeme,"--") == 0){
                    printf("SUB r%d r%d\n",reg_index,reg_index+1);
                    printf("MOV [%d] r%d\n",getMemoryAddress(root->left->lexeme),reg_index);
                    retval = setval(root->left->lexeme, lv-1);
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
                printf("OR r%d r%d\n",reg_index,reg_index+1);
                break;
            case XOR:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                retval = lv^rv;
                printf("XOR r%d r%d\n",reg_index,reg_index+1);
                break;
            case ADDSUB_ASSIGN:
                lv = evaluateTree(root->left,reg_index);
                rv = evaluateTree(root->right,reg_index+1);
                if(strcmp(root->lexeme,"+=") == 0){
                    retval = setval(root->left->lexeme,lv+rv);
                    printf("ADD r%d r%d\n",reg_index,reg_index+1);
                }
                else if(strcmp(root->lexeme,"-=") == 0){
                    retval = setval(root->left->lexeme,lv-rv);
                    printf("SUB r%d r%d\n",reg_index,reg_index+1);
                }
                printf("MOV [%d] r%d\n",getMemoryAddress(root->left->lexeme),reg_index);
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

int main() {
    initTable();
    while (1) {
        statement();
    }
    return 0;
}