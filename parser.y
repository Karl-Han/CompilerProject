%{
/*
 * Token_if = 128, Token_else, Token_int, 
 * 	Token_void, Token_while, Token_return,
 * Token_plus, Token_minus, Token_multiply, Token_divide,
 * 	Token_less, Token_lessEqual, Token_more, Token_moreEqual,
 * 	Token_equal, Token_noEqual, Token_assign, Token_semicolon,
 * 	Token_comma, Toekn_smallBracket_left, Token_smallBracket_right,
 * 	Token_middleBracket_left, Token_middleBracket_right, 
 * 	Token_largeBracket_left, Token_largeBracket_right,
 * Token_number, Token_comment, Token_identifier, Token_space, Token_none
 */
 /*
  * Extra token
  * Token_func_dec, Token_compound, Token_var_dec, Token_para
  */
#include "token.h"

%}

%debug

%union {
    void* tn;
}

// %token <Token> Token_identifier
%token <tn> Token_if Token_else Token_int Token_void Token_while Token_return
%token <tn> Token_plus Token_minus Token_multiply Token_divide
%token <tn> Token_less Token_lessEqual Token_more Token_moreEqual
%token <tn> Token_equal Token_noEqual Token_assign Token_semicolon
%token <tn> Token_comma Toekn_smallBracket_left Token_smallBracket_right
%token <tn> Token_middleBracket_left Token_middleBracket_right 
%token <tn> Token_largeBracket_left Token_largeBracket_right
%token <tn> Token_number Token_comment Token_identifier Token_space Token_none
%token <tn> Token_func Token_compound Token_var Token_para Token_call Token_var_dec

%left Token_plus Token_minus
%left Token_multiply Token_divide
%type <tn> program         
%type <tn> dec_list        
%type <tn> dec_list_sub    
%type <tn> declaration     
%type <tn> var_dec         
%type <tn> type            
%type <tn> func_dec        
%type <tn> params          
%type <tn> params_list     
%type <tn> params_list_sub 
%type <tn> param           
%type <tn> compoud_st      
%type <tn> local_dec       
%type <tn> stmt_list       
%type <tn> statement       
%type <tn> exp_st          
%type <tn> selection_st    
%type <tn> iteration_st    
%type <tn> return_st       
%type <tn> exp             
%type <tn> var             
%type <tn> simple_exp      
%type <tn> relop           
%type <tn> additive_exp    
%type <tn> addop           
%type <tn> term            
%type <tn> mulop           
%type <tn> factor          
%type <tn> call            
%type <tn> args            
%type <tn> arg_list        
%type <tn> arg_list_sub    

%%
program         :   dec_list                        { $$ = $1;}
                ;

dec_list        :   declaration dec_list_sub        {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

dec_list_sub    :   declaration dec_list_sub        {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                |   /* empty */                     { $$ = NULL; }
                ;

declaration     :   var_dec                         { $$ = $1; }
                |   func_dec                        { $$ = $1; }
                ;

var_dec         :   type Token_identifier ';'       {TreeNode* tn = getTreeNode(Token_var_dec); tn->child[0] = $1; tn->child[1] = $2; $$ = tn;}
                |   type Token_identifier '[' Token_number ']' ';' {TreeNode* tn = getTreeNode(Token_var_dec); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] = $4; $$ = tn;}
                ;

type            :   Token_int                       { $$ = $1; }
                |   Token_void                      { $$ = $1; }
                ;

func_dec        :   type Token_identifier '(' params ')'    {TreeNode* tn = getTreeNode(Token_func); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] = $4; $$ = tn;}
                |   compoud_st                      { $$ = $1; }
                ;

params          :   params_list                     { $$ = $1; }
                |   Token_void                      { $$ = $1; }
                ;

params_list     :   param params_list_sub           {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

params_list_sub :   ',' param params_list_sub       {TreeNode* tn = $2; tn->sibling = $3; $$ = tn;}
                |   /* empty */                     { $$ = NULL; }
                ;

param           :   type Token_identifier           {TreeNode* tn = getTreeNode(Token_para); tn->child[0] = $1; tn->child[1] = $2; $$ = tn;}
                |   type Token_identifier '[' ']'      {TreeNode* tn = getTreeNode(Token_para); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] == 0x1; $$ = tn;}
                ;

compoud_st      :   '{' local_dec stmt_list '}'       {TreeNode* tn = getTreeNode(Token_compound); tn->child[0] = $2; tn->child[1] = $3; $$ = tn;}
                ;

local_dec       :   var_dec local_dec               {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                |   /* empty */                     { $$ = NULL; }
                ;

stmt_list       :   statement   stmt_list           {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                |   /* empty */                     { $$ = NULL; }
                ;

statement       :   exp_st                          { $$ = $1; }
                |   compoud_st                      { $$ = $1; }
                |   selection_st                    { $$ = $1; }
                |   iteration_st                    { $$ = $1; }
                |   return_st                       { $$ = $1; }
                ;

exp_st          :   exp ';'                         { $$ = $1; }
                |   ';'                             { $$ = NULL; }
                ;

selection_st    :   Token_if '(' exp ')' statement  { TreeNode* tn = getTreeNode(Token_if); tn->child[0] = $3; tn->child[1] = $5; $$ = tn;}
                |   Token_if '(' exp ')' statement Token_else statement{ TreeNode* tn = getTreeNode(Token_if); tn->child[0] = $3; tn->child[1] = $5; tn->child[2] = $7; $$ = tn;}
                ;

iteration_st    :   Token_while '(' exp ')' statement   {TreeNode* tn = getTreeNode(Token_while); tn->child[0] = $3; tn->child[1] = $5; $$ = tn;}
                ;

return_st       :   Token_return ';'                {TreeNode* tn = getTreeNode(Token_return); $$ = tn;};
                |   Token_return exp ';'            {TreeNode* tn = getTreeNode(Token_return); tn->child[0] = $2; $$ = tn;};
                ;

exp             :   var Token_assign exp            {TreeNode* tn = getTreeNode(Token_assign); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;};
                |   simple_exp                      {$$ = $1;};
                ;

var             :   Token_identifier                {TreeNode* tn = getTreeNode(Token_var); tn->child[0] = $1; $$ = tn;};
                |   Token_identifier '[' exp ']'    {TreeNode* tn = getTreeNode(Token_var); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                ;

simple_exp      :   additive_exp relop additive_exp    {TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   additive_exp                    { $$ = $1; }
                ;

relop           :   Token_less                      { $$ = $1; } 
                |   Token_lessEqual                 { $$ = $1; }     
                |   Token_more                      { $$ = $1; } 
                |   Token_moreEqual                 { $$ = $1; }     
                |   Token_equal                     { $$ = $1; } 
                |   Token_noEqual                   { $$ = $1; }     
                ;

additive_exp    :   additive_exp addop term         {TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   term                            { $$ = $1; }
                ;

addop           :   Token_plus                      { $$ = $1; }
                |   Token_minus                     { $$ = $1; }
                ;

term            :   term mulop factor               {TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   factor                          { $$ = $1; }
                ;

mulop           :   Token_multiply                  { $$ = $1; }
                |   Token_divide                    { $$ = $1; }
                ;

factor          :   '(' exp ')'                     { $$ = $2; }            
                |   var                             { $$ = $1; }    
                |   call                            { $$ = $1; }        
                |   Token_number                    { $$ = $1; }                
                ;

call            :   Token_identifier '(' args ')'   { TreeNode* tn = getTreeNode(Token_call); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                ;

args            :   arg_list                        { $$ = $1; }
                |   /* empty */                     { $$ = NULL; }
                ;

arg_list        :   exp arg_list_sub                {TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

arg_list_sub    :   ',' exp arg_list_sub            {TreeNode* tn = $2; tn->sibling = $3; $$ = tn;}
                |   /* empty */                     { $$ = NULL; }
                ;
%%

int main() {
    TreeNode* tn = yyparse();
    return 0;
}