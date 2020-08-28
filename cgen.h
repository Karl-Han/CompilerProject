#ifndef __cgen
#define __cgen

#include "token.h"

void code_generate(TreeNode*);
void generate_stmt(TreeNode *tree);
void generate_exp(TreeNode *tree);

#endif