#ifndef __CODEGEN__
#define __CODEGEN__

#include "parser.h"

// Evaluate the syntax tree
extern int evaluateTree(BTNode *root,int reg_index);

// Print the syntax tree in prefix
extern void printPrefix(BTNode *root);

#endif // __CODEGEN__
