/****************************************************/
/* File: code.c                                     */
/* TM Code emitting utilities                       */
/* implementation for the TINY compiler             */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#include "code.h"
#include <cstdio>

// #define TraceCode true

extern bool TraceCode;
extern FILE* code;

int emitLoc = 0;
int highEmitLoc = 0;

void emitComment_appendstr(char* ch, string s){
   string com = string(ch) + s;
   emitComment(com.c_str());
}

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment( const char * c )
{ if (TraceCode) fprintf(code,"* %s\n",c);}

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRO( char *op, int r, int s, int t, char *c)
{ fprintf(code,"%3d:  %5s  %d,%d,%d ",emitLoc++,op,r,s,t);
  if (TraceCode) fprintf(code,"\t%s",c) ;
  fprintf(code,"\n") ;
  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRO */

/* Procedure emitRM emits a register-to-memory
 * TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM( char * op, int r, int d, int s, char *c)
{ 
  fprintf(code,"%3d:  %5s  %d,%d(%d) ",emitLoc++,op,r,d,s);
  if (TraceCode) fprintf(code,"\t%s",c) ;
  fprintf(code,"\n") ;
  if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
} /* emitRM */

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
// skip location and return current code pos
int emitSkip( int howMany)
{  int i = emitLoc;
   emitLoc += howMany ;
   if (highEmitLoc < emitLoc)  highEmitLoc = emitLoc ;
   return i;
} /* emitSkip */

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup( int loc)
{ if (loc > highEmitLoc) emitComment("BUG in emitBackup");
  emitLoc = loc ;
} /* emitBackup */

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
// restore the latest instruction location
void emitRestore(void)
{ emitLoc = highEmitLoc;}

void emitPush(int reg){
  // push #reg to stack
  emitComment_appendstr("-> push: ", regs[reg]);
  // emitRM("ST", reg, offset_mp, gp, "store: temp value backuping");
  emitRM("ST", reg, 0, offset_mp, "store: temp value backuping");
  emitRM("LDC", tmp, 1, 0, "loading 1 to #tmp");
  emitRO("SUB", offset_mp, offset_mp, tmp, "stack going upwards");
  emitComment_appendstr("<- push:", regs[reg]);
}

void emitPop(int reg){
  // pop stack top to #reg
  emitComment_appendstr("-> pop: ", regs[reg]);
  emitRM("LDC", tmp, 1, 0, "loading 1 to #tmp");
  emitRO("ADD", offset_mp, offset_mp, tmp, "stack going downwards");
  emitRM("LD", reg, 0, offset_mp, "load: restoring temp value to #tmp");
  emitComment_appendstr("<- pop: ", regs[reg]);
}

/* Procedure emitRM_Abs
 * converts an absolute reference 
 * to a pc-relative reference 
 * when emitting a register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
// op r, a-(emitLoc+1) (pc)
// two operator: reg[r] and mem(a-(emitLoc+1) + reg[pc])
void emitRM_Abs( char *op, int r, int a, char * c)
{ fprintf(code,"%3d:  %5s  %d,%d(%d) ",
               emitLoc,op,r,a-(emitLoc+1),pc);
  ++emitLoc ;
  if (TraceCode) fprintf(code,"\t%s",c) ;
  fprintf(code,"\n") ;
  if (highEmitLoc < emitLoc) highEmitLoc = emitLoc ;
} /* emitRM_Abs */

// ac = #func + loc(ac1)
void loadAC_exactloc_Func(int loc){
  emitPush(ac1);

  emitRM("LDC", ac1, loc, 0, "loading loc to ac1");
  emitRO("ADD", ac, ac1, func_reg, "adding: #func + loc");

  emitPop(ac1);
}