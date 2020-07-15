#pragma once
#include "lexer.h"

enum NodeType {
	START,
	COMPOUND_STATEMENT,
	STATEMENT,
	VARIABLE_DECLARATION,
	INDEX_EXPR,
	FUNCTION_DECLARATION,
	PARAMS,
	PARAM_DECL,
	TYPE,
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
	R_EXPR
};

struct Start {
	struct CompStmt* compound_statement;
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
		struct AsStmt* assignment_statement;
		struct FuncCall* function_call;
		struct IIEStmt* if_elif_else_statement;
		struct WLoop* while_loop;
		struct FLoop* for_loop;
	};
};

struct VarDecl {
	struct Token* type;
	int n_indices;
	int* indices;
	struct Token* name;
	struct Expr* expr;
};

struct FuncDecl {
	struct Token* type;
	int n_indices;
	int* indices;
	struct Token* name;
	int n_params;
	struct VarDecl** params;
	struct CompStmt* body;
};

struct VarAcc {
	Token* variable;
	int n_indices;
	int* indices;
};

enum ExprType {
	BINOP,
	UOP,
	NUM,
	FUNCCALL
	VARACC
};

struct Expr{
	enum ExprType type;
	union {
		struct {
			struct Expr* left;
			enum TokenType operator;
			struct Expr* right;
		};
		struct {
			enum TokenType operator;
			struct Expr* expr;
		};
		struct Token* num_val;
		struct FuncCall* function_call;
		struct VarAcc* variable_access;
	};
};
enum AssignType {
	S_OP,
	OTHER
};
struct AsStmt {
	struct VarAcc* variable_access;
	struct Token* assignment_type;
	struct Expr* expr;
};

struct FuncCall {
	struct Token* func;
	int n_args;
	struct Expr** args;
};

struct IIEStmt {
	int n_conditionals;
	struct IfStmt** conditional_list;
	/*
	 * Since an else is nothing other than a compound statement executed
	 * if all conditional statement are rejected:
	 */
	struct CompStmt* unconditional;
};

struct IfStmt {
	struct BExpr* boolean;
	struct CompStmt* body;
};

struct WLoop {
	struct BExpr* boolean;
	struct CompStmt* body;
};
struct FLoop {
	struct VarDecl* variable_declaration;
	struct BExpr* boolean;
	struct AsStmt* update_statement;
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
	struct Expr* left;
	struct Token* operator;
	struct Expr* right;
};

void lr_parser(char verbose);
void parser_error(int length, const char* expected,
		  int fatal, int line, int column,
		  int inject_symbol, char symbol);
struct Token* inject_token(enum TokenType type);
