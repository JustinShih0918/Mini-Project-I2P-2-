#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lex.h"

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
        for(int j = 0;j<i;j++){
            printf("%c",lexeme[j]);
        }
        printf("\n");
        return INT;
    } else if (c == '+' || c == '-') { //ADDSUB, INCDEC, ADDSUB_ASSIGN
        lexeme[0] = c;
        c = fgetc(stdin);
        if(c == lexeme[0]){ //INCDEC
            lexeme[1] = c;
            lexeme[2] = '\0';
            printf("%c%c\n",lexeme[0],lexeme[1]);
            return INCDEC;
        }
        else if(c == '='){ //ADDSUN_ASSIGN
            lexeme[1] = c;
            lexeme[2] = '\0';
            printf("%c%c\n",lexeme[0],lexeme[1]);
            return ADDSUB_ASSIGN;
        }
        else{
            ungetc(c,stdin); // ADDSUB
            lexeme[1] = '\0';
            printf("%c\n",lexeme[0]);
            return ADDSUB;
        }
    } else if (c == '*' || c == '/') { // MULDIV
        lexeme[0] = c;
        lexeme[1] = '\0';
        printf("%c\n",lexeme[0]);
        return MULDIV;
    } else if (c == '\n') { //end
        lexeme[0] = '\0';
        return END;
    } else if (c == '=') { // assign
        strcpy(lexeme, "=");
        printf("%c\n",lexeme[0]);
        return ASSIGN;
    } else if (c == '(') { //Lparen
        strcpy(lexeme, "(");
        printf("%c\n",lexeme[0]);
        return LPAREN;
    } else if (c == ')') { // Rparen
        strcpy(lexeme, ")");
        printf("%c\n",lexeme[0]);
        return RPAREN;
    } else if (isalpha(c) || c == '_') { // ID
        lexeme[0] = c;
        c = fgetc(stdin);
        i = 1;
        while (c != '\t' && c != EOF && c !='\n' && c != ' ' && c != '+' && c !='-' && c != '&' && c !='|' && c !='*'&& c !='^' && c !='/')
        {
            lexeme[i] = c;
            i++;
            c = fgetc(stdin);
        }
        ungetc(c,stdin);
        for(int j = 0;j<i;j++){
            printf("%c",lexeme[j]);
        }
        printf("\n");
        lexeme[i] = '\0';
        return ID;
    } else if(c == '&'){ // and
        lexeme[0] = c;
        lexeme[1] = '\0';
        printf("%c\n",lexeme[0]);
        return AND;
    } else if(c == '|'){ // or
        lexeme[0] = c;
        lexeme[1] = '\0';
        printf("%c\n",lexeme[0]);
        return OR;
    } else if(c == '^'){ // xor
        lexeme[0] = c;
        lexeme[1] = '\0';
        printf("%c\n",lexeme[0]);
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
