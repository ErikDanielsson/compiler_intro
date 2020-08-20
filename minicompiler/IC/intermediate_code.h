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
    QUAD_RETURN,
    QUAD_PARAM,
    QUAD_FUNC
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
    void* instruction;
    struct QuadList* next;
    enum QuadType type;
    /*
     * Flags:
     * 1: Denotes whether we are allowed to optimize away
     * the temporary the result is assigned to
     * 2:
     * 3:
     * ...
     */
    char flags;

};

struct AssignQuad {
    /*
     * The lvalue of a 'real' assingment must be a variable
     */
    struct SymTab_entry* lval;
    unsigned long lval_info;
    enum SymbolType rval_type;
    void* rval;
    unsigned long rval_info;
};

enum BinOpType {
    BINOP_PLUS,
    BINOP_MINUS,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_AND,
    BINOP_OR,
    BINOP_XOR,
    BINOP_SHR,
    BINOP_SHL,
};

struct BinOpQuad {
    struct SymTab_entry* result;
    unsigned long result_info;
    enum SymbolType op1_type;
    void* op1;
    unsigned long op1_info;
    enum BinOpType op_type;
    enum SymbolType op2_type;
    void* op2;
    unsigned long op2_info;
};
enum UOpType {
    UOP_NEG,
    UOP_NOT
};

struct UOpQuad {
    struct SymTab_entry* result;
    unsigned long result_info;
    enum UOpType operator_type;
    enum SymbolType operand_type;
    void* operand;
    unsigned long operand_info;
};

struct ConvQuad {
    struct SymTab_entry* result;
    unsigned long result_info;
    unsigned long conversion_type;
    enum SymbolType op_type;
    void* op;
    unsigned long op_info;
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
    unsigned long op1_info;
    enum RelopType op_type;
    enum SymbolType op2_type;
    void*  op2;
    unsigned long op2_info;
};

struct RetQuad {
    /*
     * Since a return instructions means jumping to the epilogue
     * of the function, i.e register restoration and stack cleanup,
     * the return statement is give its own instruction. The target of
     * the jump is determined during code generation.
     */
    enum SymbolType type;
    void*  ret_val;
    unsigned long ret_val_info;
};

struct ParamQuad {
    enum SymbolType type;
    void* op;
    unsigned long op_info;
};
struct FuncCQuad {
    struct SymTab_entry* lval;
    unsigned long lval_info;
    char* name;
};

struct BasicBlock** newlabel();
void append_label(struct BasicBlock** new);




extern struct BasicBlock** curr_block;
void print_CFG();


void init_IC_generator();
extern struct IC_table* intermediate_code;
struct BasicBlock* new_bb();

void set_uncond_target(struct BasicBlock** target_addr);
void set_cond_and_targets(struct CondQuad* cond, struct BasicBlock** true_addr, struct BasicBlock** false_addr);

void enter_function(char* name);
void leave_function();
void append_triple(void* triple, enum QuadType type, char flags);
struct AssignQuad* gen_assignment(struct SymTab_entry* lval,  void* rval, enum SymbolType rval_type);
struct BinOpQuad* gen_binop(void* op1, enum SymbolType op1_type, enum TokenType op_type, void* op2, enum SymbolType op2_type, struct SymTab_entry*  result);
struct UOpQuad* gen_uop(void* operand, enum SymbolType operand_type, enum TokenType operator, struct SymTab_entry*  result);
struct ConvQuad* gen_conv(char* conversion_type, void* op, enum SymbolType op_type, struct SymTab_entry*  result);
struct CondQuad* gen_cond(void* op1, enum SymbolType op1_type, char* op_lexeme, void* op2, enum SymbolType op2_type);
struct ParamQuad* gen_param(void* op, enum SymbolType type);
struct FuncCQuad* gen_funccall(struct SymTab_entry* lval, char* name);
struct RetQuad* gen_return(void* ret_val, enum SymbolType type);

void destroy_CFG();
