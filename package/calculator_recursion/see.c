#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 256

// Token types
typedef enum {
    UNKNOWN = 0,
    END = 1,
    ENDFILE = 2,
    INT = 3,
    ID = 4,
    ADDSUB = 5,
    MULDIV = 6,
    ASSIGN = 7,
    LPAREN = 8,
    RPAREN = 9,
    INCDEC = 10,
    AND = 11,
    OR = 12,
    XOR = 13,
    ADDSUB_ASSIGN = 14
} TokenSet;

// Test if a token matches the current token
int match(TokenSet token);

// Get the next token
void advance(void);

// Get the lexeme of the current token
char *getLexeme(void);

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
extern Symbol table[TBLSIZE];

// Initialize the symbol table with builtin variables
extern void initTable(void);

// Get the value of a variable
extern int getval(char *str);

// Set the value of a variable
extern int setval(char *str, int val);

extern int memAddress(char id[]);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
void freeTree(BTNode *root);

void statement(void);

BTNode *assign_expr(void);

BTNode *or_expr(void);

BTNode *or_expr_tail(BTNode *left);

BTNode *xor_expr(void);

BTNode *xor_expr_tail(BTNode *left);

BTNode *and_expr(void);

BTNode *and_expr_tail(BTNode *left);

BTNode *addsub_expr_tail(BTNode *left);

BTNode *addsub_expr(void);

BTNode *muldiv_expr(void);

BTNode *muldiv_expr_tail(BTNode *left);

BTNode *unary_expr(void);

BTNode *factor(void);

// Print error message and exit the program
void err(ErrorType errorNum);

int evaluateTree(BTNode *root,int reg);

// Print the syntax tree in prefix
void printPrefix(BTNode *root);

/*============================================================================================
lex implementation
============================================================================================*/
static TokenSet getToken(void);
static TokenSet curToken = UNKNOWN;
static char lexeme[MAXLEN];

TokenSet getToken(void) {
    int i = 0;
    char c = '\0';

    while ((c = fgetc(stdin)) == ' ' || c == '\t');

    if (isdigit(c)) {
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (isdigit(c) && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin); // 将字符推回到输入流中
        lexeme[i] = '\0';
        return INT;
    } else if (c == '+' || c == '-') {
        lexeme[0] = c, lexeme[1] = '\0';
        c = fgetc(stdin);
        if(c == lexeme[0]){
            lexeme[1] = c, lexeme[2] = '\0';
            return INCDEC;
        }
        if(c == '='){
            lexeme[1] = '=', lexeme[2] = '\0';
            return ADDSUB_ASSIGN;
        }
        ungetc(c, stdin);
        return ADDSUB;
    } else if (c == '*' || c == '/') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return MULDIV;
    } else if (c == '\n') {
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') {
        strcpy(lexeme, "=");
        return ASSIGN;
    } else if (c == '(') {
        strcpy(lexeme, "(");
        return LPAREN;
    } else if (c == ')') {
        strcpy(lexeme, ")");
        return RPAREN;
    } else if (isalpha(c) || c == '_') {
        lexeme[0] = c;
        i = 1;
        c = fgetc(stdin);
        while ((isdigit(c) || isalpha(c) || c == '_') && i < MAXLEN) {
            lexeme[i] = c;
            ++i;
            c = fgetc(stdin);
        }
        ungetc(c, stdin); // 将字符推回到输入流中
        lexeme[i] = '\0';
        return ID;
    } else if (c == EOF) {
        return ENDFILE;
    } else if (c == '&') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return  AND;
    } else if (c == '|') {
        lexeme[0] = c;
        lexeme[1] = '\0';
        return OR;
    } else if (c == '^'){
        lexeme[0] = c;
        lexeme[1] = '\0';
        return XOR;
    }else {
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

int memAddress(char id[]) {
    for (int i = 0; i < sbcount; i++)
        if (strcmp(id, table[i].name) == 0)
            return 4 * i;
    return -1;
}

BTNode *makeNode(TokenSet tok, const char *lexe) {
    BTNode *node = (BTNode *) malloc(sizeof(BTNode));
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
// statement := ENDFILE | END | expr END
void statement(void) {
    BTNode *retp = NULL;
    if (match(ENDFILE)) {
        printf("MOV r0 [0]\n"
               "MOV r1 [4]\n"
               "MOV r2 [8]\n");
        printf("EXIT 0\n");

        exit(0);
    } else if (match(END)) {
        advance();
    } else {
        retp = assign_expr();
        if (match(END)) {
            evaluateTree(retp,0);
            freeTree(retp);
            advance();
        } else {
            error(SYNTAXERR);
        }
    }
}

//assign_expr := ID ASSIGN assign_expr | ID ADDSUB_ASSIGN assign_expr | or_expr
BTNode *assign_expr(void) {
    BTNode *left = or_expr();
    BTNode *node;

    if (match(ASSIGN) || match(ADDSUB_ASSIGN)){
        if (left->data != ID) error(SYNTAXERR);
        if (match(ASSIGN))
            node = makeNode(ASSIGN,getLexeme());
        else
            node = makeNode(ADDSUB_ASSIGN,getLexeme());
        advance();
        node->left = left;
        node->right = assign_expr();
    }
    else
        node = left;

    return node;

}

BTNode *or_expr(void) {
    BTNode *node = xor_expr();
    return or_expr_tail(node);
}

BTNode *or_expr_tail(BTNode *left) {
    BTNode *node = NULL;
    if (match(OR)) {
        node = makeNode(OR, getLexeme());
        node->left = left;
        advance();
        node->right = xor_expr();
        return or_expr_tail(node);
    }
    return left;
}

BTNode *xor_expr(void) {
    BTNode *node = and_expr();
    return xor_expr_tail(node);
}

BTNode *xor_expr_tail(BTNode *left) {
    BTNode *node = NULL;
    if (match(XOR)) {
        node = makeNode(XOR, getLexeme());
        node->left = left;
        advance();
        node->right = and_expr();
        return xor_expr_tail(node);
    }
    return left;
}

BTNode *and_expr(void) {
    BTNode *node = addsub_expr();
    return and_expr_tail(node);
}

BTNode *and_expr_tail(BTNode *left) {
    BTNode *node = NULL;
    if (match(AND)) {
        node = makeNode(AND, getLexeme());
        node->left = left;
        advance();
        node->right = addsub_expr();
        return and_expr_tail(node);
    }
    return left;
}

BTNode *addsub_expr(void) {
    BTNode *node = muldiv_expr();
    return addsub_expr_tail(node);
}

BTNode *addsub_expr_tail(BTNode *left) {
    BTNode *node = NULL;
    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        node->left = left;
        advance();
        node->right = muldiv_expr();
        return addsub_expr_tail(node);
    }
    return left;
}

BTNode *muldiv_expr(void) {
    BTNode *node = unary_expr();
    return muldiv_expr_tail(node);
}

BTNode *muldiv_expr_tail(BTNode *left) {
    BTNode *node = NULL;
    if (match(MULDIV)) {
        node = makeNode(MULDIV, getLexeme());
        node->left = left;
        advance();
        node->right = unary_expr();
        return muldiv_expr_tail(node);
    }
    return left;
}

//unary_expr := ADDSUB unary_expr | factor
BTNode *unary_expr(void) {
    BTNode *node = NULL;
    if (match(ADDSUB)) {
        node = makeNode(ADDSUB, getLexeme());
        node->left = makeNode(INT, "0");
        advance();
        node->right = unary_expr();
        return node;
    }

    return factor();
}


// factor := INT | ID | INCDEC ID | LPAREN assign_expr RPAREN
BTNode *factor(void) {
    BTNode *retp = NULL;

    if (match(INT)) {
        retp = makeNode(INT, getLexeme());
        advance();
    } else if (match(ID)) {
        retp = makeNode(ID, getLexeme());
        advance();
    } else if (match(INCDEC)) {
        retp = makeNode(INCDEC, getLexeme());
        advance();
        if (match(ID)) {
            retp->left = makeNode(ID, getLexeme());
            advance();
        } else error(SYNTAXERR);
    } else if (match(LPAREN)) {
        advance();
        retp = assign_expr();
        if (match(RPAREN))
            advance();
        else error(MISPAREN);
    } else {
        error(NOTNUMID);
    }
    return retp;
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

int id_find(BTNode *root){
    if (root == NULL) return 0;
    if (root->data == ID) return 1;
    return id_find(root->left) || id_find(root->right);
}

int evaluateTree(BTNode *root, int reg) {
    int retval = 0, lv = 0, rv = 0;

    if (root != NULL) {
        switch (root->data) {
            case ID:
                retval = getval(root->lexeme);
                printf("MOV r%d [%d]\n", reg, memAddress(root->lexeme));
                break;
            case INT:
                retval = atoi(root->lexeme);
                printf("MOV r%d %d\n", reg, retval);
                break;
            case ASSIGN:
                rv = evaluateTree(root->right, reg);
                retval = setval(root->left->lexeme, rv);
                printf("MOV [%d] r%d\n", memAddress(root->left->lexeme), reg);
                break;
            case ADDSUB:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);
                if (strcmp(root->lexeme, "+") == 0) {
                    printf("ADD r%d r%d\n", reg, reg+1);
                    retval = lv + rv;
                } else if (strcmp(root->lexeme, "-") == 0) {
                    retval = lv - rv;
                    printf("SUB r%d r%d\n", reg, reg+1);
                }
                break;
            case MULDIV:
                if (strcmp(root->lexeme, "*") == 0) {
                    lv = evaluateTree(root->left, reg);
                    rv = evaluateTree(root->right, reg+1);
                    retval = lv * rv;
                    printf("MUL r%d r%d\n", reg, reg+1);
                } else if (strcmp(root->lexeme, "/") == 0) {
                    lv = evaluateTree(root->left, reg);
                    rv = evaluateTree(root->right, reg+1);
                    if (rv == 0&&!id_find(root->right)) error(DIVZERO);
                    if(rv!=0) retval = lv / rv;
                    printf("DIV r%d r%d\n", reg, reg+1);
                }
                break;
            case INCDEC:
                lv = evaluateTree(root->left, reg);
                printf("MOV r%d 1\n", reg+1);
                if(strcmp(root->lexeme, "++")==0){
                    printf("ADD r%d r%d\n", reg, reg+1);
                    printf("MOV [%d] r%d\n", memAddress(root->left->lexeme), reg);
                    retval=setval(root->left->lexeme, lv+1);
                }else{
                    printf("SUB r%d r%d\n", reg, reg+1);
                    printf("MOV [%d] r%d\n", memAddress(root->left->lexeme), reg);
                    retval=setval(root->left->lexeme, lv-1);
                }
                break;
            case AND:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);
                retval=lv&rv;
                printf("AND r%d r%d\n", reg, reg+1);
                break;
            case OR:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);
                printf("OR r%d r%d\n", reg, reg+1);
                retval=lv|rv;
                break;
            case XOR:
                lv = evaluateTree(root->left, reg);
                rv = evaluateTree(root->right, reg+1);
                printf("XOR r%d r%d\n", reg, reg+1);
                retval=lv^rv;
                break;
            case ADDSUB_ASSIGN:
                lv=evaluateTree(root->left, reg);
                rv= evaluateTree(root->right, reg+1);
                if(strcmp(root->lexeme, "+=")==0){
                    retval=setval(root->left->lexeme, lv+rv);
                    printf("ADD r%d r%d\n", reg, reg+1);
                }else if(strcmp(root->lexeme, "-=")==0){
                    retval=setval(root->left->lexeme, lv-rv);
                    printf("SUB r%d r%d\n", reg, reg+1);
                }
                printf("MOV [%d] r%d\n", memAddress(root->left->lexeme), reg);
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
    //freopen("a.txt", "w", stdout);
    initTable();
    while (1) {
        statement();
    }
    return 0;
}