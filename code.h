/****************************************************/
/* File: code.h                                     */
/* Code emitting utilities for the TINY compiler    */
/* and interface to the TM machine                  */
/* Compiler Construction: Principles and Practice   */
/* Kenneth C. Louden                                */
/****************************************************/

#ifndef _CODE_H_
#define _CODE_H_

/* pc = program counter  */
#define pc 7

/* mp = "memory pointer" points
 * to top of memory (for temp storage)
 */
// #define mp 6

/* gp = "global pointer" points
 * to bottom of memory for (global)
 * variable storage
 */
#define gp 5

// used to record the start memory location 
//  of the current function
#define func 2

// used to record the bound of the current function
#define top 3

// used to record the offset of the stack
#define offset_mp 4

// temp register
#define tmp 6

/* accumulator */
#define ac 0

/* 2nd accumulator */
#define ac1 1

#include <string>
using std::string;

const string regs[8] = {
    "ac", "ac1", "func", "top", "offset_mp", "gp", "tmp", "pc"
};

/* TM location number for **current instruction** emission */
extern int emitLoc;

/* **Highest TM location emitted so far**
   For use in conjunction with emitSkip,
   emitBackup, and emitRestore */
extern int highEmitLoc;

/* code emitting utilities */

void emitComment_appendstr(char* ch, string s);

/* Procedure emitComment prints a comment line 
 * with comment c in the code file
 */
void emitComment(const char *c);

/* Procedure emitRO emits a register-only
 * TM instruction
 * op = the opcode
 * r = target register
 * s = 1st source register
 * t = 2nd source register
 * c = a comment to be printed if TraceCode is TRUE
 */
// register only is the same as RR
void emitRO(char *op, int r, int s, int t, char *c);

/* Procedure emitRM emits a register-to-memory
 * TM instruction
 * op = the opcode
 * r = target register
 * d = the offset
 * s = the base register
 * c = a comment to be printed if TraceCode is TRUE
 */
// probably this is the RM and RA
void emitRM(char *op, int r, int d, int s, char *c);

/* Function emitSkip skips "howMany" code
 * locations for later backpatch. It also
 * returns the current code position
 */
int emitSkip(int howMany);

/* Procedure emitBackup backs up to 
 * loc = a previously skipped location
 */
void emitBackup(int loc);

/* Procedure emitRestore restores the current 
 * code position to the highest previously
 * unemitted position
 */
void emitRestore(void);

void emitPush(int loc);
void emitPop(int loc);

/* Procedure emitRM_Abs converts an absolute reference 
 * to a pc-relative reference when emitting a
 * register-to-memory TM instruction
 * op = the opcode
 * r = target register
 * a = the absolute location in memory
 * c = a comment to be printed if TraceCode is TRUE
 */
void emitRM_Abs(char *op, int r, int a, char *c);

void loadAC_exactloc_Func(int);

void inc_reg(int);

void dec_reg(int);

#endif