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
struct BasicBlock* block_stack[2];
struct BasicBlock** curr_block = block_stack;
struct QuadList* instr_stack[2];
struct QuadList** curr_instr_ptr = instr_stack;


void init_IC_generator()
{
    intermediate_code = create_IC_table(IC_TABLE_SIZE);
    IC_table_create_entry(intermediate_code, "main");
    *top_entry = IC_table_get_entry(intermediate_code, "main");
    new_bb();
}

struct BasicBlock* new_bb()
{

    int n_blocks = (*top_entry)->n_blocks;
    if (n_blocks) {
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
        *curr_block = (*top_entry)->basic_block_list[n_blocks];
        return *curr_block;
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
        *curr_block = (*top_entry)->basic_block_list[0];
        return *curr_block;
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

void set_uncond_target(struct BasicBlock** target_addr)
{
    (*curr_block)->jump_type = QUAD_UNCOND;
    (*curr_block)->jump = target_addr;
}

void set_cond_and_targets(struct CondQuad* cond, struct BasicBlock** true_addr, struct BasicBlock** false_addr)
{
    (*curr_block)->jump_type = QUAD_COND;
    (*curr_block)->condition = cond;
    (*curr_block)->true = true_addr;
    (*curr_block)->false = false_addr;
}

struct AssignQuad* gen_assignment(struct SymTab* lval,  void* rval, enum SymbolType rval_type)
{
    struct AssignQuad* triple = malloc(sizeof(struct AssignQuad));
    triple->lval = lval;
    triple->rval_type = rval_type;
    triple->rval = rval;
    return triple;
}

struct BinOpQuad* gen_binop(void* op1, enum SymbolType op1_type, enum TokenType op_type, void* op2, enum SymbolType op2_type, struct SymTab_entry* result)
{
    struct BinOpQuad* triple = malloc(sizeof(struct BinOpQuad));
    triple->result = result;
    triple->op1_type = op1_type;
    triple->op1 = op1;
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
    triple->op2_type = op2_type;
    triple->op2 = op2;
    return triple;
}

struct UOpQuad* gen_uop(void* operand, enum SymbolType operand_type, enum TokenType operator, struct SymTab_entry*  result)
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
    triple->operand_type = operand_type;
    triple->operand = operand;
    return triple;
}

struct ConvQuad* gen_conv(char* conversion_type, void* op, enum SymbolType op_type, struct SymTab_entry*  result)
{
    struct ConvQuad* triple = malloc(sizeof(struct ConvQuad));
    triple->result = result;
    triple->conversion_type = conversion_type;
    triple->op_type = op_type;
    triple->op = op;
    return triple;
}

struct CondQuad* gen_cond(void* op1, enum SymbolType op1_type, char* op_lexeme, void* op2, enum SymbolType op2_type)
{
    struct CondQuad* triple = malloc(sizeof(struct CondQuad));
    triple->op1_type = op1_type;
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
    triple->op2_type = op2_type;
    triple->op2 = op2;
    return triple;
}
int gen_done = FALSE;
void print_BasicBlock(struct BasicBlock* bb, int indent)
{
    print_w_indent(indent, "Block %ld\n", bb->bbnum);
    struct QuadList* curr_instr = bb->instructions;
    for (int i = 0;curr_instr->next != NULL; i++) {
        print_w_indent(indent+1, "%d\t", i);
        switch(curr_instr->type) {
            case QUAD_ASSIGN: {
                struct AssignQuad* assign = curr_instr->instruction;

                printf("%s = ", assign->lval->key, assign->rval);
                if (assign->rval_type == CONSTANT)
                    printf("%s\n", assign->rval);
                else
                    printf("%s\n", ((struct SymTab_entry*)(assign->rval))->key);
                break;
            }
            case QUAD_BINOP: {
                struct BinOpQuad* binop = curr_instr->instruction;
                char* op_arr[] = {"+", "-", "*", "/", "%", "^", "&", "|", ">>", "<<"};
                printf("%s = ", binop->result->key);
                if (binop->op1_type == CONSTANT)
                    printf("%s ", binop->op1);
                else
                    printf("%s ", ((struct SymTab_entry*)(binop->op1))->key);
                printf("%s ", op_arr[binop-> op_type]);
                if (binop->op2_type == CONSTANT)
                    printf("%s\n", binop->op2);
                else
                    printf("%s\n", ((struct SymTab_entry*)(binop->op2))->key);
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
                printf("%s = (%s)", conv->result->key, conv->conversion_type);
                if (conv->op_type == CONSTANT)
                    printf("%s\n", conv->op);
                else
                    printf("%s\n", ((struct SymTab_entry*)(conv->op))->key);

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
    if (bb->jump_type == QUAD_COND)
    {

        struct CondQuad* cond = bb->condition;
        if (cond->op1_type == CONSTANT)
            print_w_indent(indent+1, "%s, ", cond->op1);
        else
            print_w_indent(indent+1, "%s, ", ((struct SymTab_entry*)(cond->op1))->key);

        printf("op: %d, ", cond->op_type);
        if (cond->op2_type == CONSTANT)
            printf("%s\n", cond->op2);
        else
            printf("%s\n", ((struct SymTab_entry*)(cond->op2))->key);
    } else {
        print_w_indent(indent+1, "uncond");
    }
    printf("\n");
}

void printr(struct BasicBlock** bb, int indent, long max_bb)
{
    print_BasicBlock(*bb, indent);
    if ((*bb)->bbnum == max_bb-1)
        return;
    if ((*bb)->jump_type == QUAD_COND) {
        print_w_indent(indent, "true: \n");
        printr((*bb)->true, indent+1, max_bb);
        print_w_indent(indent, "false: \n");
        printr((*bb)->false, indent+1, max_bb);
    } else {
        printr((*bb)->jump, indent+1, max_bb);
    }
}

void with_childs(struct IC_entry* entry)
{
    printr(entry->basic_block_list, 0, entry->n_blocks);
}
