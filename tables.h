#ifndef __tables
#define __tables

#include <string>
#include <vector>
#include <map>

using std::string;
using std::vector;
using std::map;

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
void sym_insert(SymTab* s, string name, int lineno, int loc, int count, SymType type);

// symbol table look up
SymInfo_ret sym_lookup(SymTab* s, string name);

// extern "C"{
//     SymTab* init_symtab_c();
//     void sym_insert_c(SymTab* s, char* name, int lineno, int loc, int count, SymType type);
//     SymInfo_ret sym_lookup_c(SymTab* s, char* name);
// }

#endif