#pragma once
#include "intermediate_code.h"
#include "symbol_table.h"

struct AddrTypePair {
    struct SymTab_entry* addr;
    enum SymbolType type;
};

void generate_IC(struct CompStmt* node);
void emit(char* instr, ...);
void emitlabel(char* label);

void visit_CompStmt(struct CompStmt* node);
void visit_Stmt(struct Stmt* node);
void visit_VarDecl(struct VarDecl* node);
void visit_StructDecl(struct StructDecl* node);
void visit_FuncDecl(struct FuncDecl* node);
char* visit_VarAcc(struct VarAcc* node);
char* visit_Expr_rval(struct Expr* node);
void visit_Expr_jump(struct Expr* node);
void visit_AStmt(struct AStmt* node);
char* visit_FuncCall(struct FuncCall* node);
void visit_IEEStmt(struct IEEStmt* node);
struct BasicBlock** if_with_else(struct CondStmt* node, struct BasicBlock** next, struct BasicBlock** prev_false);
void visit_WLoop(struct CondStmt* node);
void visit_FLoop(struct FLoop* node);
void visit_ReturnStmt(struct Expr* node);
