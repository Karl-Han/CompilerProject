#include "analyze.h"

extern "C"
{
#include "y.tab.h"
}

#define MAXCHILDREN 4
static char *TYPE[] = {"VOID", "INTEGER", "ARRAY"};
map<string, SymTab *> *symtabs;
map<string, FuncTab *> *functabs;
static string global_str = "global";

namespace analyze
{
    string current_func;
} // namespace analyze

void seg_fault()
{
    ((TreeNode *)0x0)->type = 1;
}

// get the parameter list
// invoke by functab
vector<ParaInstant *> *get_param(TreeNode *para)
{
    TreeNode *tmp = para;
    vector<ParaInstant *> *v = new vector<ParaInstant *>();
    if (para->token == (int)Token_void)
    {
        // no parameter
        return v;
    }

    while (tmp != nullptr)
    {
        // get the variable name
        ParaInstant *pi = new ParaInstant();
        pi->name = tmp->str;
        pi->type = tmp->num;

        v->push_back(pi);
        tmp = tmp->sibling;
    }

    return v;
}

// used to traverse the tree structure
// also change the analyze::current_function during the process
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
            analyze::current_func = t->str;

            // init function table if no find
            if (symtabs->find(analyze::current_func) == symtabs->end())
            {
                // this is a new function
                // 1. create new symbol table for func
                // 2. insert into function table
                (*symtabs)[analyze::current_func] = init_symtab(analyze::current_func);
                int type;

                // deal with return type
                switch (t->child[0]->token)
                {
                // 1 for void, 2 for int
                // 0 is default int
                case Token_int:
                {
                    // return type is int
                    type = 2;
                    break;
                }
                case Token_void:
                {
                    // return type is void
                    type = 1;
                    break;
                }

                default:
                    type = 0;
                    break;
                }

                vector<ParaInstant *> *param = get_param(t->child[2]);
                (*functabs)[analyze::current_func] = init_functab(analyze::current_func, param, type);
            }

            for (int i = 0; i < MAXCHILDREN; i++)
            {
                traverse(t->child[i], preProc, postProc);
            }
            // restore the env
            analyze::current_func = "global";

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
    case Token_read:
    {
        // this is insert_node instead of check node
        tmp = t;
        counter = 1;
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
            // this is a integer
            counter = 1;
            type = Integer;
        }

        if (t->child[0]->token != Token_int)
        {
            printf("Wrong type of variable `%s` type %s declararion.\nIn building symbol.", t->child[1]->str, tokens[t->child[0]->token - tokens_offset]);
            seg_fault();
        }
        break;
    }
    case Token_para:
    {

        tmp = t;
        if (t->num == 2)
        {
            // it is pass as reference in the future
            type = Array;
        }
        else
        {
            type = Integer;
            counter = 1;
        }
        break;
    }
    case Token_assign:
    {
        tmp = t;
        type = Integer;
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
            sym_insert((*symtabs)[analyze::current_func], t->child[1]->str, t->child[1]->lineno, 1, Integer);
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
    default:
        // printf("No ID appears in %s\nExiting", tokens[t->token-tokens_offset]);
        // exit(1);
        break;
    }
    if (tmp != nullptr)
    {
        // do insert
        sym_insert((*symtabs)[analyze::current_func], tmp->str, tmp->lineno, counter, type);
    }
}

void null_proc(TreeNode *t)
{
    return;
}

void init_functabs_size()
{
    for (auto i = functabs->begin(); i != functabs->end(); i++)
    {
        // i is (string, FuncTab*)
        // for each functab
        init_functab_size(i->second);
    }
}

// build symbol tables
void build_symtabs(TreeNode *t)
{
    analyze::current_func = "global";
    symtabs = new map<string, SymTab *>();
    functabs = new map<string, FuncTab *>();
    // could be delete because no funcion recursion
    //  when building symbol table
    // new_func = false;

    // // init `input` and `output`
    // (*functabs)["input"] = init_functab("input", new vector<ParaInstant *>(), 2);
    // vector<ParaInstant *> *output = new vector<ParaInstant *>();
    // ParaInstant *pi = new ParaInstant();
    // pi->name = "x";
    // pi->type = 1;
    // output->push_back(pi);
    // (*functabs)["output"] = init_functab("output", output, 1);

    (*symtabs)[analyze::current_func] = init_symtab(analyze::current_func);

    // traverse(t, insert_node, wrap_up);
    traverse(t, insert_node, null_proc);

    init_functabs_size();
}

// print symbol tables
void print_symtabs(FILE *listing)
{
    for (auto m = symtabs->begin(); m != symtabs->end(); m++)
    {
        string table_name = m->first;
        auto table = m->second;
        fprintf(listing, "'%s'\n", table_name.c_str());
        print_symtab(table, listing);
        fprintf(listing, "\n");
    }
}

// print function tables
void print_functabs(FILE *listing)
{
    fprintf(listing, "%-16s%-10s%-15s%s\n", "function", "return", "functab size", "parameters");
    fprintf(listing, "-------------   --------  -----------  --------------------\n");
    for (auto m = functabs->begin(); m != functabs->end(); m++)
    {
        // string func_name = m->first;
        FuncTab *func_info = m->second;
        print_functab(func_info, listing);
        fprintf(listing, "\n");
    }
    fprintf(listing, "\n");
}

// used to check all the proper type of the exp
// just use the env, traverse will handle that thing
void check_node(TreeNode *t)
{
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
    case Token_noEqual:
    {
        // if both operators valid, then it is valid
        if (t->child[0]->type == 1 && t->child[1]->type == 1)
        {
            // valid arithmetic
            t->type = 1;
        }
        else
        {
            printf("Invalid exp for %s", token2char(t->token));
            t->type = 0;
        }

        break;
    }
    case Token_call:
    {
        // function name is .str
        // check return type valid
        auto f = functabs->find(t->str);
        FuncTab* ft = nullptr;
        if (f != functabs->end())
        {
            // find it 
            ft = f->second;
        }
        else{
            // no such function table
            printf("No such function table");
            seg_fault();
        }
        
        switch (ft->ret_type)
        {
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
            printf("No such funtion return type %d\nExiting", (*functabs)[t->str]->ret_type);
            seg_fault();
        }

        // check argument type and number
        // use vector to indicate sequence instead of map
        // map<string, int>* params = get_paras_list_functab((*functabs)[analyze::current_func]);
        const vector<ParaInstant *> *params = get_paras_list_functab((*functabs)[t->str]);
        TreeNode *exp = t->child[1];
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
                seg_fault();
            }

            counter++;
            exp = exp->sibling;
        }

        if (!valid || (counter != params->size()))
        {
            // it is not valid
            printf("Invalid parameters type or mismatch parameters number.\n");
            seg_fault();
        }
        // it is valid then keep going
        break;
    }
    case Token_var:
    {
        // .num is its type
        if (t->num == 0)
        {
            // invalid variable type
            printf("Invalid variable type Void for %s", t->str);
            seg_fault();
        }
        SymInfo_ret si = sym_lookup((*symtabs)[analyze::current_func], t->str);
        if (si.loc != -1 && si.type != Void)
        {
            // it is a local variable
            if (si.type == Array)
            {
                // this symbol is array
                if (t->child[0] == nullptr)
                {
                    // 0 for void, 1 for integer, 2 for array
                    // refer here as array
                    t->type = 2;
                }
                else
                {
                    // refer here as integer
                    t->type = 1;
                }
            }
            else if (si.type == Integer)
            {
                t->type = 1;
            }
        }
        else
        {
            SymInfo_ret si = sym_lookup((*symtabs)[global_str], t->str);
            if (si.loc != -1 && si.type != Void)
            {
                // it is a global variable
                if (si.type == Array)
                {
                    // this symbol is array
                    if (t->child[0] == nullptr)
                    {
                        // 0 for void, 1 for integer, 2 for array
                        // refer here as array
                        t->type = 2;
                    }
                    else
                    {
                        // refer here as integer
                        t->type = 1;
                    }
                }
                else if (si.type == Integer)
                {
                    t->type = 1;
                }
            }
            else
            {
                printf("No such symbol.\nImpossible to get here");
                seg_fault();
            }
        }

        break;
    }
    case Token_number:
        t->type = 1;
        break;

    // Below is the statement check
    case Token_read:
    {
        SymInfo_ret ret = sym_lookup((*symtabs)[analyze::current_func], t->str);
        if (ret.loc = -1 && ret.type == Void)
        {
            ret = sym_lookup((*symtabs)[global_str], t->str);
        }
        
        if (ret.type == Void)
        {
            // error
            printf("Refer to %s before declaration.\n", t->str);
            exit(1);
        }
        else if (ret.type == Array && t->child[0]->type != 1)
        {
            printf("Refer to Array(%s) with wrong index.\n", t->str);
            exit(1);
        }
        break;
    }
    case Token_write:
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
    {
        // valid only if
        // 1. var in symtab
        // 2. var is Integer type
        // underneath will deal with var's type
        SymInfo_ret ret = sym_lookup((*symtabs)[analyze::current_func], t->str);
        if (ret.loc = -1 && ret.type == Void)
        {
            ret = sym_lookup((*symtabs)[global_str], t->str);
        }
        if (ret.loc = -1 && ret.type == Void)
        {
            // no such symbol
            printf("No such symbol %s\n", t->str);
        }
        if (ret.type == Array && t->child[0] == nullptr)
        {
            // invalid assignment
            printf("Invalid assignment to `%s(%s)`\n", "Array", t->str);
            seg_fault();
        }

        // check validity of exp
        if (t->child[1]->type != 1)
        {
            // it is a invalid exp
            printf("Invalid expression in assignment\n");
            seg_fault();
        }
        break;
    }
    case Token_return:
    {
        bool valid = 1;
        int ret_type = (*functabs)[analyze::current_func]->ret_type;
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
        else
        {
            // it has return value with type Integer
            //  and must be Integer 2 or default 0
            if (t->child[0]->type == 1)
            {
                if (ret_type != 0 && ret_type != 2)
                {
                    // return type is not Integer
                    valid = 0;
                }
            }
            else
            {
                valid = 0;
            }
        }
        if (valid)
        {
            // do something to store it in a proper loc?
            // TODO
        }
        else
        {
            printf("Mismatch return type in function `%s`", analyze::current_func.c_str());
        }
        break;
    }
    default:
        // printf("Fall through to default in type_check with type %s", tokens[t->token - tokens_offset]);
        // ((TreeNode*)0x0)->type = 1;
        // exit(1);
        break;
    }
}

// if it is match the type of stmt and exp
void type_check(TreeNode *t)
{
    analyze::current_func = "global";
    traverse(t, null_proc, check_node);
}

void tag_treenode(TreeNode *t)
{
    switch (t->token)
    {
    // all compare -> arithmetic op
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
    // call stmt
    case Token_call:
    // var reference stmt
    case Token_var:
    // number
    case Token_number:
    {
        t->nk = ExpK;
        break;
    }

    // all statements
    case Token_func:
    case Token_if:
    case Token_while:
    case Token_var_dec:
    case Token_para:
    case Token_assign:
    case Token_return:
    {
        t->nk = StmtK;
        break;
    }
    default:
        // printf("This node %s's type unknown\n", token2char(t->token));
        break;
    }
}

void tag_kind(TreeNode *t)
{
    traverse(t, tag_treenode, null_proc);
}