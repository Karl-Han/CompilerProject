#include <map>
#include "tables.h"

SymInfo* init_syminfo(string name, int loc, int length, SymType type){
    SymInfo* si = (SymInfo*)malloc(sizeof(SymInfo));

    si->name = name;
    si->memloc = loc;
    si->length = length;
    si->refer_line = vector<int>();
    si->type = type;

    return si;
}

// initialize symbol table
SymTab* init_symtab(){
    SymTab* st = (SymTab*)malloc(sizeof(SymTab));
    st->m = map<string, SymInfo*>();

    return st;
}

// insert info to symbol table
// if not present, insert symbol
// if present, insert reference
void sym_insert(SymTab* t, string name, int lineno, int loc, int count, SymType type){
    if (count == 0 && type != Array)
    {
        return ;
    }
    
    auto iter = t->m.find(name);
    if (iter == t->m.end())
    {
        // no such symbol
        // insert new symbol
        SymInfo* si = init_syminfo(name, loc, count, type);
        memloc++;
        t->m[name] = si;
    }
    else{
        // find symbol
        // insert symbol reference
        // use second to manipulate it
        iter->second->refer_line.push_back(lineno);
    }
    
}

// symbol table look up
SymInfo_ret sym_lookup(SymTab* t, string name){
    auto iter = t->m.find(name);
    if (iter == t->m.end())
    {
        // no such symbol
        // return memloc = -1
        return {.loc = -1,.type = SymType::Void};
    }
    else{
        // find symbol
        // return correct info
        return {.loc = iter->second->memloc, .type = iter->second->type};
    }
}