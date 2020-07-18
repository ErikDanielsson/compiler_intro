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
    void* stmt;
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
    void* init_stmt;
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

void lr_parser(char verbose);
void parser_error(int length, const char* expected,
          int fatal, int line, int column,
          int inject_symbol, char symbol);
struct Token* inject_token(enum TokenType type);

static inline void create_token_record(void*** record_ptr, struct Token* token);
void create_node_record(void*** stack, int n_pop);

static inline void reduce_to_compound_compound_list(void*** top);
// 2
static inline void reduce_to_compound_statement(void*** top);
// 3
static inline void reduce_to_stmt_vardecl(void*** top);
// 4
static inline void reduce_to_stmt_funcdecl_(void*** top);
// 5
static inline void reduce_to_stmt_assignment_statement(void*** top);
// 6
static inline void reduce_to_stmt_funccall(void*** top);
// 7
static inline void reduce_to_stmt_ieestmt(void*** top);
// 8
static inline void reduce_to_stmt_wloop(void*** top);
//9
static inline void reduce_to_stmt_floop(void*** top);
// 10
static inline void reduce_to_stmt_scope(void*** top);
// 11
static inline void reduce_to_vardecl_w_ind(void*** top);
// 12
static inline void reduce_to_vardecl_w_ind_n_expr(void*** top);
// 13
static inline void reduce_to_vardecl(void*** top);
// 14
static inline void reduce_to_vardecl_w_expr(void*** top);
// 15
static inline void reduce_to_func_decl_w_ind_n_params(void*** top);
// 16
static inline void reduce_to_func_decl_w_ind(void*** top);
// 17
static inline void reduce_to_func_decl_w_params(void*** top);
// 18
static inline void reduce_to_func_decl(void*** top);
// 19
static inline void reduce_to_empty_ind_list(void*** top);
//20
static inline void reduce_to_empty_ind(void*** top);
// 21
static inline void reduce_to_param_list(void*** top);
// 22
static inline void reduce_to_param(void*** top);
// 23
static inline void reduce_to_ind_list_w_expr(void*** top);
//24
static inline void reduce_to_ind_w_expr(void*** top);
//25
static inline void reduce_to_ind_list(void*** top);
// 26
static inline void reduce_to_ind(void*** top);
// 27
static inline void reduce_to_varacc(void*** top);
// 28
static inline void reduce_to_varacc_w_ind(void*** top);
// 29, 30, 31, 32, 33, 34
static inline void reduce_to_expr_binop(void*** top);
// 35
static inline void reduce_to_expr_paren(void*** top);
// 36, 37, 38
static inline void reduce_to_expr_const(void*** top);
// 39
static inline void reduce_to_expr_varacc(void*** top);
// 40
static inline void reduce_to_expr_funccall(void*** top);
// 41, 42
static inline void reduce_to_expr_unary(void*** top);
// 43, 44
static inline void reduce_to_assign(void*** top);
// 45
static inline void reduce_to_assign_suffixop(void*** top);
// 46
static inline void reduce_to_funccall_w_args(void*** top);
// 47
static inline void reduce_to_funccall(void*** top);
// 48
static inline void reduce_to_args_args(void*** top);
// 49
static inline void reduce_to_args_expr(void*** top);
// 50
static inline void reduce_to_ieestmt_ifstmt(void*** top);
// 51
static inline void reduce_to_ieestmt_eliflist(void*** top);
// 52
static inline void reduce_to_eliflist_eliflist(void*** top);
// 53
static inline void reduce_to_eliflist_elif(void*** top);
// 54
static inline void reduce_to_eliflist_else(void*** top);
// 55, 56, 58
static inline void reduce_to_cond(void*** top);
// 57
static inline void reduce_to_else(void*** top);
// 59
static inline void reduce_to_for_vardecl(void*** top);
// 60
static inline void reduce_to_for_assign(void*** top);
// 61
static inline void reduce_to_bexpr_binop(void*** top);
// 62
static inline void reduce_to_bexpr_binop_w_paren(void*** top);
// 63
static inline void reduce_to_b_expr_r_expr(void*** top);
// 64
static inline void reduce_to_bexpr_bexpr_w_paren(void*** top);
// 65
static inline void reduce_to_rexpr_binop(void*** top);
// 66
static inline void reduce_to_rexpr_expr(void*** top);
// 67
static inline void reduce_to_scope(void*** top);

void write_indent(int nest_level);
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
