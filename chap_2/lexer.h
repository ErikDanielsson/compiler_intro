#pragma once
typedef enum TokenType {
	NUM = 256,
	ID,
	TRUE,
	FALSE,
} TokenType;

typedef struct Token {
	enum TokenType type;
	union {
		char string[256];
		int value;
	};
} token;

void initLexer();
token* Lexer();
