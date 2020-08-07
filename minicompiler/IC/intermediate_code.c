#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intermediate_code.h"
#include "IC_table.h"
#include "lexer.h"
#include "type_checker.h"
long vertex_num = 0;
long* new_vertex()
{
    long* label = malloc(sizeof(long));
    *label = vertex_num;
    vertex_num++;
    return label;
}

#define IC_TABLE_SIZE 97
struct IC_table* intermediate_code;
/*
 * Since functions can only be nested in main we only need a stack of size 2
 */
struct IC_entry* entry_stack[2];
struct IC_entry** top_entry = entry_stack;
struct QuadList* instr_stack[2];
struct QuadList** curr_instr_ptr = instr_stack;


void init_IC_generator()
{
    intermediate_code = create_IC_table(IC_TABLE_SIZE);
    IC_table_create_entry(intermediate_code, "main");
    *top_entry = IC_table_get_entry(intermediate_code, "main");
    *curr_instr_ptr = (*top_entry)->instruction_list;
}

void enter_function(char* name)
{
    IC_table_create_entry(intermediate_code, name);
    top_entry++;
    *top_entry = IC_table_get_entry(intermediate_code, name);
    curr_instr_ptr++;
    *curr_instr_ptr = (*top_entry)->instruction_list;
}

void leave_function()
{
    top_entry--;
    curr_instr_ptr--;
}

void append_triple(void* triple, enum QuadType type)
{
    *curr_instr_ptr = malloc(sizeof(struct QuadList));
    (*curr_instr_ptr)->type = type;
    (*curr_instr_ptr)->instruction = triple;
    *curr_instr_ptr = (*curr_instr_ptr)->next;
    *curr_instr_ptr = NULL;
}

struct AssignQuad* gen_assignment(enum OperandType lval_type,  char* lval,
                    enum OperandType rval_type, char* rval)
{
    struct AssignQuad* triple = malloc(sizeof(struct AssignQuad));
    triple->lval = get_curr_name_entry(lval);
    triple->rval_type = rval_type;
    triple->rval = rval;
    return triple;
}

struct BinOpQuad* gen_binop(char* op1, enum TokenType op_type, char* op2, char* result)
{
    struct BinOpQuad* triple = malloc(sizeof(struct BinOpQuad));
    triple->result = result;
    struct SymTab_entry* op_entry;
    if ((op_entry = get_curr_name_entry(op1)) == NULL) {
        triple->op1_type = OPERAND_TEMP;
        triple->op1 = op1;
    } else {
        triple->op1_type = OPERAND_VAR;
        triple->op1 = op_entry;
    }
    switch (op_type) {
        case '+':
            triple->op_type = BINOP_PLUS;
            break;
        case '-':
            triple->op_type = BINOP_MINUS;
            break;
        case '*':
            triple->op_type = BINOP_MUL;
            break;
        case '/':
            triple->op_type = BINOP_DIV;
            break;
        case '%':
            triple->op_type = BINOP_MOD;
            break;
        case '^':
            triple->op_type = BINOP_XOR;
            break;
        case '&':
            triple->op_type = BINOP_AND;
            break;
        case '|':
            triple->op_type = BINOP_OR;
            break;
        case SHR:
            triple->op_type = BINOP_SHR;
            break;
        case SHL:
            triple->op_type = BINOP_SHL;
            break;
        default:
            fprintf(stderr,
                "Internal error:Did not expect TokenType %d in BinOpQuad construction",
                op_type);
            exit(-1);
    }
    if ((op_entry = get_curr_name_entry(op2)) == NULL) {
        triple->op2_type = OPERAND_TEMP;
        triple->op2 = op2;
    } else {
        triple->op2_type = OPERAND_VAR;
        triple->op2 = op_entry;
    }
    return triple;
}

struct UOpQuad* gen_uop(char* operand, enum TokenType operator, char* result)
{
    struct UOpQuad* triple = malloc(sizeof(struct UOpQuad));
    triple->result = result;
    switch (operator) {
        case '-':
            triple->operator_type = UOP_NEG;
            break;
        case '~':
            triple->operator_type = UOP_NOT;
            break;
        default:
            fprintf(stderr,
                "Internal error:Did not expect TokenType %d in UOpQuad construction",
                operator);
            exit(-1);
    }
    struct SymTab_entry* op_entry;
    if ((op_entry = get_curr_name_entry(operand)) == NULL) {
        triple->operand_type = OPERAND_TEMP;
        triple->operand = operand;
    } else {
        triple->operand_type = OPERAND_VAR;
        triple->operand = op_entry;
    }
    return triple;
}

struct ConvQuad* gen_conv(char* conversion_type, char* op, char* result)
{
    struct ConvQuad* triple = malloc(sizeof(struct ConvQuad));
    triple->result = result;
    triple->conversion_type = conversion_type;
    struct SymTab_entry* op_entry;
    if ((op_entry = get_curr_name_entry(op)) == NULL) {
        triple->op_type = OPERAND_TEMP;
        triple->op = op;
    } else {
        triple->op_type = OPERAND_VAR;
        triple->op = op_entry;
    }
    return triple;
}

struct CondQuad* gen_cond(char* op1, char* op_lexeme, char* op2, long* label)
{
    struct CondQuad* triple = malloc(sizeof(struct CondQuad));
    struct SymTab_entry* op_entry;
    if ((op_entry = get_curr_name_entry(op1)) == NULL) {
        triple->op1_type = OPERAND_TEMP;
        triple->op1 = op1;
    } else {
        triple->op1_type = OPERAND_VAR;
        triple->op1 = op_entry;
    }
    switch (op_lexeme[0]) {
        case '<':
            if (strlen(op_lexeme) == 2)
                triple->op_type = RELOP_LESS_EQ;
            else
                triple->op_type = RELOP_LESS;
            break;
        case '>':
            if (strlen(op_lexeme) == 2)
                triple->op_type = RELOP_MORE_EQ;
            else
                triple->op_type = RELOP_MORE;
            break;
        case '=':
            triple->op_type = RELOP_EQ;
            break;
        case '!':
            triple->op_type = RELOP_NOT_EQ;
            break;
        default:
            fprintf(stderr,
                "Internal error:Did not expect lexeme %s in CondQuad construction",
                op_lexeme);
            exit(-1);
    }
    if ((op_entry = get_curr_name_entry(op2)) == NULL) {
        triple->op2_type = OPERAND_TEMP;
        triple->op2 = op2;
    } else {
        triple->op2_type = OPERAND_VAR;
        triple->op2 = op_entry;
    }
    triple->label = label;
    return triple;
}

struct UncondQuad* gen_uncond(long* label)
{
    struct UncondQuad* triple = malloc(sizeof(struct UncondQuad));
    triple->label = label;
    return triple;
}
