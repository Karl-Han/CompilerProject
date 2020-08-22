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
void sym_insert(SymTab* t, string name, int lineno, int count, SymType type){
    if (count == 0 && type != Array)
    {
        return ;
    }
    
    auto iter = t->m.find(name);
    if (iter == t->m.end())
    {
        // no such symbol
        // insert new symbol
        SymInfo* si = init_syminfo(name, memloc, count, type);
        memloc++;
        si->refer_line.push_back(lineno);
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

void print_symtab(SymTab *table, FILE *listing)
{
    fprintf(listing, "Variable Name  Location   Type    Line Numbers\n");
    fprintf(listing, "-------------  --------  ------- ------------\n");
    for (auto i = table->m.begin(); i != table->m.end(); i++)
    {
        // for all string -> SymInfo*
        SymInfo *si = i->second;
        fprintf(listing, "%-14s ", si->name.c_str());
        fprintf(listing, "%-8d  ", si->memloc);
        if (si->type == Integer)
        {
            // this is an integer
            fprintf(listing, "Integer ");
        }
        else{
            fprintf(listing, " Array ");
        }
        
        for (auto j = si->refer_line.begin(); j != si->refer_line.end(); j++)
        {
            fprintf(listing, "%4d ", *j);
        }
        fprintf(listing, "\n");
    }
}

FuncTab* init_functab(std::string name, map<string, int>* para, int type){
    FuncTab* ft = new FuncTab();
    ft->func_name = name;
    ft->para_list = para;
    ft->ret_type = type;

    return ft;
}

int get_ret_type_functab(FuncTab* ft){
    return ft->ret_type;
}

map<string, int>* get_paras_functab(FuncTab* ft){
    map<string, int>* m = new map<string,int>(*ft->para_list);
    return m;
}

void print_functab(FuncTab* ft, FILE* f){
    fprintf(f, "%-14s ", ft->func_name.c_str());
    switch (ft->ret_type)
    {
    case 1:
        // No return value
        fprintf(f, " %-8s ", "Void");
        break;
    case 0:
    case 2:
        // Return Integer
        fprintf(f, " %-8s ", "Integer");
        break;
    
    default:
        printf("No such return type\nExiting");
        exit(1);
        break;
    }
    for (auto &i : *ft->para_list)
    {
        string s;
        // 1 for int, 2 for array
        switch (i.second)
        {
        case 1:
            s = "Integer";
            break;
        case 2:
            s = "Array";
            break;
        default:
            printf("No such parameter type\nExiting");
            break;
        }
        fprintf(f, " %s(%s) ", s.c_str(), i.first.c_str());
    }
    
}