#pragma once
#include "lexer.h"
#include "symbol_table.h"
#include "IC_table.h"

enum QuadType {
    QUAD_ASSIGN,
    QUAD_BINOP,
    QUAD_UOP,
    QUAD_CONV,
    QUAD_COND,
    QUAD_UNCOND,
    QUAD_RETURN
};

struct BasicBlock {
    long bbnum;
    struct QuadList* instructions;
    /*
     * A basic block can at most contain one jump
     */
    enum QuadType jump_type;
    union {
        struct BasicBlock** jump;
        struct {
            struct CondQuad* condition;
            struct BasicBlock** true;
            struct BasicBlock** false;
        };
    };


};

struct QuadList {
    enum QuadType type;
    void* instruction;
    struct QuadList* next;
};

struct AssignQuad {
    /*
     * The lvalue of a 'real' assingment must be a variable
     */
    struct SymTab_entry* lval;
    enum SymbolType rval_type;
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
    struct SymTab_entry* result;
    enum SymbolType op1_type;
    void* op1;
    enum BinOpType op_type;
    enum SymbolType op2_type;
    void* op2;
};

struct ConvQuad {
    struct SymTab_entry* result;
    char* conversion_type;
    enum SymbolType op_type;
    void* op;
};

enum UOpType {
    UOP_NEG,
    UOP_NOT
};
struct UOpQuad {
    struct SymTab_entry* result;
    enum UOpType operator_type;
    enum SymbolType operand_type;
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
    enum SymbolType op1_type;
    void*  op1;
    enum RelopType op_type;
    enum SymbolType op2_type;
    void*  op2;
    /*
     * Before construction of basic blocks and control flow graph (CFG),
     * the jump target is simply a pointer to an index of the instruction
     * pointer array. When the CFG is constructed, we change this to be a
     * pointer to another basic block
     */
};

struct RetQuad {
    /*
     * Since a return instructions means jumping to the epilogue
     * of the function, i.e register restoration and stack cleanup,
     * the return statement is give its own instruction. The target of
     * the jump is determined during code generation.
     */
    enum SymbolType ret_val_type;
    void*  op;
};

extern struct BasicBlock** curr_block;
extern struct IC_table* intermediate_code;
void with_childs(struct IC_entry* entry);
void set_cond_and_targets(struct CondQuad* cond, struct BasicBlock** true_addr, struct BasicBlock** false_addr);


void init_IC_generator();
struct BasicBlock* new_bb();
void enter_function(char* name);
void leave_function();
void append_triple(void* triple, enum QuadType type);
struct AssignQuad* gen_assignment(struct SymTab* lval,  void* rval, enum SymbolType rval_type);
struct BinOpQuad* gen_binop(void* op1, enum SymbolType op1_type, enum TokenType op_type, void* op2, enum SymbolType op2_type, struct SymTab_entry*  result);
struct UOpQuad* gen_uop(void* operand, enum SymbolType operand_type, enum TokenType operator, struct SymTab_entry*  result);
struct ConvQuad* gen_conv(char* conversion_type, void* op, enum SymbolType op_type, struct SymTab_entry*  result);
struct CondQuad* gen_cond(void* op1, enum SymbolType op1_type, char* op_lexeme, void* op2, enum SymbolType op2_type);
struct UncondQuad* gen_uncond(long* label);

void print_BasicBlock(struct BasicBlock* bb, int indent);
