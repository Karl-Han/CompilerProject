#include "tables.h"

extern "C"
{
#include "token.h"
#include "y.tab.h"
}

#define MAXCHILDREN 4

static int memloc = 0;

string current_env;
map<string, SymTab *> envs;
vector<string> func_stack;

void traverse(TreeNode *t, void (*preProc)(TreeNode *), void (*postProc)(TreeNode *))
{
    if (t != NULL)
    {
        if (t->token != (int)Token_func)
        {
            // this is not a function node 
            preProc(t);
            {
                int i;
                for (i = 0; i < MAXCHILDREN; i++)
                    traverse(t->child[i], preProc, postProc);
            }
            postProc(t);
            traverse(t->sibling, preProc, postProc);
        }
        else
        {
            // this is a fucntion node
            current_env = t->child[1]->str;
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
        // sym_insert(envs[current_env], t->child[0]->str, t->child[0]->lineno, memloc++, 1, Void);
        tmp = t->child[0];
        counter = 1;
        type = Void;
        break;
    }
    case Token_var_dec:
    {
        tmp = t->child[1];
        // this is the place for number
        if (t->child[2] != 0)
        {
            // this is a array
            counter = t->child[2]->num;
        }
        else
        {
            counter = 1;
        }
        if (t->child[0]->token == Token_int)
        {
            type = Integer;
        }
        else
        {
            printf("Wrong type of variable declararion.\nIn building symbol.");
            exit(1);
        }
        break;
    }
    case Token_para:
    {
        tmp = t->child[1];
        if (t->child[2] == (TreeNode *)0x1)
        {
            type = Array;
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
            sym_insert(envs[current_env], t->child[1]->str, t->child[1]->lineno, memloc++, 1, Integer);
        }
        break;
    }
    case Token_identifier:
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
        sym_insert(envs[current_env], tmp->str, tmp->lineno, memloc++, counter, type);
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

void null_proc(TreeNode* t){
    return ;
}

void buildSymtabs(TreeNode *t)
{
    current_env = "global";
    envs = map<string, SymTab *>();
    // could be delete because no funcion recursion
    //  when building symbol table
    func_stack = vector<string>();
    // new_func = false;

    envs[current_env] = init_symtab();
    func_stack.push_back("global");

    // traverse(t, insert_node, wrap_up);
    traverse(t, insert_node, null_proc);

    assert(func_stack.back() == "global");
    printf("Pass build symbol tables.");
}

// void buildSymtabs_c(TreeNode *t)
// {
//     return buildSymtabs(t);
// }
