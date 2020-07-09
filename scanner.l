%{
// Declaration
// Will be copy to lex.yy.c directly
// #include "token.h"
#include "y.tab.h"
#include "token.h"

#define MAX_CHILDREN 4

struct {
    Token token;

    int num;
    char* str;
    struct TreeNode* sibling;
    struct TreeNode*   children[MAX_CHILDREN];
}TreeNode;


int colno = 0, lineno= 1;

Token getSymbol(char);
void comment();
void lex_error(char*);
void newline();
char* token2string(Token t);

void init_sib_child(TreeNode*);
// TreeNode section
TreeNode* getTreeNode(Token);
TreeNode* getTreeNode_number(Token, int);
TreeNode* getTreeNode_identifier(Token, char*);

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
%}

/* 
 * Definition
 * For symbols with specific meaning used afterwards
 */

IDENTIFIER  ([_a-zA-Z][0-9_a-zA-Z]*)
NUMBER      ([0-9]+)
SPACE       ([ \t]+)
SYMBOL      ([+\-*/<>=;,(){}\[\]])

%%
{SPACE}         {colno += yyleng; /* Do nothing */}
[\n]            {/* Do nothing */ newline();}
{SYMBOL}        {Token token = getSymbol(yytext[0]);TreeNode* tn = getTreeNode(token); yylval = tn;colno += yyleng; return token}
"int"           {TreeNode* tn = getTreeNode(Token_int); yylval = tn;colno += yyleng; return Token_lessEqual;}
"void"          {TreeNode* tn = getTreeNode(Token_void); yylval = tn;colno += yyleng; return Token_lessEqual;}
"if"            {TreeNode* tn = getTreeNode(Token_if); yylval = tn;colno += yyleng; return Token_lessEqual;}
"else"          {TreeNode* tn = getTreeNode(Token_else); yylval = tn;colno += yyleng; return Token_lessEqual;}
"while"         {TreeNode* tn = getTreeNode(Token_while); yylval = tn;colno += yyleng; return Token_lessEqual;}
"return"        {TreeNode* tn = getTreeNode(Token_return); yylval = tn;colno += yyleng; return Token_lessEqual;}
"<="            {TreeNode* tn = getTreeNode(Token_lessEqual); yylval = tn;colno += yyleng; return Token_lessEqual;}
">="            {TreeNode* tn = getTreeNode(Token_moreEqual); yylval = tn;colno += yyleng; return Token_moreEqual;}
"=="            {TreeNode* tn = getTreeNode(Token_equal); yylval = tn;colno += yyleng; return Token_equal;}
"!="            {TreeNode* tn = getTreeNode(Token_noEqual); yylval = tn;colno += yyleng; return Token_noEqual;}
"/*"            {colno += yyleng; comment(); return Token_comment;}
{NUMBER}        {TreeNode* tn = getTreeNode(Token_number); yylval = tn; colno += yyleng; return Token_number;}
{IDENTIFIER}    {TreeNode* tn = getTreeNode(Token_identifier); yylval = tn;colno += yyleng; return Token_identifier;}
<<EOF>>         {return 0;}
.               {colno += yyleng; lex_error(yytext);}

%%
void init_sib_child(TreeNode* tn){
    tn->sibling = NULL;
    for(size_t i = 0; i < MAX_CHILDREN; i++){
        tn->children[i] = NULL;
    }
}

TreeNode* getTreeNode(Token token){
    // number iden symbol, lessE, moreE, equal, noE,
    // SYMBOL      ([+-*\<>=;,(){}[]])
    TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

    tn->token = token;
    tn->num = 0;
    tn->str = NULL;
    init_sib_child(tn);
}

TreeNode* getTreeNode_number(Token token, int num){
    TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

    if(token != Token_number){
        lex_error("This is not a number.(getTreeNode_number)");
        return NULL;
    }

    tn->token = token;
    tn->num = num;
    tn->str = null;
    init_sib_child(tn);
}

TreeNode* getTreeNode_identifier(Token token, char* str){
    TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

    if(token != Token_identifier){
        lex_error("This is not a identifier.(getTreeNode_identifier)");
        return NULL;
    }

    tn->token = token;
    tn->num = 0;
    tn->str = strdup(str);
    init_sib_child(tn);
}

void newline(){
    lineno++;
    colno = 0;
}

void lex_error(char* str){
    printf("Lex ERROR in line %d, column %d. %s\n", str);
}

int main(){
    Token token = Token_none;
    while(token = yylex()){
        if(token != Token_comment)
            printf("%s\t%s\n", yytext, token2string(token));
    }
    return 0;
}

int yywrap(){
    return 1;
}

void comment(){
    char ch1, ch2;
    loop:
    while((ch1 = input()) != '*' && ch1 != 0){
        // just skip them
        if(ch1 == '\0')
            return;
        if(ch1 == '\n')
            newline();
        else 
            colno++;
    }
    ch2 = input();
    if(ch2 != '/' && ch2 != 0){
        ch1 = ch2 = '0';
        if(ch2 == '\n')
            newline();
        else 
            colno++;
        goto loop;
    }
}

Token getSymbol(char ch){
    // SYMBOL      ([+\-*/<>=;,(){}\[\]])
    switch(ch){
        case '+':
            return Token_plus;
        case '-':
            return Token_minus;
        case '*':
            return Token_multiply;
        case '/':
            return Token_divide;
        case '<':
            return Token_less;
        case '>':
            return Token_more;
        case '=':
            return Token_assign;
        case ';':
            return Token_semicolon;
        case ',':
            return Token_comma;
        case '(':
            return Toekn_smallBracket_left;
        case ')':
            return Token_smallBracket_right;
        case '[':
            return Token_middleBracket_left;
        case ']':
            return Token_middleBracket_right;
        case '{':
            return Token_largeBracket_left;
        case '}':
            return Token_largeBracket_left;
        default:
            printf("This char is %c\n", ch);
            exit(1);
    }
}

char* token2string(Token t){
	char* str_token[] = {"Token_if","Token_else","Token_int","Token_void","Token_while","Token_return","Token_plus","Token_minus","Token_multiply","Token_divide","Token_less","Token_lessEqual","Token_more","Token_moreEqual","Token_equal","Token_noEqual","Token_assign","Token_semicolon","Token_comma","Toekn_smallBracket_left","Token_smallBracket_right","Token_middleBracket_left","Token_middleBracket_right","Token_largeBracket_left","Token_largeBracket_right","Token_number","Token_comment","Token_identifier","Token_space", "Token_func_dec", "Token_compound", "Token_var_dec", "Token_para"};
	return str_token[t - Token_if];
}