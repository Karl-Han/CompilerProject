#ifndef __gen_dot
#define __gen_dot

#include<stdio.h>
#include "token.h"

typedef enum
{
  CHILD,
  SIBLING
} Relation;

void generate_dot(TreeNode *t, FILE* fp);

char* gen_dot_str(TreeNode* t);

#endif
