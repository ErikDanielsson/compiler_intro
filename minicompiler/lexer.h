
#pragma once
#define LINELENGTH 256
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
    STRUCT,
    RETURN,
    ASSIGN,
    RELOP,
    SUFFIXOP
};

struct Token {
    enum TokenType type;
    int line;
    int column;
    union {
        char* lexeme;
        char c_val;
    };

};
extern const char* filename;
extern int file_desc;
extern struct SymTab* keywords;
extern int error_flag;

void init_lexer();
void error(const char* type_msg, int length,
       const char* expected, int fatal,
       int line, int column,
       int inject_symbol, char symbol);
int copy_current_line(char** buffer);
void print_token(struct Token* token);
void print_token_str(struct Token* token);
struct Token* get_token();
