#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "intermediate_code.h"
#include "IC_table.h"
#include "symbol_table.h"
#include "type_checker.h"
#include "io.h"
#include <ctype.h>
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
    new_bb();
}

void new_bb()
{

    int n_blocks = (*top_entry)->n_blocks;
    if (n_blocks) {
        (*top_entry)->basic_block_list[0]->symbol_table = get_curr_symtab();
        print_BasicBlock((*top_entry)->basic_block_list[n_blocks-1], 0);
        struct BasicBlock* tmp_blocks[n_blocks];
        char tmp_blockinfo[n_blocks];
        memcpy(tmp_blocks, (*top_entry)->basic_block_list, sizeof(struct BasicBlock*)*n_blocks);
        //memcpy(tmp_blockinfo, (*top_entry)->blockinfo, sizeof(char)*n_blocks);
        (*top_entry)->basic_block_list = malloc(sizeof(struct BasicBlock*)*(n_blocks+1));
        memcpy((*top_entry)->basic_block_list, tmp_blocks, sizeof(struct BasicBlock*)*n_blocks);
        //memcpy((*top_entry)->blockinfo, tmp_blockinfo, sizeof(char)*n_blocks);
        (*top_entry)->basic_block_list[n_blocks] = malloc(sizeof(struct BasicBlock));
        (*top_entry)->basic_block_list[n_blocks]->bbnum = n_blocks;
        (*top_entry)->basic_block_list[n_blocks]->instructions = malloc(sizeof(struct QuadList));
        (*top_entry)->basic_block_list[n_blocks]->jump_type = -1;
        (*top_entry)->basic_block_list[n_blocks]->jump = NULL;
        (*top_entry)->basic_block_list[n_blocks]->true = NULL;
        (*top_entry)->basic_block_list[n_blocks]->false = NULL;
        *curr_instr_ptr = (*top_entry)->basic_block_list[n_blocks]->instructions;
        (*curr_instr_ptr)->next = NULL;

        (*top_entry)->n_blocks++;
    } else {
        (*top_entry)->basic_block_list = malloc(sizeof(struct BasicBlock*));
        (*top_entry)->basic_block_list[0] = malloc(sizeof(struct BasicBlock));
        (*top_entry)->basic_block_list[0]->bbnum = 0;
        (*top_entry)->basic_block_list[0]->instructions = malloc(sizeof(struct QuadList));
        (*top_entry)->basic_block_list[0]->jump_type = -1;
        (*top_entry)->basic_block_list[0]->jump = NULL;
        (*top_entry)->basic_block_list[0]->true = NULL;
        (*top_entry)->basic_block_list[0]->false = NULL;
        *curr_instr_ptr = (*top_entry)->basic_block_list[n_blocks]->instructions;
        (*curr_instr_ptr)->next = NULL;
        (*top_entry)->n_blocks = 1;
    }
}

void enter_function(char* name)
{
    IC_table_create_entry(intermediate_code, name);
    top_entry++;
    *top_entry = IC_table_get_entry(intermediate_code, name);
    curr_instr_ptr++;
    //s*curr_instr_ptr = (*top_entry)->instructions;
}

void leave_function()
{
    top_entry--;
    curr_instr_ptr--;
}

void append_triple(void* triple, enum QuadType type)
{
    (*curr_instr_ptr)->type = type;
    (*curr_instr_ptr)->instruction = triple;
    (*curr_instr_ptr)->next = malloc(sizeof(struct QuadList));
    *curr_instr_ptr = (*curr_instr_ptr)->next;
    (*curr_instr_ptr)->next = NULL;
}

struct AssignQuad* gen_assignment(char* lval, char* rval)
{
    struct AssignQuad* triple = malloc(sizeof(struct AssignQuad));
    triple->lval = lval;
    /*
     * If the first char of the symbol is a digit, it must be a digit
     */
    if (isdigit(rval[0])) {
        triple->rval_type = CONSTANT;
        triple->rval = rval;
    } else {
        triple->rval = rval;
        triple->rval_type = get_curr_name_entry(rval)->type;
    }
    return triple;
}

struct BinOpQuad* gen_binop(char* op1, enum TokenType op_type, char* op2, char* result)
{
    struct BinOpQuad* triple = malloc(sizeof(struct BinOpQuad));
    triple->result = result;
    struct SymTab_entry* op_entry;
    /*
     * If the first char of the symbol is a digit, it must be a digit
     */
    if (isdigit(op1[0])) {
        triple->op1_type = CONSTANT;
        triple->op1 = op1;
    } else {
        triple->op1 = op1;
        triple->op1_type = get_curr_name_entry(op1)->type;
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
    /*
     * If the first char of the symbol is a digit, it must be a digit
     */
    if (isdigit(op2[0])) {
        triple->op2_type = CONSTANT;
        triple->op2 = op2;
    } else {
        triple->op2 = op2;
        triple->op2_type = get_curr_name_entry(op1)->type;
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
    triple->operand = operand;
    /*
     * If the first char of the symbol is a digit, it must be a digit
     */
    if (isdigit(operand[0])) {
        triple->operand_type = CONSTANT;
    } else {
        triple->operand_type = get_curr_name_entry(operand)->type;
    }
    return triple;
}

struct ConvQuad* gen_conv(char* conversion_type, char* op, char* result)
{
    struct ConvQuad* triple = malloc(sizeof(struct ConvQuad));
    triple->result = result;
    triple->conversion_type = conversion_type;
    /*
     * If the first char of the symbol is a digit, it must be a digit
     */
    if (isdigit(op[0])) {
        triple->op_type = CONSTANT;
        triple->op = op;
    } else {
        triple->op = op;
        triple->op_type = get_curr_name_entry(op)->type;
    }
    return triple;
}

struct CondQuad* gen_cond(char* op1, char* op_lexeme, char* op2, long* label)
{
    struct CondQuad* triple = malloc(sizeof(struct CondQuad));
    struct SymTab_entry* op_entry;
    op_entry = get_curr_name_entry(op1);
    triple->op1_type = op_entry->type;
    triple->op1 = op1;

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
    op_entry = get_curr_name_entry(op2);
    triple->op2_type = op_entry->type;
    triple->op2 = op_entry;
    return triple;
}

void print_BasicBlock(struct BasicBlock* bb, int indent)
{
    print_w_indent(indent, "Block %ld\n", bb->bbnum);
    struct QuadList* curr_instr = bb->instructions;
    for (int i = 0;curr_instr->next != NULL; i++) {
        print_w_indent(indent+1, "%d\t", i);
        switch(curr_instr->type) {
            case QUAD_ASSIGN: {
                struct AssignQuad* assign = curr_instr->instruction;

                printf("%s = %s\n", assign->lval, assign->rval);

                break;
            }
            case QUAD_BINOP: {
                struct BinOpQuad* binop = curr_instr->instruction;
                char* op_arr[] = {"+", "-", "*", "/", "%", "^", "&", "|", ">>", "<<"};
                printf("%s = %s %s %s\n", binop->result, binop->op1, op_arr[binop-> op_type], binop->op2);
                break;
            }
            case QUAD_UOP: {
                struct UOpQuad* uop = curr_instr->instruction;
                char op_arr[] = {'-', '~'};
                printf("%s = %c %s\n", uop->result, op_arr[uop->operator_type], uop->operand);
                break;
            }
            case QUAD_CONV: {
                struct ConvQuad* conv = curr_instr->instruction;
                printf("%s = (%s)%s\n", conv->result, conv->conversion_type, conv->op);
                break;
            }
            case QUAD_COND:
                break;
            case QUAD_UNCOND:
                break;
            case QUAD_RETURN:
                break;
        }
        curr_instr = curr_instr->next;
    }
    printf("\n\n");
}
