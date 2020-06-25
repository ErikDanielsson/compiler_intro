
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
	EOF
};

struct Token {
	enum TokenType type;
	int line;
	int column;
	union {
		char* string;
		int value;
	};
};

void init_lexer();
struct Token* lexer();
