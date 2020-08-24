#include "analyze.h"
#include "tables.h"

extern "C"
{
#include "y.tab.h"
}

#define MAXCHILDREN 4
static char* TYPE[] = {"VOID", "INTEGER", "ARRAY"};

string current_func;
map<string, SymTab *> symtabs;
vector<string> func_stack;
map<string, FuncTab*> functabs;

// get the parameter list
// invoke by functab
vector<ParaInstant*>* get_param(TreeNode* para){
    TreeNode* tmp = para;
    vector<ParaInstant*>* v = new vector<ParaInstant*>();
    if (para->token == (int)Token_void)
    {
        // no parameter
        return v;
    }
    

    while (tmp != nullptr)
    {
        // get the variable name
        ParaInstant* pi = new ParaInstant();
        pi->name = tmp->str;
        pi->type = tmp->num;

        v->push_back(pi);
        tmp = tmp->sibling;
    }

    return v;
}


// used to traverse the tree structure
// also change the current_function during the process
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
            current_func = t->str;
            func_stack.push_back(current_func);

            // init function table if no find
            if (symtabs.find(current_func) == symtabs.end())
            {
                // this is a new function
                // 1. create new symbol table for func
                // 2. insert into function table
                symtabs[current_func] = init_symtab();
                int type;
                switch (t->child[0]->token)
                {
                    // 1 for void, 2 for int
                    // 0 is default int
                    case Token_int:{
                        // return type is int
                        type = 2;
                        break;
                    }
                    case Token_void:{
                        // return type is void
                        type = 1;
                        break;
                    }
                
                default:
                    type = 0;
                    break;
                }
                
                vector<ParaInstant*>* param = get_param(t->child[2]);
                functabs[current_func] = init_functab(current_func, param, type);
            }

            for (int i = 0; i < MAXCHILDREN; i++)
            {
                traverse(t->child[i], preProc, postProc);
            }
            // restore the env
            func_stack.pop_back();
            current_func = func_stack.back();

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
            sym_insert(symtabs[current_func], t->child[1]->str, t->child[1]->lineno, 1, Integer);
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
        sym_insert(symtabs[current_func], tmp->str, tmp->lineno, counter, type);
    }
}

void null_proc(TreeNode *t)
{
    return;
}

// build symbol tables
void build_symtabs(TreeNode *t)
{
    current_func = "global";
    symtabs = map<string, SymTab *>();
    // could be delete because no funcion recursion
    //  when building symbol table
    func_stack = vector<string>();
    // new_func = false;

    symtabs[current_func] = init_symtab();
    func_stack.push_back("global");

    // traverse(t, insert_node, wrap_up);
    traverse(t, insert_node, null_proc);

    assert(func_stack.back() == "global");
    printf("Pass build symbol tables.\n");
}

// print symbol tables
void print_symtabs(FILE* listing){
    for(auto m = symtabs.begin(); m != symtabs.end(); m++){
        string table_name = m->first;
        auto table = m->second;
        fprintf(listing, "%s\n", table_name.c_str());
        print_symtab(table, listing);
        fprintf(listing, "\n");
    }
}

// print function tables
void print_functabs(FILE* listing){
    fprintf(listing, "%-16s%-10s%s\n", "function", "return", "parameters");
    for(auto m = functabs.begin(); m != functabs.end(); m++){
        // string func_name = m->first;
        FuncTab* func_info = m->second;
        print_functab(func_info, listing);
        fprintf(listing, "\n");
    }
}

bool is_in_current_symtab(string name){
    SymTab* c = symtabs[current_func];
    return c->m.find(name) != c->m.end();
}

// used to check all the proper type of the exp 
// just use the env, traverse will handle that thing
void check_node(TreeNode* t){
    switch (t->token)
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
    case Token_noEqual:{
        // if both operators valid, then it is valid
        if (t->child[0]->type == 1 && t->child[1]->type == 1)
        {
            // valid arithmetic
            t->type = 1;
        }
        else{
            printf("Invalid exp for %s", token2char(t->token));
            t->type = 0;
        }
        
        break;
    }
    case Token_call: {
        // function name is .str
        // check return type valid
        switch(functabs[t->str]->ret_type){
            case 1:
                // return type Void
                t->type = 0;
                break;
            case 0:
            case 2:
                // return type Integer
                t->type = 1;
                break;
            default:
                printf("No such funtion return type %d\nExiting", functabs[t->str]->ret_type);
                exit(1);
        }

        // check argument type and number
        // use vector to indicate sequence instead of map
        // map<string, int>* params = get_paras_list_functab(functabs[current_func]);
        const vector<ParaInstant*>* params = get_paras_list_functab(functabs[current_func]);
        TreeNode* exp = t->child[1];
        bool valid = true;
        int counter = 0;

        while (exp != nullptr)
        {
            // for params: 1 for int, 2 for array
            int type_para = (*params)[counter]->type;
            // for TN type: 0 for void, 1 for integer, 2 for array
            int type_tn = exp->type;

            if (type_para != type_tn)
            {
                valid = false;
                // passing type_tn to type_para
                printf("Invalid parameters type. Passing %s to %s(%s)\n", TYPE[type_tn], TYPE[type_para], (*params)[counter]->name.c_str());
            }

            counter++;
            exp = exp->sibling;
        }
        
        if (!valid || (counter != params->size()) )
        {
            // it is not valid
            printf("Invalid parameters type or mismatch parameters number.\n");
            exit(1);
        }
        // it is valid then keep going
        break;
    }
    case Token_var:{
        if (t->num == 0)
        {
            // invalid variable type
            printf("Invalid variable type Void for %s", t->str);
            exit(1);
        }
        if (t->num == 2)
        {
            if (t->child[0] != nullptr)
            {
                // this is an valid reference in the array
                t->type = 1;
            }
            else{
                t->type = 2;
            }
        }
        else{
            t->type = 1;
        }
        break;
    }
    case Token_number:
        t->type = 1;
        break;


    // Below is the statement check
    case Token_if:
    case Token_while:
        if (t->child[0]->type == 1)
        {
            // it can be a valid exp
            t->type = 1;
        }
        break;
    case Token_var_dec:
        // it does not to be check
        break;
    case Token_assign:
        // valid only if 
        // 1. var in symtab 
        // 2. var is Integer type
        // underneath will deal with var's type
        if (t->child[0]->type != 1 && !is_in_current_symtab(t->child[0]->str))
        {
            // invalid assignment
            printf("Invalid assignment to `%s(%s)`\n", TYPE[t->child[0]->type], t->child[0]->str);
            exit(1);
        }
        if (t->child[1]->type != 1)
        {
            // it is a invalid exp
            printf("Invalid expression in assignment\n");
            exit(1);
        }
        
        break;
    case Token_return:{
        bool valid = 1;
        int ret_type = functabs[current_func]->ret_type;
        if (t->child[0] == nullptr)
        {
            // 1 for void, 2 for int
            // 0 is default int

            // it has no return value
            if (ret_type != 1)
            {
                // valid if return type is Void 1
                valid = 0;
            }
        }
        else{
            // it has return value with type Integer
            //  and must be Integer 2 or default 0
            if (t->child[0]->type == 1){
                if (ret_type != 0 && ret_type != 2)
                {
                    // return type is not Integer
                    valid = 0;
                }
            }
            else{
                valid = 0;
            }
        }
        if (valid)
        {
            // do something to store it in a proper loc?
            // TODO
        }
        else{
            printf("Mismatch return type in function `%s`", current_func.c_str());
        }
        break;
    }
    default:
        printf("Fall through to default in type_check with type %s", tokens[t->token - tokens_offset]);
        exit(1);
        break;
    }
}

void type_check(TreeNode* t){
    current_func = "global";
    func_stack.clear();
    traverse(t, null_proc, check_node);
}