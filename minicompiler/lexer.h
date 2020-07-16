
#pragma once

enum TokenType {
    NUM = 128,
    ID,
    ICONST,
    FCONST,
    SCONST,
    NAND,
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOR,
    DEFINE,
    RETURN,
    ASSIGN,
    RELOP,
    SUFFIXOP
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
void error(const char* type_msg, int length,
       const char* expected, int fatal,
       int line, int column,
       int inject_symbol, char symbol);
struct Token* get_token();
