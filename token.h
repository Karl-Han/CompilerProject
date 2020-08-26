#ifndef __token_h
#define __token_h

#include <stdlib.h>

#define MAX_CHILDREN 4

typedef enum {
    StmtK,
    ExpK
}NodeKind;

typedef struct _TreeNode
{
    // this int is the same as in y.tab.h
    int token;
    int num;
    // used to store identifier's string
    char *str;

    int lineno;

    struct _TreeNode *sibling;
    struct _TreeNode *child[MAX_CHILDREN];
    // used to seperate different token of the same kind
    char *name;

    // node expression type for type check
    // 0 for void, 1 for integer, 2 for array
    int type;
    NodeKind nk;
} TreeNode;

void init_sib_child(TreeNode *);
// TreeNode section
TreeNode *getTreeNode(int);
TreeNode *getTreeNode_number(int, int);
TreeNode *getTreeNode_identifier(int, char *);
#define tokens_offset 258

extern char *tokens[];

char* token2char(int);

// typedef enum {
// 	// for all keyword
// 	Token_if = 128, Token_else, Token_int,
// 		Token_void, Token_while, Token_return,
// 	// for all special token
// 	Token_plus, Token_minus, Token_multiply, Token_divide,
// 		Token_less, Token_lessEqual, Token_more, Token_moreEqual,
// 		Token_equal, Token_noEqual, Token_assign, Token_semicolon,
// 		Token_comma, Toekn_smallBracket_left, Token_smallBracket_right,
// 		Token_middleBracket_left, Token_middleBracket_right,
// 		Token_largeBracket_left, Token_largeBracket_right,
// 	// for other token
// 	Token_number, Token_comment, Token_identifier, Token_space, Token_none,
// 	// new tokens in parser
// 	Token_func_dec, Token_compound, Token_var_dec, Token_para
// } Token;
#endif
