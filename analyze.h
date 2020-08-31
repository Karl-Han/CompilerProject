#ifndef __analyze
#define __analyze

#include "tables.h"
extern "C"{
    #include "token.h"
}
#include <cstdio>
#include <string>
#include <map>

extern map<string, SymTab *>* symtabs;
extern map<string, FuncTab*>* functabs;

void seg_fault();

void build_symtabs(TreeNode *);
void print_symtabs(FILE* listing);
void print_functabs(FILE* listing);
void type_check(TreeNode*);
void tag_kind(TreeNode*);

#endif