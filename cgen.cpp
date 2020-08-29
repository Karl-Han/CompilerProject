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
      code_generate_inner(tree->sibling);
   }
}

void code_generate(TreeNode *tree)
{
   // all the comments are start with `*`
   emitComment("TINY Compilation to TM Code");

   /* generate standard prelude */
   emitComment("Standard prelude:");
   emitRM("LD", offset_mp, 0, ac, "load maxaddress from location 0");
   emitRM("ST", ac, 0, ac, "clear location 0");
   emitComment("End of standard prelude.");

   /* generate code for TINY program */
   code_generate_inner(tree);

   /* finish */
   emitComment("End of execution.");
   emitRO("HALT", 0, 0, 0, "");
   // start with the function main
}

// generate fix up of the function
// offset_mp now points to the next element
// of the parameter number
void return_stmt(TreeNode *t)
{
   // if exp return some value, it will be in ac
   if (t->child[0] != nullptr)
   {
      code_generate_inner(t->child[0]);
   }

   // recover registers
   // #top = #func
   emitPush(func);
   emitPop(top);
   // #func = mem(#offset +2)
   emitRM("LD", func, 2, offset_mp, "loading: restoring old #func");
   // #pc= mem(#offset +3)
   emitRM("LD", pc, 3, offset_mp, "loading: restoring old #pc");
}

void generate_stmt(TreeNode *tree)
{
   TreeNode *p1, *p2, *p3;
   // this is local variable
   int savedLoc1, savedLoc2, currentLoc;
   int loc;
   switch ((yytokentype)tree->token)
   {
   case Token_read:
   {
      // ac <- value
      emitRO("IN", ac, 0, 0, "read integer value");
      // loc = st_lookup(tree->attr.name);
      SymInfo_ret ret = sym_lookup(symtabs[current_func], tree->str);
      emitPush(ac);
      loadAC_exactloc_Func(ret.loc);

      // pop the input to #ac1
      emitPop(ac1);
      emitRM("ST", ac1, 0, ac, "read: store value");
      break;
   }
   case Token_write:
   {
      /* generate code for expression to write */
      code_generate_inner(tree->child[0]);
      /* now output it */
      emitRO("OUT", ac, 0, 0, "write ac");
      break;
   }
   case Token_compound:
   {
      code_generate_inner(tree->child[0]);
      code_generate_inner(tree->child[1]);
      break;
   }
   case Token_func:
   {
      if (tree->str != "main")
      {
         // get the start position of the function
         functabs[tree->str]->vmcode_startpos = emitSkip(0);

         // in generating order
         // initialization
         // ac <- param + 3
         emitRM("LDC", ac, 3, offset_mp, "loading: ac <- #offset + 3");
         // param = mem(#offset +1)
         emitRM("LD", ac1, 1, offset_mp, "loading: para num to ac1");
         emitPush(ac);
         emitPop(tmp);
         emitRO("ADD", ac, ac, ac1, "adding: ac = ac + params");
         // ac1 <- 0
         emitRM("LDC", ac1, 0, 0, "loading: ac1 <- 0");

         // loc1
         // saved for future restore
         savedLoc1 = emitSkip(0);
         // loop predicate
         // stop when ac == tmp
         // #tmp = #offset_mp + 3
         emitRM("LDA", tmp, 3, offset_mp, "loading: next ele after last ele of func");
         emitRO("SUB", tmp, ac, tmp, "subtracting: tmp <- ac - param");

         savedLoc2 = emitSkip(1);

         // loc2+1
         // body
         // LD  tmp, ac, gp
         emitRM("LD", tmp, ac, gp, "loading: get parameter");
         // ST  tmp, ac1, func
         emitRM("ST", tmp, ac1, func, "storing: storing first parameter");
         // increase ac1 by 1
         inc_reg(ac1);
         // decrease ac by 1
         // SUB ac, ac, 1
         dec_reg(ac);
         // LDC pc, loc2
         emitRM("LDC", pc, savedLoc1, gp, "jumping: unconditionally jump to savedLoc1");

         // loc2:
         // JLE tmp, loc3, gp       // jump out
         currentLoc = emitSkip(0);
         emitBackup(savedLoc2);
         emitRM("JLE", tmp, currentLoc, gp, "jumping: to currentLoc if finished");
         emitRestore();

         // loc3:
         // generate compound_st
         code_generate_inner(tree->child[3]);

         // ToDo: fix up the rest of return
         TreeNode *tn_tmp = new TreeNode();
         return_stmt(tn_tmp);
         free(tn_tmp);
      }

      break;
   }
   case Token_if:
   {
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
   }
   case Token_while:
   {
      if (TraceCode)
         emitComment("-> while");
      p1 = tree->child[0];
      p2 = tree->child[1];
      // savedLoc1 is the start of all repeat instruction
      savedLoc1 = emitSkip(1);
      emitComment("while: jump after body comes back here");

      /* generate code for body */
      code_generate_inner(p1);

      // here to know where the predicate is
      currentLoc = emitSkip(0);
      emitBackup(savedLoc1);
      // load next pc directly
      emitRM("LDC", pc, currentLoc, 0, "while: jump to predicate");
      emitRestore();

      /* generate code for test */
      code_generate_inner(p2);
      // reg[ac] = 0
      emitRM_Abs("JEQ", ac, savedLoc1 + 1, "repeat: jmp back to body");
      if (TraceCode)
         emitComment("<- while");
      break;
   }

   case Token_assign:
   {
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
      else
      {
         emitPush(ac);
         // if it is assign to a array element
         // exact loc is in `loc` in ac1
         emitRM("LD", ac1, loc, gp, "loading exact address of array stored in `loc`");
         // load offset

         // mem(reg[gp] + ac1 + offset) = ac
         // ac1 = ac1 + offset
         emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
         emitPop(ac);
         // store #ac to #gp + ac1
         emitRM("ST", ac, ac1, gp, "assign: store value");
      }

      if (TraceCode)
         emitComment("<- assign");
      break; /* assign_k */
   }
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
   {
      if (TraceCode)
         emitComment("-> Op");
      p1 = tree->child[0];
      p2 = tree->child[1];
      /* gen code for ac = left arg */
      code_generate_inner(p1);
      /* gen code to push left operand */
      // use a temp mem
      emitPush(ac);
      /* gen code for ac = right operand */
      // the stack should be balanced
      code_generate_inner(p2);
      /* now load left operand */
      // pop the temp value
      emitPop(ac1);

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
   }

   case Token_call:{
      TreeNode* tn_tmp = tree->child[1];
      if (tn_tmp == nullptr || tn_tmp->token == Token_void)
      {
         // no parameter
      }
      else{
         // insert parameter
         while (tn_tmp != nullptr)
         {
            // push all parameters
            code_generate_inner(tn_tmp);
            emitPush(ac);
         }
         
         // push registers: PC, func, paras
         int paras_num = functabs[tree->str]->para_type_list->size();
         emitPush(pc);
         emitPush(func);
         emitPush(paras_num);

         // set registers: func, top, PC
         emitPush(top);
         emitPop(func);
         emitRM("LDC", tmp, functabs[tree->str]->table_size, 0, "loading: load the size of function table");
         emitRO("ADD", top, top, tmp, "adding: top = old top + table size");
         emitRM("LDC", pc, functabs[tree->str]->vmcode_startpos, 0, "loading: const start pos to PC");

         // jump back to here after function
         // clean up the stack for other instructions
         for (size_t i = 0; i < paras_num +3; i++)
         {
            emitPop(ac1);
         }
      }
      
      break;
   }

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
   case Token_number:
   {
      if (TraceCode)
         emitComment("-> Const");
      /* gen code to load integer constant using LDC */
      emitRM("LDC", ac, tree->num, 0, "load const");
      if (TraceCode)
         emitComment("<- Const");
      break; /* ConstK */
   }

   default:
      break;
   }
}