#ifndef __LEX__
#define __LEX__

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
extern int match(TokenSet token);

// Get the next token
extern void advance(void);

// Get the lexeme of the current token
extern char *getLexeme(void);

#endif // __LEX__
