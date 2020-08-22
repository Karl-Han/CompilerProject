#ifndef __tables
#define __tables

#include <string>
#include <vector>
#include <map>
#include <cstdio>

using std::string;
using std::vector;
using std::map;

static int memloc = 0;

enum SymType{
    Integer,
    Array,
    Void,
};

typedef struct _SymInfo
{
    string name;
    int memloc;
    int length;
    vector<int> refer_line;
    enum SymType type;
}SymInfo;

typedef struct _SymInfo_ret{
    int loc;
    SymType type;
} SymInfo_ret;

typedef struct _SymTab{
    map<string, SymInfo*> m;
}SymTab;

SymTab* init_symtab();

// insert info to symbol table
// if not present, insert symbol
// if present, insert reference
void sym_insert(SymTab* s, string name, int lineno, int count, SymType type);

// symbol table look up
SymInfo_ret sym_lookup(SymTab* s, string name);
void print_symtab(SymTab *table, FILE *listing);

// extern "C"{
//     SymTab* init_symtab_c();
//     void sym_insert_c(SymTab* s, char* name, int lineno, int loc, int count, SymType type);
//     SymInfo_ret sym_lookup_c(SymTab* s, char* name);
// }

typedef struct {
    string func_name;
    // 1 for void, 2 for int
    // 0 is default int
    // Not adding to practical usage: 3 for array 
    int ret_type;
    // 1 for int, 2 for array
    map<string, int>* para_list;
} FuncTab;

FuncTab* init_functab(string name, map<string, int>* para, int type);

int get_ret_type_functab(FuncTab* ft);

map<string, int>* get_paras_functab(FuncTab* ft);

void print_functab(FuncTab* ft, FILE* f);

#endif