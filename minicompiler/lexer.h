
#pragma once
enum TokenType {
	NUM = 256,
	ID,
	INT,
	FLOAT,
	IF,
	ELSE,
	WHILE,
	FOR,
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
