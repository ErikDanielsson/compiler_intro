
#pragma once

enum TokenType {
	NUM = 128,
	ID,
	ICONST,
	FCONST,
	SCONST,
	//Keywords
	UTILIZING,
	PROGRAM,
	FTYPE,
	ITYPE,
	STYPE,
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
extern const char* filename;
extern int file_desc;
extern struct SymTab* symbol_table;
extern int error_flag;

void init_lexer();
struct Token* lexer();
void token_error(int length, char* expected);
struct Token* get_token();
