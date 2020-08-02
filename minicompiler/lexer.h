
#pragma once
#include "consts.h"


enum TokenType {
    NUM = 128,
    ID,
    ICONST,
    FCONST,
    SCONST,
    IF,
    ELIF,
    ELSE,
    WHILE,
    FOR,
    DEFINE,
    STRUCT,
    RETURN,
    ASSIGN,
    RELOP,
    AND,
    OR,
    SHIFT,
    SUFFIXOP
};

struct Token {
    enum TokenType type;
    int line;
    int column;
    union {
        char* lexeme;
        char c_val;
        long i_val;
        double f_val;
    };
};
struct Line {
    char line[LINELENGTH];
    int num;
};

extern const char* filename;
extern int file_desc;
extern struct KeywordTab* keywords;
extern int error_flag;

void init_lexer();
void error(const char* type_msg, int length,
       const char* expected, int fatal,
       int line, int column,
       int inject_symbol, char symbol);
void copy_current_line(struct Line* buffer);
void print_token(struct Token* token);
void print_token_str(struct Token* token);
struct Token* get_token();
