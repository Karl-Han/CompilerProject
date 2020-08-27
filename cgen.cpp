#include "cgen.h"
#include "code.h"
#include "analyze.h"

extern "C"{
   #include "y.tab.h"
}

extern bool TraceCode;

vector<string> func_stack;
string current_func;
static int tmpOffset = 0;

void generate_stmt(TreeNode *tree){

}

void generate_exp(TreeNode *tree){
   int loc;
   TreeNode *p1, *p2;
   switch ((yytokentype)tree->token)
   {

   case Token_number:
      if (TraceCode)
         emitComment("-> Const");
      /* gen code to load integer constant using LDC */
      emitRM("LDC", ac, tree->num, 0, "load const");
      if (TraceCode)
         emitComment("<- Const");
      break; /* ConstK */

   case Token_var:
      if (TraceCode)
         emitComment("-> Id");
      // loc = st_lookup(tree->attr.name);
      // loc is relative location
      loc = sym_lookup()
      emitRM("LD", ac, loc, gp, "load id value");
      if (TraceCode)
         emitComment("<- Id");
      break; /* IdK */

   case OpK:
      if (TraceCode)
         emitComment("-> Op");
      p1 = tree->child[0];
      p2 = tree->child[1];
      /* gen code for ac = left arg */
      code_generate(p1);
      /* gen code to push left operand */
      // CONFUSED, why tmpOffset appears?
      // What is the purpose of this statement
      emitRM("ST", ac, tmpOffset--, mp, "op: push left");
      /* gen code for ac = right operand */
      code_generate(p2);
      /* now load left operand */
      // What is the purpose of this statement
      emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");

      // all of them manipulate ac
      switch (tree->op)
      {
      case PLUS:
         emitRO("ADD", ac, ac1, ac, "op +");
         break;
      case MINUS:
         emitRO("SUB", ac, ac1, ac, "op -");
         break;
      case TIMES:
         emitRO("MUL", ac, ac1, ac, "op *");
         break;
      case OVER:
         emitRO("DIV", ac, ac1, ac, "op /");
         break;
      case LT:
         emitRO("SUB", ac, ac1, ac, "op <");
         emitRM("JLT", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case EQ:
         emitRO("SUB", ac, ac1, ac, "op ==");
         emitRM("JEQ", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      default:
         emitComment("BUG: Unknown operator");
         break;
      } /* case op */
      if (TraceCode)
         emitComment("<- Op");
      break; /* OpK */

   default:
      break;
   }
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