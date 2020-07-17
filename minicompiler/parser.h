#pragma once
#include "lexer.h"

enum NodeType {
    START,
    COMPOUND_STATEMENT,
    STATEMENT,
    VARIABLE_DECLARATION,
    FUNCTION_DECLARATION,
    EMPTY_INDICES,
    PARAMS,
    INDICES,
    VARIABLE_ACCESS,
    EXPR,
    ASSIGNMENT_STATEMENT,
    FUNCTION_CALL,
    ARGS,
    IF_ELIF_ELSE_STATEMENT,
    ELIF_LIST,
    IF_STATEMENT,
    ELIF_STATEMENT,
    ELSE_STATEMENT,
    WHILE_LOOP,
    FOR_LOOP,
    B_EXPR,
    R_EXPR,
    SCOPE,
    TOKEN
};

struct CompStmt {
    int n_statements;
    struct Stmt** statement_list;
};

struct Stmt {
    enum NodeType statement_type;
    union {
        struct VarDecl* variable_declaration;
        struct FuncDecl* function_declaration;
        struct AStmt* assignment_statement;
        struct FuncCall* function_call;
        struct IEEStmt* if_elif_else_statement;
        struct CondStmt* while_loop;
        struct FLoop* for_loop;
        struct CompStmt* scope;
    };
};

struct VarDecl {
    struct Token* type;
    int n_indices;
    struct Expr** indices;
    struct Token* name;
    struct Expr* expr;
};

struct FuncDecl {
    struct Token* type;
    int n_indices;
    struct Token* name;
    int n_params;
    struct VarDecl** params;
    struct CompStmt* body;
};

struct Params {
    int n_params;
    struct VarDecl** params;
};

struct Inds {
    // Empty brackets are assigned NULL
    int n_indices;
    struct Expr** indices;
};

struct VarAcc {
    struct Token* variable;
    int n_indices;
    struct Expr** indices;
};

enum ExprType {
    BINOP,
    UOP,
    CONST,
    FUNCCALL,
    VARACC
};

struct Expr {
    /*
     * This could be optimised for memory, since we don't need a pointer
     * to store a pointer.
     */
    enum ExprType type;
    union {
        struct {
            struct Expr* left;
            struct Token* binary_op;
            struct Expr* right;
        };
        struct {
            struct Token* unary_op;
            struct Expr* expr;
        };
        struct Token* val;
        struct FuncCall* function_call;
        struct VarAcc* variable_access;
    };
};

struct AStmt {
    struct VarAcc* variable_access;
    struct Token* assignment_type;
    struct Expr* expr;
};

struct FuncCall {
    struct Token* func;
    int n_args;
    struct Expr** args;
};

struct Args {
    int n_args;
    struct Expr** args;
};

struct IEEStmt {
    struct CondStmt* if_stmt;
    int n_elifs;
    struct CondStmt** elif_list;
    struct CompStmt* _else;
};

/*
 * Since if, elif and while have the same structure,
 * they can be in the same struct(ure).
 */
struct CondStmt {
    struct BExpr* boolean;
    struct CompStmt* body;
};

struct EList {
    int n_elifs;
    struct CondStmt** elif_list;
    /*
     * Since an 'else' is nothing other than a compound statement executed
     * if all conditional statements are rejected:
     */
    struct CompStmt* _else;
};

struct FLoop {
    enum NodeType type;
    union {
        struct VarDecl* variable_declaration;
        struct AStmt* assignment_statement;
    };
    struct BExpr* boolean;
    struct AStmt* update_statement;
    struct CompStmt* body;
};

struct BExpr {
    enum ExprType type;
    union {
        struct {
            struct BExpr* left;
            struct BExpr* right;
        };
        struct RExpr* r_expr;
    };
};

struct RExpr {
    enum ExprType type;
    union {
        struct {
            struct Expr* left;
            struct Token* operator;
            struct Expr* right;
        };
        struct Expr* expr;
    };
};


struct Record {
    void* value;
    enum NodeType type;
};

void lr_parser(char verbose);
void parser_error(int length, const char* expected,
          int fatal, int line, int column,
          int inject_symbol, char symbol);
struct Token* inject_token(enum TokenType type);

void create_node(void** node_ptr, enum NodeType type);
void create_token_record(struct Record* record_ptr, struct Token* token);
void create_node_record(struct Record** stack, enum NodeType type, int n_pop);
void print_tree(struct CompStmt* tree);

void print_CompStmt(struct CompStmt* node, int nest_level, char labels, char leafs);
void print_Stmt(struct Stmt* node, int nest_level, char labels, char leafs);
void print_VarDecl(struct VarDecl* node, int nest_level, char labels, char leafs);
void print_FuncDecl(struct FuncDecl* node, int nest_level, char labels, char leafs);
void print_VarAcc(struct VarAcc* node, int nest_level, char labels, char leafs);
void print_Expr(struct Expr* node, int nest_level, char labels, char leafs);
void print_AStmt(struct AStmt* node, int nest_level, char labels, char leafs);
void print_FuncCall(struct FuncCall* node, int nest_level, char labels, char leafs);
void print_IEEStmt(struct IEEStmt* node, int nest_level, char labels, char leafs);
void print_CondStmt(struct CondStmt* node, int nest_level, char labels, char leafs);
void print_WLoop(struct CondStmt* node, int nest_level, char labels, char leafs);
void print_FLoop(struct FLoop* node, int nest_level, char labels, char leafs);
void print_BExpr(struct BExpr* node, int nest_level, char labels, char leafs);
void print_RExpr(struct RExpr* node, int nest_level, char labels, char leafs);
