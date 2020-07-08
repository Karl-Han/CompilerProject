#ifndef __token_h
#define __token_h

typedef enum {
	// for all keyword
	Token_if = 128, Token_else, Token_int, 
		Token_void, Token_while, Token_return,
	// for all special token
	Token_plus, Token_minus, Token_multiply, Token_divide,
		Token_less, Token_lessEqual, Token_more, Token_moreEqual,
		Token_equal, Token_noEqual, Token_assign, Token_semicolon,
		Token_comma, Toekn_smallBracket_left, Token_smallBracket_right,
		Token_middleBracket_left, Token_middleBracket_right, 
		Token_largeBracket_left, Token_largeBracket_right,
	// for other token
	Token_number, Token_comment, Token_identifier, Token_space, Token_none
} Token;

char* token2string(Token t){
	char* str_token[] = {"Token_if","Token_else","Token_int","Token_void","Token_while","Token_return","Token_plus","Token_minus","Token_multiply","Token_divide","Token_less","Token_lessEqual","Token_more","Token_moreEqual","Token_equal","Token_noEqual","Token_assign","Token_semicolon","Token_comma","Toekn_smallBracket_left","Token_smallBracket_right","Token_middleBracket_left","Token_middleBracket_right","Token_largeBracket_left","Token_largeBracket_right","Token_number","Token_comment","Token_identifier","Token_space"};
	return str_token[t - Token_if];
}

#endif
