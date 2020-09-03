#include "tables.h"

map<string, int> func2memloc;

SymInfo* init_syminfo(string name, int loc, int length, SymType type){
    SymInfo* si = (SymInfo*)malloc(sizeof(SymInfo));

    si->name = name;
    si->memloc = loc;
    si->length = length;
    si->refer_line = new vector<int>();
    si->type = type;

    return si;
}

// initialize symbol table
SymTab* init_symtab(string name){
    SymTab* st = (SymTab*)malloc(sizeof(SymTab));
    st->name_table = name;
    st->m = new map<string, SymInfo*>();

    return st;
}

// insert info to symbol table
// if not present, insert symbol
// if present, insert reference
void sym_insert(SymTab* t, string name, int lineno, int count, SymType type){
    // no count && not a array
    // => invalid symbol
    if (count == 0 && type != Array)
    {
        return ;
    }

    if (func2memloc.find(t->name_table) == func2memloc.end())
    {
        // not yet allocate memloc for `name`
        func2memloc[t->name_table] = 0;
    }
    
    // find if the name is already appear in table
    auto iter = t->m->find(name);
    if (iter == t->m->end())
    {
        // no such symbol
        // insert new symbol
        SymInfo* si = init_syminfo(name, func2memloc[t->name_table], count, type);
        if (type == Array)
        {
            // one extra memory space for exact location
            func2memloc[t->name_table] += (count +1);
        }
        else{
            // other is just a integer
            func2memloc[t->name_table] += 1;
        }
        
        si->refer_line->push_back(lineno);
        (*t->m)[name] = si;
    }
    else{
        // find symbol
        // insert symbol reference
        // use second to manipulate it
        iter->second->refer_line->push_back(lineno);
    }
}

// symbol table look up
SymInfo_ret sym_lookup(SymTab* t, string name){
    auto iter = t->m->find(name);
    if (iter == t->m->end())
    {
        // no such symbol
        // return memloc = -1

        // SymInfo_ret* ret = new SymInfo_ret();
        // ret->loc = -1;
        // ret->type = Void;
        // return ret;
        return {.loc = -1,.type = SymType::Void};
    }
    else{
        // find symbol
        // return correct info
        // SymInfo_ret* ret = new SymInfo_ret();
        // ret->loc = iter->second->memloc;
        // ret->type = iter->second->type;
        // return ret;
        return {.loc = iter->second->memloc,.type = iter->second->type};
    }
}

void print_symtab(SymTab *table, FILE *listing)
{
    fprintf(listing, "Variable Name  Location   Type    Line Numbers\n");
    fprintf(listing, "-------------  --------  ------- ------------\n");
    for (auto i = table->m->begin(); i != table->m->end(); i++)
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
        
        for (auto j = si->refer_line->begin(); j != si->refer_line->end(); j++)
        {
            fprintf(listing, "%4d ", *j);
        }
        fprintf(listing, "\n");
    }
}

map<string, int>* init_para_type_map(vector<ParaInstant*>* v){
    map<string, int>* m = new map<string, int>();
    for (auto i = v->begin(); i != v->end(); i++)
    {
        // for all element
        string s =  (*i)->name;
        (*m)[s] = (*i)->type;
    }
    
    return m;
}

FuncTab* init_functab(std::string name, vector<ParaInstant*>* para_list, int type){
    FuncTab* ft = new FuncTab();
    ft->func_name = name;
    // ft->para_type_map = para;
    ft->para_type_list = para_list;
    ft->para_type_map = init_para_type_map(para_list);
    ft->ret_type = type;

    return ft;
}

void init_functab_size(FuncTab* ft){
    ft->table_size = func2memloc[ft->func_name];
}

int get_ret_type_functab(FuncTab* ft){
    return ft->ret_type;
}

map<string, int>* get_paras_map_functab(FuncTab* ft){
    map<string, int>* m = new map<string,int>(*ft->para_type_map);
    return m;
}

const vector<ParaInstant*>* get_paras_list_functab(FuncTab* ft){
    return ft->para_type_list;
}

void print_functab(FuncTab* ft, FILE* f){
    fprintf(f, "%-14s ", ft->func_name.c_str());

    // print return type
    switch (ft->ret_type)
    {
    // 1 for void, 2 for int
    // 0 is default int
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

    // print function variable occupation
    fprintf(f, " %-12d ", ft->table_size);

    // parameters
    for (auto &i : *ft->para_type_map)
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
            printf("No such parameter for `%s` with type `%s`\nExiting", s.c_str(), i.first.c_str());
            exit(1);
            break;
        }
        fprintf(f, " %s(%s) ", s.c_str(), i.first.c_str());
    }
}