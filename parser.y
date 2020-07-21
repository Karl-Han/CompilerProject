%{
/*
 * Token_if = 128, Token_else, Token_int, 
 * 	Token_void, Token_while, Token_return,
 * Token_plus, Token_minus, Token_multiply, Token_divide,
 * 	Token_less, Token_lessEqual, Token_more, Token_moreEqual,
 * 	Token_equal, Token_noEqual, Token_assign, Token_semicolon,
 * 	Token_comma, Token_smallBracket_left, Token_smallBracket_right,
 * 	Token_middleBracket_left, Token_middleBracket_right, 
 * 	Token_largeBracket_left, Token_largeBracket_right,
 * Token_number, Token_comment, Token_identifier, Token_space, Token_none
 */
 /*
  * Extra token
  * Token_func_dec, Token_compound, Token_var_dec, Token_para
  */
#include "token.h"
#include <stdio.h>
// y reduce to x
#define TRAN(x,y) // printf("%s reduce to %s\n", y, x)
yydebug = 1;
TreeNode* root;

%}

%error-verbose
%verbose
%define parse.trace
// %require "3.2"
// %language "c++"


%union 	{
    void* tn;
}

// %token <Token> Token_identifier
%token <tn> Token_if 
%token <tn> Token_else 
%token <tn> Token_int 
%token <tn> Token_void 
%token <tn> Token_while 
%token <tn> Token_return
%token <tn> Token_plus 
%token <tn> Token_minus 
%token <tn> Token_multiply 
%token <tn> Token_divide
%token <tn> Token_less 
%token <tn> Token_lessEqual 
%token <tn> Token_more 
%token <tn> Token_moreEqual
%token <tn> Token_equal 
%token <tn> Token_noEqual 
%token <tn> Token_assign 
%token <tn> Token_semicolon
%token <tn> Token_comma 
%token <tn> Token_smallBracket_left 
%token <tn> Token_smallBracket_right
%token <tn> Token_middleBracket_left 
%token <tn> Token_middleBracket_right 
%token <tn> Token_largeBracket_left 
%token <tn> Token_largeBracket_right
%token <tn> Token_number 
%token <tn> Token_comment 
%token <tn> Token_identifier 
%token <tn> Token_space 
%token <tn> Token_none
%token <tn> Token_func 
%token <tn> Token_compound 
%token <tn> Token_var 
%token <tn> Token_para 
%token <tn> Token_call 
%token <tn> Token_var_dec

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
program         :   dec_list                        	{TRAN("program", "dec_list"); $$ = $1; root = $$;}
                ;

dec_list        :   declaration dec_list_sub        	{TRAN("dec_list","dec, dec_list_sub");TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

dec_list_sub    :   declaration dec_list_sub        	{TRAN("dec_list_sub","dec, dec_list_sub");TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                |   /* empty */                     	{TRAN("dec_list_sub","epsilon"); $$ = NULL; }
                ;

declaration     :   var_dec                         	{TRAN("dec","var_dec"); $$ = $1; }
                |   func_dec                        	{TRAN("dec","func_dec"); $$ = $1; }
                ;

var_dec         :   type Token_identifier Token_semicolon       	{TRAN("var_dec","var ID;");TreeNode* tn = getTreeNode(Token_var_dec); tn->child[0] = $1; tn->child[1] = $2; $$ = tn;}
                |   type Token_identifier Token_middleBracket_left Token_number Token_middleBracket_right Token_semicolon 	{TRAN("var_dec","var ID[];");TreeNode* tn = getTreeNode(Token_var_dec); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] = $4; $$ = tn;}
                ;

type            :   Token_int                       	{TRAN("type", "Token_int"); $$ = $1; }
                |   Token_void                      	{TRAN("type", "Token_void"); $$ = $1; }
                ;

func_dec        :   type Token_identifier Token_smallBracket_left params Token_smallBracket_right    	{TRAN("func_dec","func()");TreeNode* tn = getTreeNode(Token_func); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] = $4; $$ = tn;}
                |   compoud_st                      	{TRAN("func_dec","compoud"); $$ = $1; }
                ;

params          :   params_list                     	{TRAN("params","params_list"); $$ = $1; }
                |   Token_void                      	{TRAN("params","void"); $$ = $1; }
                ;

params_list     :   param params_list_sub           	{TRAN("params_list","param params_list_sub");TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

params_list_sub :   Token_comma param params_list_sub       	{TRAN("params_list_sub",",param params_list_sub");TreeNode* tn = $2; tn->sibling = $3; $$ = tn;}
                |   /* empty */                     	{TRAN("params_list_sub","epsilon"); $$ = NULL; }
                ;

// different param format is indicated in the child[2]
param           :   type Token_identifier           	{TRAN("param","var ID");TreeNode* tn = getTreeNode(Token_para); tn->child[0] = $1; tn->child[1] = $2; $$ = tn;}
                |   type Token_identifier Token_middleBracket_left Token_middleBracket_right      	{TRAN("param","var ID[]");TreeNode* tn = getTreeNode(Token_para); tn->child[0] = $1; tn->child[1] = $2; tn->child[2] = (TreeNode*)0x1; $$ = tn;}
                ;

compoud_st      :   Token_largeBracket_left local_dec stmt_list Token_largeBracket_right       	{TRAN("compoud","{ local_dec stmt_list }");TreeNode* tn = getTreeNode(Token_compound); tn->child[0] = $2; tn->child[1] = $3; $$ = tn;}
                ;

local_dec       :   var_dec local_dec               	{TRAN("local_dec","var_dec local_dec");TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                |   /* empty */                     	{TRAN("local_dec","epsilon"); $$ = NULL; }
                ;

stmt_list       :   statement   stmt_list           	{TRAN("stmt_list","statement stmt_list");TreeNode* tn = $1; if(tn == NULL) tn = $2; else tn->sibling = $2; $$ = tn;}
                |   /* empty */                     	{TRAN("stmt_list","epsilon"); $$ = NULL; }
                ;

statement       :   exp_st                          	{TRAN("statement","exp_st"); $$ = $1; }
                |   compoud_st                      	{TRAN("statement","compoud"); $$ = $1; }
                |   selection_st                    	{TRAN("statement","selection_st"); $$ = $1; }
                |   iteration_st                    	{TRAN("statement","iteration_st"); $$ = $1; }
                |   return_st                       	{TRAN("statement","return_st"); $$ = $1; }
                ;

exp_st          :   exp Token_semicolon                         	{TRAN("exp_st","exp;"); $$ = $1; }
                |   Token_semicolon                             	{TRAN("exp_st",";"); $$ = NULL; }
                ;

selection_st    :   Token_if Token_smallBracket_left exp Token_smallBracket_right statement  	{TRAN("selection_st","if exp"); TreeNode* tn = getTreeNode(Token_if); tn->child[0] = $3; tn->child[1] = $5; $$ = tn;}
                |   Token_if Token_smallBracket_left exp Token_smallBracket_right statement Token_else statement	{TRAN("selection_st","if_else"); TreeNode* tn = getTreeNode(Token_if); tn->child[0] = $3; tn->child[1] = $5; tn->child[2] = $7; $$ = tn;}
                ;

iteration_st    :   Token_while Token_smallBracket_left exp Token_smallBracket_right statement   	{TRAN("iteration_st","while exp");TreeNode* tn = getTreeNode(Token_while); tn->child[0] = $3; tn->child[1] = $5; $$ = tn;}
                ;

return_st       :   Token_return Token_semicolon                	{TRAN("return_st","return;");TreeNode* tn = getTreeNode(Token_return); $$ = tn;};
                |   Token_return exp Token_semicolon            	{TRAN("return_st","return exp;");TreeNode* tn = getTreeNode(Token_return); tn->child[0] = $2; $$ = tn;};
                ;

exp             :   var Token_assign exp            	{TRAN("exp","var = exp");TreeNode* tn = getTreeNode(Token_assign); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;};
                |   simple_exp                      	{TRAN("exp","simple_exp");$$ = $1;};
                ;

var             :   Token_identifier                	{TRAN("var","identifier");TreeNode* tn = getTreeNode(Token_var); tn->child[0] = $1; $$ = tn;};
                |   Token_identifier Token_middleBracket_left exp Token_middleBracket_right    	{TRAN("var","identifier[]");TreeNode* tn = getTreeNode(Token_var); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                ;

simple_exp      :   additive_exp relop additive_exp    	{TRAN("simple_exp","additive_exp lop additive_exp");TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   additive_exp                    	{TRAN("simple_exp","additive_exp"); $$ = $1; }
                ;

relop           :   Token_less                      	{TRAN("relop","<"); $$ = $1; } 
                |   Token_lessEqual                 	{TRAN("relop","<="); $$ = $1; }     
                |   Token_more                      	{TRAN("relop",">"); $$ = $1; } 
                |   Token_moreEqual                 	{TRAN("relop",">="); $$ = $1; }     
                |   Token_equal                     	{TRAN("relop","=="); $$ = $1; } 
                |   Token_noEqual                   	{TRAN("relop","!="); $$ = $1; }     
                ;

additive_exp    :   additive_exp addop term         	{TRAN("additive_exp","additive_exp addop term");TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   term                            	{TRAN("additive_exp","term"); $$ = $1; }
                ;

addop           :   Token_plus                      	{TRAN("addop","+"); $$ = $1; }
                |   Token_minus                     	{TRAN("addop","-"); $$ = $1; }
                ;

term            :   term mulop factor               	{TRAN("term","term mulop factor");TreeNode* tn = $2; tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                |   factor                          	{TRAN("term","factor"); $$ = $1; }
                ;

mulop           :   Token_multiply                  	{TRAN("mulop","*"); $$ = $1; }
                |   Token_divide                    	{TRAN("mulop","/"); $$ = $1; }
                ;

factor          :   Token_smallBracket_left exp Token_smallBracket_right                     	{TRAN("factor","(exp)"); $$ = $2; }            
                |   var                             	{TRAN("factor","var"); $$ = $1; }    
                |   call                            	{TRAN("factor","call"); $$ = $1; }        
                |   Token_number                    	{TRAN("factor","num"); $$ = $1; }                
                ;

call            :   Token_identifier Token_smallBracket_left args Token_smallBracket_right   	{TRAN("call","id, args"); TreeNode* tn = getTreeNode(Token_call); tn->child[0] = $1; tn->child[1] = $3; $$ = tn;}
                ;

args            :   arg_list                        	{TRAN("args","arg_list"); $$ = $1; }
                |   /* empty */                     	{TRAN("args","epsilon"); $$ = NULL; }
                ;

arg_list        :   exp arg_list_sub                	{TRAN("arg_list","exp, arg_list_sub");TreeNode* tn = $1; tn->sibling = $2; $$ = tn;}
                ;

arg_list_sub    :   Token_comma exp arg_list_sub            	{TRAN("arg_list_sub","comma, exp, arg_list_sub");TreeNode* tn = $2; tn->sibling = $3; $$ = tn;}
                |   /* empty */                     	{TRAN("arg_list_sub","epsilon"); $$ = NULL; }
                ;
%%

// int main() 	{
//     yyparse();
//     TreeNode* tn = root;
//     return 0;
// }
