
#pragma once
enum TokenType {
	NUM = 256,
	ID,
	ICONST,
	FCONST,
	SCONST,
	//Keywords
	UTILIZING,
	PROGRAM,
	FLOAT_TYPE,
	INT_TYPE,
	NAND,
	IF,
	ELIF,
	ELSE,
	WHILE,
	FOR,
	DEFINE,
	RETURN,
	INPUT,
	OUTPUT,

	ASSIGN,
	RELOP,
	SUFFIXOP,

	_EOF
};

struct Token {
	enum TokenType type;
	int line;
	int column;
	char* lexeme;
};

void init_lexer();
struct Token* lexer();
void token_error(int length, char* expected);
