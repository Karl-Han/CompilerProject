#include "cgen.h"
#include "code.h"
#include "analyze.h"

extern "C"
{
#include "y.tab.h"
}

extern bool TraceCode;

vector<string> func_stack;
string current_func;
static int tmpOffset = 0;

void code_generate_inner(TreeNode *tree)
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

void code_generate(TreeNode *tree)
{
   // start with the function main
   current_func = "main";
   code_generate_inner(tree);
}

void generate_stmt(TreeNode *tree)
{
   TreeNode *p1, *p2, *p3;
   // this is local variable
   int savedLoc1, savedLoc2, currentLoc;
   int loc;
   switch ((yytokentype)tree->token)
   {
   case Token_if:
      if (TraceCode)
         emitComment("-> if");

      // predicate, pos_stmt, neg_stmt
      p1 = tree->child[0];
      p2 = tree->child[1];
      p3 = tree->child[2];

      /* generate code for test expression */
      // predicate part generate
      code_generate_inner(p1);

      // used to store the location for true
      char *buf;
      buf = (char *)malloc(sizeof(char) * 20);
      savedLoc1 = emitSkip(1);
      sprintf(buf, "savedLoc1: %d", savedLoc1);
      emitComment(buf);

      /* recurse on then part */
      // pos_stmt part generate
      emitComment("if: jump to else belongs here");
      code_generate_inner(p2);
      savedLoc2 = emitSkip(1);

      // use the predicate to know where to go
      emitComment("if: jump to end belongs here");
      currentLoc = emitSkip(0);
      // restore savedLoc1 to emitLoc
      emitBackup(savedLoc1);
      // if reg[ac]=0, jump to currentLoc, which is mem(currentLoc - emitLoc - 1 + reg[pc])
      // for SAMPLE.tm it is reg[pc] = 14, currentLoc = 41
      emitRM_Abs("JEQ", ac, currentLoc, "if: jmp to else");
      // restore to highest loc
      emitRestore();

      /* recurse on else part */
      // neg_stmt part generate
      code_generate_inner(p3);
      currentLoc = emitSkip(0);
      emitBackup(savedLoc2);
      // I think it is just a relative value between
      // currentLoc and emitLoc
      emitRM_Abs("LDA", pc, currentLoc, "jmp to end");
      emitRestore();
      if (TraceCode)
         emitComment("<- if");
      break; /* if_k */

   case Token_assign:
      if (TraceCode)
         emitComment("-> assign");
      /* generate code for rhs */
      // finally the result is in reg[ac]
      code_generate_inner(tree->child[0]);
      /* now store value */
      SymInfo_ret ret = sym_lookup(symtabs[current_func], tree->str);
      // relative loc
      loc = ret.loc;
      if (ret.type == Integer)
      {
         // This is an Integer symbol
         // just get the exact loc(ac1) = #func + loc
         emitRM("LDA", ac1, loc, func, "loading exact address");

         // mem(reg[gp] + ac1) = ac
         emitRM("ST", ac, ac1, gp, "assign: store value");
      }
      else{
         emitBackup_reg(ac);
         // if it is assign to a array element
         // exact loc is in `loc` in ac1
         emitRM("LD", ac1, loc, gp, "loading exact address of array stored in `loc`");
         // load offset

         // mem(reg[gp] + ac1 + offset) = ac
         // ac1 = ac1 + offset
         emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
         emitRestore_reg(ac);
         // store #ac to #gp + ac1
         emitRM("ST", ac, ac1, gp, "assign: store value");
      }
      
      if (TraceCode)
         emitComment("<- assign");
      break; /* assign_k */

   case ReadK:
      emitRO("IN", ac, 0, 0, "read integer value");
      loc = st_lookup(tree->attr.name);
      emitRM("ST", ac, loc, gp, "read: store value");
      break;
   case WriteK:
      /* generate code for expression to write */
      code_generate_inner(tree->child[0]);
      /* now output it */
      emitRO("OUT", ac, 0, 0, "write ac");
      break;
   default:
      break;
   }
}

void generate_exp(TreeNode *tree)
{
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
   {
      if (TraceCode)
         emitComment("-> Id");
      // loc = st_lookup(tree->attr.name);
      // loc is relative location
      SymInfo_ret ret = sym_lookup(symtabs[current_func], tree->str);
      loc = ret.loc;

      if (ret.type == Integer)
      {
         // it is just totally a integer
         // get the exact location = reg[func] + loc
         emitRM("LD", ac, loc, func, "load id value");
      }
      else
      {
         // two case:
         // 1. refer as integer
         // 2. refer the whole array
         if (tree->child[0] == nullptr)
         {
            // refer the whole array
            // load its location
            printf("CGEN ERROR, get a array variable.");
            exit(1);
         }
         else
         {
            // get the offset of the element in the array
            // store in ac
            code_generate_inner(tree->child[0]);

            // get the exact location of the array
            // in the first element
            emitRM("LDA", ac1, loc, func, "loading exact location of array");
            emitRO("ADD", ac, ac, ac1, "getting the exact element location");
            emitRO("LD", ac, ac, gp, "loading the element content");
         }
      }

      if (TraceCode)
         emitComment("<- Id");
      break; /* IdK */
   }

   case Token_plus:
   case Token_minus:
   case Token_multiply:
   case Token_divide:
   case Token_less:
   case Token_lessEqual:
   case Token_more:
   case Token_moreEqual:
   case Token_equal:
   case Token_noEqual:
      if (TraceCode)
         emitComment("-> Op");
      p1 = tree->child[0];
      p2 = tree->child[1];
      /* gen code for ac = left arg */
      code_generate(p1);
      /* gen code to push left operand */
      // use a temp reg to store thing
      emitRM("ST", ac, tmpOffset--, mp, "op: push left");
      /* gen code for ac = right operand */
      code_generate(p2);
      /* now load left operand */
      // What is the purpose of this statement
      emitRM("LD", ac1, ++tmpOffset, mp, "op: load left");

      // all of them manipulate ac
      switch ((yytokentype)tree->token)
      {
      case Token_plus:
         emitRO("ADD", ac, ac1, ac, "op +");
         break;
      case Token_minus:
         emitRO("SUB", ac, ac1, ac, "op -");
         break;
      case Token_multiply:
         emitRO("MUL", ac, ac1, ac, "op *");
         break;
      case Token_divide:
         emitRO("DIV", ac, ac1, ac, "op /");
         break;
      case Token_less:
         emitRO("SUB", ac, ac1, ac, "op <");
         emitRM("JLT", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case Token_more:
         emitRO("SUB", ac, ac1, ac, "op >");
         emitRM("JGT", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case Token_lessEqual:
         emitRO("SUB", ac, ac1, ac, "op <=");
         emitRM("JLE", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case Token_moreEqual:
         emitRO("SUB", ac, ac1, ac, "op >=");
         emitRM("JGE", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case Token_equal:
         emitRO("SUB", ac, ac1, ac, "op ==");
         emitRM("JEQ", ac, 2, pc, "br if true");
         emitRM("LDC", ac, 0, ac, "false case");
         emitRM("LDA", pc, 1, pc, "unconditional jmp");
         emitRM("LDC", ac, 1, ac, "true case");
         break;
      case Token_noEqual:
         emitRO("SUB", ac, ac1, ac, "op !=");
         emitRM("JNE", ac, 2, pc, "br if true");
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