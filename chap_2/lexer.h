#pragma once
enum TokenType {
	NUM = 256,
	ID,
	TRUE,
	FALSE,
};

struct Token {
	enum TokenType type;
	int line;
	int column;
	union {
		char string[256];
		int value;
	};
};

void init_lexer();
struct Token* lexer();
