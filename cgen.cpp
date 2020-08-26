#include "cgen.h"

void generate_stmt(TreeNode *tree){

}

void generate_exp(TreeNode *tree){

}

void code_generate(TreeNode *tree)
{
   if (tree != NULL)
   {
      switch (tree->nk)
      {
      case StmtK:
         generate_stmt(tree);
         break;
      case ExpK:
         generate_exp(tree);
         break;
      default:
         break;
      }
      code_generate(tree->sibling);
   }
}