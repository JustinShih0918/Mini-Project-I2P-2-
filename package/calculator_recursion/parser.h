#ifndef __PARSER__
#define __PARSER__

#include "lex.h"
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

extern int getMemoryAddress(char *str);

// Make a new node according to token type and lexeme
extern BTNode *makeNode(TokenSet tok, const char *lexe);

// Free the syntax tree
extern void freeTree(BTNode *root);

extern BTNode *term(void);
extern BTNode *term_tail(BTNode *left);
extern BTNode *expr(void);
extern BTNode *expr_tail(BTNode *left);

//new
extern void statement(void);
extern BTNode *assign_expr(void);
extern BTNode *or_expr(void);
extern BTNode *or_expr_tail(BTNode *left);
extern BTNode *xor_expr(void);
extern BTNode *xor_expr_tail(BTNode *left);
extern BTNode *and_expr(void);
extern BTNode *and_expr_tail(BTNode *left);
extern BTNode *addsub_expr(void);
extern BTNode *addsub_expr_tail(BTNode *left);
extern BTNode *muldiv_expr(void);
extern BTNode *muldiv_expr_tail(BTNode *left);
extern BTNode *unary_expr(void);
extern BTNode *factor(void);
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
extern void err(ErrorType errorNum);

#endif // __PARSER__
