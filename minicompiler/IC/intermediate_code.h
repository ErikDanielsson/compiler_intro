#pragma once
#include "lexer.h"
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

void init_IC_generator();
void enter_function(char* name);
void leave_function();
void append_triple(void* triple, enum QuadType type);
struct AssignQuad* gen_assignment(enum OperandType lval_type,  char* lval,
                    enum OperandType rval_type, char* rval);
struct BinOpQuad* gen_binop(char* op1, enum TokenType op_type, char* op2, char* result);
struct UOpQuad* gen_uop(char* operand, enum TokenType operator, char* result);
struct ConvQuad* gen_conv(char* conversion_type, char* op, char* result);
struct CondQuad* gen_cond(char* op1, char* op_lexeme, char* op2, long* label);
struct UncondQuad* gen_uncond(long* label);
