#pragma once
char* get_temp();
struct triple {
    int type;
    /*
     * Arguments could either be literals, pointers to symbol table or
     * pointer to another triple struct
     */
    void* arg1;
    void* arg2;
};
struct triple_list {
    int* types;
    void** args1;
    void** args2;
};

void generate_IC(struct CompStmt* node);
void emit(char* instr);
void emitlabel(char* label);

void visit_CompStmt(struct CompStmt* node);
void visit_Stmt(struct Stmt* node);
void visit_VarDecl(struct VarDecl* node);
void visit_StructDecl(struct StructDecl* node);
void visit_FuncDecl(struct FuncDecl* node);
void visit_VarAcc(struct VarAcc* node);
void visit_Expr(struct Expr* node);
void visit_AStmt(struct AStmt* node);
void visit_FuncCall(struct FuncCall* node);
void visit_IEEStmt(struct IEEStmt* node);
void visit_CondStmt(struct CondStmt* node);
void visit_WLoop(struct CondStmt* node);
void visit_FLoop(struct FLoop* node);
void visit_ReturnStmt(struct Expr* node);
