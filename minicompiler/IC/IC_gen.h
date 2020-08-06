#pragma once

enum QuadType {
    QUAD_ASSIGN,
    QUAD_BINOP,
    QUAD_UOP,
    QUAD_CONV,
    QUAD_COND,
    QUAD_UNCOND,
    QUAD_RETURN
};

struct QuadList {
    enum QuadType type;
    void* instruction;
    struct QuadList* next;
};

enum OperandType {
    OPERAND_TEMP,
    OPERAND_VAR
};

struct AssignQuad {
    /*
     * The lvalue of a 'real' assingment, must be a variable
     */
    void* lval;
    enum OperandType rval_type;
    void* rval;
};
enum BinOpType {
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_XOR,
    BINOP_AND,
    BINOP_OR,
    BINOP_SHR,
    BINOP_SHL,
};
struct BinOpQuad {
    char* result;
    enum OperandType op1_type;
    void* op1;
    enum BinOpType op_type;
    enum OperandType op2_type;
    void* op2;
};

struct ConvQuad {
    char* result;
    char* conversion_type;
    enum OperandType op_type;
    void* op;
};

enum UOpType {
    UOP_NEG,
    UOP_NOT,
};
struct UOpQuad {
    char* result;
    enum UOpType operator_type;
    enum OperandType operand_type;
    void* operand;
};

enum RelopType {
    RELOP_LESS,
    RELOP_MORE,
    RELOP_LESS_EQ,
    RELOP_MORE_EQ,
    RELOP_EQ,
    RELOP_NOT_EQ
};

struct CondQuad {
    enum OperandType op1_type;
    void* op1;
    enum RelopType op_type;
    enum OperandType op2_type;
    void* op2;
    /*
     * Before construction of basic blocks and control flow graph (CFG),
     * the jump target is simply a pointer to an index of the instruction
     * pointer array. When the CFG is constructed, we change this to be a
     * pointer to another basic block
     */
    long* label;
};

struct UncondQuad {
    long* label;
};

struct RetQuad {
    /*
     * Since a return instructions means jumping to the epilogue
     * of the function, i.e register restoration and stack cleanup,
     * the return statement is give its own instruction. The target of
     * the jump is determined during code generation.
     */
    enum OperandType ret_val_type;
    void* op;
    long* cleanup_label;
};






struct AddrPair {
    enum OperandType type;
    void* addr;
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
void if_with_else(struct CondStmt* node, char* next);
void visit_WLoop(struct CondStmt* node);
void visit_FLoop(struct FLoop* node);
void visit_ReturnStmt(struct Expr* node);
