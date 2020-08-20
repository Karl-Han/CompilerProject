#ifndef __gen_dot
#define __gen_dot

#include<stdio.h>
#include "token.h"

// // traversal
// static void traverse(TreeNode *t,
//                          void (*preProc)(TreeNode *),
//                          void (*linkProc)(TreeNode *, TreeNode *, Relation r));

typedef enum
{
  CHILD,
  SIBLING
} Relation;

void generate_dot(TreeNode *t, FILE* fp);

char* gen_dot_str(TreeNode* t);

#endif
