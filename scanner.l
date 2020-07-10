%{
// Declaration
// Will be copy to lex.yy.c directly
#include "token.h"
#include "y.tab.h"
#include "lexee.h"

int colno = 1, lineno= 1;

yytoken_kind_t getSymbol(char);
void comment();
void yyerror(const char*);
void newline();
// char* token2string(yytoken_kind_t t);
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
{SPACE}         {init_linkedlist(); colno += yyleng; /* Do nothing */}
[\n]            {init_linkedlist(); /* Do nothing */ newline();}
{SYMBOL}        {init_linkedlist(); yytoken_kind_t token = getSymbol(yytext[0]);TreeNode* tn = getTreeNode(token); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return token;}
"int"           {init_linkedlist(); TreeNode* tn = getTreeNode(Token_int); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_int;}
"void"          {init_linkedlist(); TreeNode* tn = getTreeNode(Token_void); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_void;}
"if"            {init_linkedlist(); TreeNode* tn = getTreeNode(Token_if); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_if;}
"else"          {init_linkedlist(); TreeNode* tn = getTreeNode(Token_else); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_else;}
"while"         {init_linkedlist(); TreeNode* tn = getTreeNode(Token_while); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_while;}
"return"        {init_linkedlist(); TreeNode* tn = getTreeNode(Token_return); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_return;}
"<="            {init_linkedlist(); TreeNode* tn = getTreeNode(Token_lessEqual); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_lessEqual;}
">="            {init_linkedlist(); TreeNode* tn = getTreeNode(Token_moreEqual); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_moreEqual;}
"=="            {init_linkedlist(); TreeNode* tn = getTreeNode(Token_equal); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_equal;}
"!="            {init_linkedlist(); TreeNode* tn = getTreeNode(Token_noEqual); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_noEqual;}
"/*"            {init_linkedlist(); insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; comment();}
{NUMBER}        {init_linkedlist(); TreeNode* tn = getTreeNode_number(Token_number, atoi(yytext)); yylval.tn = tn; insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_number;}
{IDENTIFIER}    {init_linkedlist(); TreeNode* tn = getTreeNode_identifier(Token_identifier, yytext); yylval.tn = tn;insert_lexee(get_lexee(yytext, "Token_", lineno, colno)); colno += yyleng; return Token_identifier;}
<<EOF>>         {init_linkedlist(); return 0;}
.               {init_linkedlist(); colno += yyleng; yyerror(yytext);}

%%
void init_sib_child(TreeNode* tn){
	tn->sibling = NULL;
	for(size_t i = 0; i < MAX_CHILDREN; i++){
		tn->child[i] = NULL;
	}
}

TreeNode* getTreeNode(int token){
	// number iden symbol, lessE, moreE, equal, noE,
	// SYMBOL      ([+-*\<>=;,(){}[]])
	TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

	tn->token = token;
	tn->num = 0;
	tn->str = NULL;
	init_sib_child(tn);
	return tn;
}

TreeNode* getTreeNode_number(int token, int num){
	TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

	if(token != Token_number){
		yyerror("This is not a number.(getTreeNode_number)");
		return NULL;
	}

	tn->token = token;
	tn->num = num;
	tn->str = NULL;
	init_sib_child(tn);
	return tn;
}

TreeNode* getTreeNode_identifier(int token, char* str){
	TreeNode* tn = (TreeNode*)malloc(sizeof(TreeNode));

	if(token != Token_identifier){
		yyerror("This is not a identifier.(getTreeNode_identifier)");
		return NULL;
	}

	tn->token = token;
	tn->num = 0;
	tn->str = strdup(str);
	init_sib_child(tn);
	return tn;
}

void newline(){
	lineno++;
	colno = 1;
}

void yyerror(const char* str){
	printf("Lex ERROR in line %d, column %d. %s\n", lineno, colno, str);
}

// Wrong because it is old order
// int main_scanner(){
//     yytoken_kind_t token = Token_none;
//     while(token = yylex()){
//         if(token != Token_comment)
//             printf("%s\t%s\n", yytext, token2string(token));
//     }
//     return 0;
// }

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

int getSymbol(char ch){
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
			return Token_largeBracket_right;
		default:
			printf("This char is %c\n", ch);
			exit(1);
	}
}

void init_linkedlist(){
    if(lexee == 0){
        lexee = (LexeeLinkedlist*)malloc(sizeof(LexeeLinkedlist));
        lexee->head = (Lexee*)malloc(sizeof(Lexee));
        lexee->end = lexee->head;
        Lexee* l = lexee->head;
        l->str = l->token = l->lineno = l->colno = l->next = 0;
    }
}

void insert_lexee(Lexee* l){
    lexee->end->next = l;
    lexee->end = l;
}

Lexee* get_lexee(char* str, char* token, int lineno, int colno){
    Lexee* l = (Lexee*)malloc(sizeof(Lexee));
    l->str = strdup(str);
    l->token = strdup(token);
    l->lineno = lineno;
    l->colno = colno;
    l->next = 0;
    return l;
}



// Wrong order
// char* token2string(yytoken_kind_t t){
//     char* str_token[] = {"Token_if","Token_else","Token_int","Token_void","Token_while","Token_return","Token_plus","Token_minus","Token_multiply","Token_divide","Token_less","Token_lessEqual","Token_more","Token_moreEqual","Token_equal","Token_noEqual","Token_assign","Token_semicolon","Token_comma","Toekn_smallBracket_left","Token_smallBracket_right","Token_middleBracket_left","Token_middleBracket_right","Token_largeBracket_left","Token_largeBracket_right","Token_number","Token_comment","Token_identifier","Token_space", "Token_func_dec", "Token_compound", "Token_var_dec", "Token_para"};
//     return str_token[t - Token_if];
// }