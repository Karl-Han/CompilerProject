#include "token.h"

void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*linkProc)(TreeNode *))
{
}

void insert_node(TreeNode *t)
{
}

void null_proc(TreeNode *t)
{
    return;
}

void buildSymtab(TreeNode *t)
{
    traverse(t, insert_node, null_proc);
}