#include "analyze.h"
#include "tables.h"

extern "C"
{
#include "y.tab.h"
}

#define MAXCHILDREN 4

string current_env;
map<string, SymTab *> symtabs;
vector<string> func_stack;
map<string, FuncTab*> functabs;

map<string, int>* get_param(TreeNode* para){
    TreeNode* tmp = para;
    map<string, int>* m = new map<string, int>();
    if (para->token == (int)Token_void)
    {
        return m;
    }
    

    while (tmp != nullptr)
    {
        string s = tmp->str;
        (*m)[s] = tmp->num;
        tmp = tmp->sibling;
    }

    return m;
}

void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *))
{
    if (t != NULL)
    {
        if (t->token != (int)Token_func)
        {
            // this is not a function node
            preProc(t);
            for (int i = 0; i < MAXCHILDREN; i++)
            {
                traverse(t->child[i], preProc, postProc);
            }
            postProc(t);
            traverse(t->sibling, preProc, postProc);
        }
        else
        {
            // this is a fucntion node
            // set the env to the new func
            current_env = t->str;
            func_stack.push_back(current_env);
            if (symtabs.find(current_env) == symtabs.end())
            {
                // this is a new function
                // 1. create new symbol table for func
                // 2. insert into function table
                symtabs[current_env] = init_symtab();
                int type;
                switch (t->child[0]->token)
                {
                    case Token_int:{
                        // return type is int
                        type = 1;
                        break;
                    }
                    case Token_void:{
                        // return type is void
                        type = 2;
                        break;
                    }
                
                default:
                    type = 0;
                    break;
                }
                
                map<string, int> * param = get_param(t->child[2]);
                functabs[current_env] = init_functab(current_env, param, type);
            }
            

            for (int i = 0; i < MAXCHILDREN; i++)
            {
                traverse(t->child[i], preProc, postProc);
            }
            // restore the env
            func_stack.pop_back();
            current_env = func_stack.back();

            traverse(t->sibling, preProc, postProc);
        }
    }
}

// used to insert symbol info into symbol table
void insert_node(TreeNode *t)
{
    TreeNode *tmp = nullptr;
    int counter = 0;
    SymType type = Void;

    switch (t->token)
    {
    case Token_assign:
    {
        tmp = t->child[0];
        type = Integer;
        break;
    }
    case Token_var_dec:
    {
        // str
        tmp = t->child[1];

        // this is the place for number
        if (t->child[2] != 0)
        {
            // this is a array
            counter = t->child[2]->num;
            type = Array;
        }
        else
        {
            counter = 1;
            type = Integer;
        }
        if (t->child[0]->token != Token_int)
        {
            printf("Wrong type of variable `%s` type %s declararion.\nIn building symbol.", t->child[1]->str, tokens[t->child[0]->token - tokens_offset]);
            exit(1);
        }
        break;
    }
    case Token_para:
    {

        tmp = t;
        if (t->num == 2)
        {
            type = Array;
        }
        else{
            type = Integer;
            counter = 1;
        }
        break;
    }
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
        if (t->child[0]->token == (int)Token_identifier)
        {
            // deal with one operand
            tmp = t->child[0];
            counter = 1;
            type = Void;
        }
        if (t->child[1]->token == (int)Token_identifier)
        {
            sym_insert(symtabs[current_env], t->child[1]->str, t->child[1]->lineno, 1, Integer);
        }
        break;
    }
    case Token_identifier:
    case Token_var:
    {
        tmp = t;
        counter = 1;
        break;
    }
    case Token_func:
    {
        break;
    }

    default:
        // printf("No ID appears in %s\nExiting", tokens[t->token-tokens_offset]);
        // exit(1);
        break;
    }
    if (tmp != nullptr)
    {
        // do insert
        sym_insert(symtabs[current_env], tmp->str, tmp->lineno, counter, type);
    }
}

// void wrap_up(TreeNode *t)
// {
//     if (new_func)
//     {
//         // need to pop the env
//         func_stack.pop_back();
//         current_env = func_stack.back();
//     }
// }

void null_proc(TreeNode *t)
{
    return;
}

void build_symtabs(TreeNode *t)
{
    current_env = "global";
    symtabs = map<string, SymTab *>();
    // could be delete because no funcion recursion
    //  when building symbol table
    func_stack = vector<string>();
    // new_func = false;

    symtabs[current_env] = init_symtab();
    func_stack.push_back("global");

    // traverse(t, insert_node, wrap_up);
    traverse(t, insert_node, null_proc);

    assert(func_stack.back() == "global");
    printf("Pass build symbol tables.\n");
}

// void buildSymtabs_c(TreeNode *t)
// {
//     return build_symtabs(t);
// }

void print_symtabs(FILE* listing){
    for(auto m = symtabs.begin(); m != symtabs.end(); m++){
        string table_name = m->first;
        auto table = m->second;
        fprintf(listing, "%s\n", table_name.c_str());
        print_symtab(table, listing);
        fprintf(listing, "\n");
    }
}

void print_functabs(FILE* listing){
    fprintf(listing, "%-16s%-10s%s\n", "function", "return", "parameters");
    for(auto m = functabs.begin(); m != functabs.end(); m++){
        // string func_name = m->first;
        FuncTab* func_info = m->second;
        print_functab(func_info, listing);
        fprintf(listing, "\n");
    }
}