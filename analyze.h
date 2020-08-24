#ifndef __analyze
#define __analyze

extern "C"{
    #include "token.h"
}
#include <cstdio>
#include <string>
#include <map>

void build_symtabs(TreeNode *);
void print_symtabs(FILE* listing);
void print_functabs(FILE* listing);
void type_check(TreeNode*);

#endif