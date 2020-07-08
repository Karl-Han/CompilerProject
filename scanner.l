%{
// Declaration
// Will be copy to lex.yy.c directly
#include "token.h"
int colno = 0, lineno= 1;
Token getSymbol(char);
void comment();
void lex_error(char*);
void newline();
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
{NUMBER}        {colno += yyleng; return Token_number;}
{IDENTIFIER}    {colno += yyleng; return Token_identifier;}
{SPACE}         {colno += yyleng; /* Do nothing */}
{SYMBOL}        {colno += yyleng; return getSymbol(yytext[0]);}
[\n]            {/* Do nothing */ newline();}
"<="            {colno += yyleng; return Token_lessEqual;}
">="            {colno += yyleng; return Token_moreEqual;}
"=="            {colno += yyleng; return Token_equal;}
"!="            {colno += yyleng; return Token_noEqual;}
"/*"            {colno += yyleng; comment(); return Token_comment;}
<<EOF>>         {return 0;}
.               {colno += yyleng; lex_error(yytext);}
%%

void newline(){
    lineno++;
    colno = 0;
}

void lex_error(char* str){
    printf("Lex ERROR in line %d, column %d. %s\n", lineno, colno, str);
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