#include "cgen.h"
#include "code.h"
#include "analyze.h"

extern "C"
{
#include "y.tab.h"
}

extern bool TraceCode;

// vector<string> func_stack;
string current_func;
const static string main_str = "main";
// static int tmpOffset = 0;
static int prelude2main;
static int main_ret_loc;

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

int prelude_global()
{
   // find out the size of global
   // #func = global_size
   int global_size = func2memloc["global"];
   emitRM("LDC", func, global_size, 0, "loading: #func starts from global size");

   // initialize all the array variable
   string s = "global";
   current_func = s;
   SymTab *st = (*symtabs)[s];
   map<string, SymInfo *> *m = st->m;
   for (auto i = m->begin(); i != m->end(); i++)
   {
      // for all the symbols
      SymInfo *si = i->second;

      // only deal with array
      if (si->type == Array)
      {
         // the exact location
         int memloc = si->memloc;
         // memory layout:
         // [0]:exact location, [1..]: elements
         // so the start location is memloc +1
         // mem(memloc) = memloc +1
         emitRM("LDC", tmp, memloc + 1, 0, "loading: loading global array's location");
         emitRM("ST", tmp, memloc, gp, "storing: storing global array's exact location");
      }
   }
   return global_size;
}

void prelude(TreeNode *t)
{
   // all the comments are start with `*`
   emitComment("MiniC Compilation to TM Code");

   // initialize offset and mp
   emitComment("Standard prelude:");
   emitRM("LD", offset_mp, 0, ac, "load offset from location 0");
   // emitRM("LDC", offset_mp, 1023, ac, "load offset from location 0");
   // emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
   emitRM("ST", ac, 0, ac, "clear location 0");

   // initialize gp
   emitRM("LDC", gp, 0, 0, "loading: initialize gp to 0");

   // allocate memory location for global variable
   // #func = global_size
   int global_size = prelude_global();

   // initilize top
   auto main_pt = functabs->find(main_str);
   if (main_pt == functabs->end())
   {
      // no main function
      printf("No main function, please check the file.\n");
      exit(1);
   }
   FuncTab *main = main_pt->second;

   int main_size = main->table_size;
   emitRM("LDC", top, global_size + main_size, 0, "loading: initialize top to main's size");

   // this emitLoc should be wait until main appear
   // int main_startloc = main->vmcode_startpos;
   // emitRM("LDC", pc, main_startloc, 0, "loading: initialize pc to start loc");
   emitComment("Skip for jumping to main");
   prelude2main = emitSkip(1);

   emitComment("End of standard prelude.");
}

void code_generate(TreeNode *tree)
{
   // prelude for initialization
   prelude(tree);

   /* generate code for TINY program */
   code_generate_inner(tree);

   // finish for main's return
   emitComment("End of execution. And dst of main's return");
   int currentLoc = emitSkip(0);
   emitBackup(main_ret_loc);
   emitRM("LDC", pc, currentLoc, 0, "loading: loading halt loc to pc for main's return");
   emitRestore();

   emitRO("HALT", 0, 0, 0, "");
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
      if (TraceCode)
      {
         string str = string("-> read");
         emitComment(str.c_str());
      }

      // #ac = value
      emitRO("IN", ac, 0, 0, "read integer value");

      SymInfo_ret ret = sym_lookup((*symtabs)[current_func], tree->str);
      if (ret.loc != -1 && ret.type != Void)
      {
         // relative loc and it is local variable
         loc = ret.loc;
         if (ret.type == Integer)
         {
            // This is an Integer symbol
            // just get the exact loc(ac1) = #func + loc
            emitRM("LDA", ac1, loc, func, "loading exact address");

            // mem(#ac1) = ac
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
         else
         {
            // This is an Array
            emitPush(ac);
            // if it is assign to a array element
            // exact loc is in `loc` in ac1
            emitRM("LD", ac1, loc, func, "loading exact address of array stored in `loc`");
            // load offset
            code_generate_inner(tree->child[0]);

            // mem(ac1 + offset) = ac
            // ac1 = ac1 + offset(in ac)
            emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
            emitPop(ac);
            // store #ac to ac1
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
      }
      else
      {
         // it is a global variable
         string global_str = "global";
         SymInfo_ret ret_global = sym_lookup((*symtabs)[global_str], tree->str);
         loc = ret_global.loc;
         if (ret_global.type == Integer)
         {
            // This is an Integer symbol
            // just get the exact loc(ac1) = #gp + loc
            emitRM("LDA", ac1, loc, gp, "loading exact address");

            // mem(reg[gp] + ac1) = ac
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
         else
         {
            // This is an Array
            emitPush(ac);
            // if it is assign to a array element
            // exact loc is in `loc` in ac1
            emitRM("LD", ac1, loc, gp, "loading exact address of array stored in `loc`");
            // load offset
            code_generate_inner(tree->child[0]);

            // mem(reg[gp] + ac1 + offset) = ac
            // ac1 = ac1 + offset(in ac)
            emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
            emitPop(ac);
            // store #ac to #gp + ac1
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
      }

      if (TraceCode)
      {
         string str = string("<- read");
         emitComment(str.c_str());
      }
      break;
   }
   case Token_write:
   {
      if (TraceCode)
      {
         string str = string("-> write");
         emitComment(str.c_str());
      }

      /* generate code for expression to write */
      code_generate_inner(tree->child[0]);
      /* now output it */
      emitRO("OUT", ac, 0, 0, "write ac");

      if (TraceCode)
      {
         string str = string("<- write");
         emitComment(str.c_str());
      }
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
      if (TraceCode)
      {
         emitComment_appendstr("-> func: ", tree->str);
      }

      // if is main, then it needs to emit prelude's jump
      if (tree->str != main_str)
      {
         // set the environment
         current_func = tree->str;
         FuncTab *ft = (*functabs)[tree->str];
         // get the start position of the function
         ft->vmcode_startpos = emitSkip(0);

         int paras_num = ft->para_type_list->size();

         if (paras_num != 0)
         {
            // deal with the parameters
            // no need for dynamic generation
            // #ac = #offset + 4 + paras_num -1
            // ac stores location of the first para
            emitPush(offset_mp);
            emitPop(ac);
            emitRM("LDC", ac1, 4, 0, "loading: loading 4 to ac1");
            emitRO("ADD", ac, ac, ac1, "adding: ac = offset + 4");
            emitRM("LDC", ac1, paras_num - 1, 0, "loading: loading paras_num -1 to ac1");
            emitRO("ADD", ac, ac, ac1, "adding: ac = offset + 4 + size -1");
            emitComment("ac now points to the first argument");

            // loading the parameters to function memory location
            for (size_t i = 0; i < paras_num; i++)
            {
               // #ac1 = mem(#ac - i)
               emitRM("LD", ac1, -i, ac, "loading: #ac1 = mem(#ac -i)");
               // mem(#func + i) = #ac
               emitRM("ST", ac1, i, func, "storing: mem(#func + i) = #ac");
            }
         }
      }
      else
      {
         current_func = main_str;
         // if the function is main
         // no argument pass but prelude need to jump here
         currentLoc = emitSkip(0);
         emitBackup(prelude2main);
         emitRM("LDC", pc, currentLoc, 0, "loading: load main location to prelude's PC");
         emitRestore();
      }
      // // initialize arrays before use
      // string func_str = tree->str;
      // SymTab* st = (*symtabs)[func_str];
      // for (auto i = st->m->begin(); i != st->m->end(); i++)
      // {
      //    // for all string -> SymInfo
      //    string sym_name = i->first;
      //    SymInfo* si = i->second;
      //    if (si->type == Array)
      //    {
      //       // fill the first allocated place
      //    }
      //
      // }

      // generate compound_st
      code_generate_inner(tree->child[3]);

      // ToDo: fix up the rest of return
      TreeNode *tn_tmp = new TreeNode();
      return_stmt(tn_tmp);
      free(tn_tmp);

      if (TraceCode)
      {
         emitComment_appendstr("<- func: ", tree->str);
      }

      // restore the function environment to global
      current_func = "global";
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
      // p1 is predicate, p2 is body
      p1 = tree->child[0];
      p2 = tree->child[1];

      // loc1 is for predicating
      savedLoc1 = emitSkip(0);

      // predicate
      code_generate_inner(p1);

      // loc2 is for further jump out of while
      savedLoc2 = emitSkip(1);

      code_generate_inner(p2);
      // finish body then jump back to p1
      emitRM("LDC", pc, savedLoc1, 0, "loading: load while predicate location to pc");

      // jump out
      currentLoc = emitSkip(0);
      emitBackup(savedLoc2);
      emitRM("JEQ", ac, currentLoc, 0, "jumping: jump to outside if #ac == 0");
      emitRestore();

      if (TraceCode)
         emitComment("<- while");
      break;
   }
   case Token_var_dec:
   {
      string global_str = "global";
      if (current_func == global_str)
      {
         break;
      }

      if (TraceCode)
         emitComment("-> var dec");
      // for Integer, just go pass
      // initialize all the array variable

      SymTab *st = (*symtabs)[current_func];
      // for .str
      string symbol_str = tree->child[1]->str;
      SymInfo *si = st->m->find(symbol_str)->second;

      // only deal with array
      if (si->type == Array)
      {
         // the relative location
         int memloc = si->memloc;
         // memory layout:
         // [0]:exact location, [1..]: elements
         // so the start location is memloc +1
         // mem(memloc) = #func + memloc +1
         emitRM("LDC", tmp, memloc + 1, 0, "loading: loading func array's relative location");
         emitRO("ADD", tmp, tmp, func, "adding: getting the array's exact location");
         emitRM("ST", tmp, memloc, func, "storing: storing global array's exact location");
      }
      if (TraceCode)
         emitComment("<- var dec");
      break;
   }
   case Token_assign:
   {
      if (TraceCode)
         emitComment("-> assign");

      /* generate code for rhs */
      // finally the result is in reg[ac]
      code_generate_inner(tree->child[1]);

      /* now store value */
      SymInfo_ret ret = sym_lookup((*symtabs)[current_func], tree->str);
      if (ret.loc != -1 && ret.type != Void)
      {
         // relative loc and it is local variable
         loc = ret.loc;
         if (ret.type == Integer)
         {
            // This is an Integer symbol
            // just get the exact loc(ac1) = #func + loc
            emitRM("LDA", ac1, loc, func, "loading exact address");

            // mem(#ac1) = ac
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
         else
         {
            // This is an Array
            emitPush(ac);
            // if it is assign to a array element
            // exact loc is in `loc` in ac1
            emitRM("LD", ac1, loc, func, "loading exact address of array stored in `loc`");
            // load offset
            code_generate_inner(tree->child[0]);

            // mem(ac1 + offset) = ac
            // ac1 = ac1 + offset(in ac)
            emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
            emitPop(ac);
            // store #ac to ac1
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
      }
      else
      {
         // it is a global variable
         string global_str = "global";
         SymInfo_ret ret_global = sym_lookup((*symtabs)[global_str], tree->str);
         loc = ret_global.loc;
         if (ret_global.type == Integer)
         {
            // This is an Integer symbol
            // just get the exact loc(ac1) = #gp + loc
            emitRM("LDA", ac1, loc, gp, "loading exact address");

            // mem(reg[gp] + ac1) = ac
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
         else
         {
            // This is an Array
            emitPush(ac);
            // if it is assign to a array element
            // exact loc is in `loc` in ac1
            emitRM("LD", ac1, loc, gp, "loading exact address of array stored in `loc`");
            // load offset
            code_generate_inner(tree->child[0]);

            // mem(reg[gp] + ac1 + offset) = ac
            // ac1 = ac1 + offset(in ac)
            emitRO("ADD", ac1, ac, ac1, "calculate the address of the element");
            emitPop(ac);
            // store #ac to #gp + ac1
            emitRM("ST", ac, 0, ac1, "assign: store value");
         }
      }

      if (TraceCode)
         emitComment("<- assign");
      break; /* assign_k */
   }
   case Token_return:
   {
      if (TraceCode)
      {
         string str = string("-> return");
         emitComment(str.c_str());
      }

      if (current_func == main_str)
      {
         // just jump to the halt
         main_ret_loc = emitSkip(1);
         emitComment_appendstr("skipping: ", std::to_string(main_ret_loc));
      }
      else
      {
         // return things
         return_stmt(tree);
      }

      if (TraceCode)
      {
         string str = string("<- return");
         emitComment(str.c_str());
      }
      break;
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

   case Token_call:
   {
      if (TraceCode)
      {
         emitComment_appendstr("-> call: ", tree->str);
      }

      // step1: pass all the paramters by stack
      TreeNode *tn_tmp = tree->child[1];
      if (tn_tmp == nullptr || tn_tmp->token == Token_void)
      {
         // no parameter
      }
      else
      {
         // insert parameter
         while (tn_tmp != nullptr)
         {
            // push all parameters
            // code_generate_inner(tn_tmp);
            generate_exp(tn_tmp);
            emitPush(ac);
            tn_tmp = tn_tmp->sibling;
         }
      }

      // step2: jump to the target function
      // push registers: PC, func, paras
      int paras_num = (*functabs)[tree->str]->para_type_list->size();
      // PC can not be saved in this way
      //  because it will store next instruction
      //  instead of going back
      // emitPush(pc);
      // skip 4 instruction for store current_loc and push it
      int saved_loc4pc = emitSkip(4);

      emitPush(func);
      // ALERT: it is really necessary?
      // you are pushing the ac instead of paras_num
      emitRM("LDC", tmp, paras_num, 0, "loading: load paras_num to tmp");
      emitPush(tmp);

      // set registers: func, top, PC
      emitPush(top);
      emitPop(func);
      emitRM("LDC", tmp, (*functabs)[tree->str]->table_size, 0, "loading: load the size of function table");
      emitRO("ADD", top, top, tmp, "adding: top = old top + table size");
      emitRM("LDC", pc, (*functabs)[tree->str]->vmcode_startpos, 0, "loading: const start pos to PC");

      // jump back to here after function
      // generate pushing the PC
      int current_loc = emitSkip(0);
      emitBackup(saved_loc4pc);
      emitRM("LDC", ac, current_loc, 0, "loading: current location for jumping back.");
      emitPush(ac);
      // restore current instruction location
      emitRestore();

      // clean up the stack for other instructions
      for (size_t i = 0; i < paras_num + 3; i++)
      {
         emitPop(ac1);
      }

      if (TraceCode)
      {
         emitComment_appendstr("<- call: ", tree->str);
      }
      break;
   }
   case Token_var:
   {
      if (TraceCode)
         emitComment("-> Id");

      SymInfo_ret ret = sym_lookup((*symtabs)[current_func], tree->str);
      if (ret.loc != -1 && ret.type != Void)
      {
         // this is a local variable
         loc = ret.loc;
         if (ret.type == Integer)
         {
            // it is just totally a integer
            // get the exact location = reg[func] + loc
            // #ac = mem(reg[func] + loc)
            emitRM("LD", ac, loc, func, "load id value");
         }
         else
         {
            // two case:
            // 1. refer as integer
            // 2. refer the whole array, load its exact location
            if (tree->child[0] == nullptr)
            {
               // refer the whole array
               // load its location
               emitRM("LD", ac, loc, func, "loading: array's exact location");
            }
            else
            {
               // get the offset of the element in the array
               // store in ac
               code_generate_inner(tree->child[0]);

               // get the exact location of the array
               // in the first element
               emitRM("LD", ac1, loc, func, "loading exact location of array, stored in mem(#func + loc");
               emitRO("ADD", ac, ac, ac1, "getting the exact element location");
               emitRO("LD", ac, 0, ac, "loading the element content");
            }
         }
         // code_generate_inner(tree->sibling);
      }
      else
      {
         // this is a global variable or not exist
         string global_str = "global";
         SymInfo_ret ret_global = sym_lookup((*symtabs)[global_str], tree->str);
         if (ret_global.loc == -1 && ret_global.type == Void)
         {
            // no such symbol, but it is impossible
            printf("No such symbol in both current function or global environment.\n");
            seg_fault();
         }
         else
         {
            // found in global variable
            loc = ret_global.loc;
            if (ret_global.type == Integer)
            {
               // it is just totally a integer
               // get the exact location = loc
               // #ac = mem(0 + loc)
               emitRM("LD", ac, loc, gp, "loading: load id value from global");
            }
            else
            {
               // two case:
               // 1. refer as integer
               // 2. refer the whole array, load its exact location
               if (tree->child[0] == nullptr)
               {
                  // refer the whole array
                  // load its location
                  emitRM("LD", ac, loc, gp, "loading: array's exact location");
               }
               else
               {
                  // get the offset of the element in the array
                  // store in ac
                  code_generate_inner(tree->child[0]);

                  // get the exact location of the array
                  // in the first element
                  emitRM("LD", ac1, loc, gp, "loading exact location of array");
                  emitRO("ADD", ac, ac, ac1, "getting the exact element location");
                  emitRO("LD", ac, 0, ac, "loading the element content");
               }
            }
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