#ifndef __tables
#define __tables

#include <string>
#include <vector>
#include <map>
#include <cstdio>

using std::string;
using std::vector;
using std::map;

extern map<string, int> func2memloc;

typedef enum {
    Void,
    Integer,
    Array,
}SymType;

typedef struct _SymInfo
{
    // symbol name
    string name;
    // relative memory location
    int memloc;
    // length of the symbol
    // for array is exact length +1
    int length;
    vector<int>* refer_line;
    SymType type;
}SymInfo;

typedef struct _SymInfo_ret{
    int loc;
    SymType type;
} SymInfo_ret;

typedef struct _SymTab{
    string name_table;
    map<string, SymInfo*>* m;
}SymTab;

SymTab* init_symtab(string);

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
    string name;
    // 1 for int, 2 for array
    int type;
} ParaInstant;

typedef struct {
    string func_name;
    // 1 for void, 2 for int
    // 0 is default int
    // Not adding to practical usage: 3 for array 
    int ret_type;
    // 1 for int, 2 for array
    map<string, int>* para_type_map;
    vector<ParaInstant*>* para_type_list;
    // the occupation size of the table
    int table_size;
    // TM code start pos
    int vmcode_startpos;
} FuncTab;

FuncTab* init_functab(std::string name, vector<ParaInstant*>* para_list, int type);

void init_functab_size(FuncTab*);

int get_ret_type_functab(FuncTab* ft);

map<string, int>* get_paras_map_functab(FuncTab* ft);
const vector<ParaInstant*>* get_paras_list_functab(FuncTab* ft);

void print_functab(FuncTab* ft, FILE* f);

#endif