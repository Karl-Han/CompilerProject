#ifndef __lexee_h
#define __lexee_h

typedef struct _Lexee
{
    // token's literal string
    char *str;
    // Token type yytoken_kind_t in int
    char *token;
    // token's line number
    int lineno;
    // token's column number
    int colno;
    struct _Lexee *next;
} Lexee;

typedef struct _LexeeLinkedlist
{
    Lexee *head;
    Lexee *end;
} LexeeLinkedlist;

extern LexeeLinkedlist *lexee;

void init_linkedlist();
void insert_lexee(Lexee *l);
Lexee *get_lexee(char *str, char *token, int lineno, int colno);
#endif