#ifndef __analyze
#define __analyze

#include "tables.h"
extern "C"{
    #include "token.h"
}
#include <cstdio>
#include <string>
#include <map>

static map<string, SymTab *> symtabs;
static map<string, FuncTab*> functabs;

void build_symtabs(TreeNode *);
void print_symtabs(FILE* listing);
void print_functabs(FILE* listing);
void type_check(TreeNode*);
void tag_kind(TreeNode*);

#endif