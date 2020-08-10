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
        struct BasicBlock* tmp_blocks[n_blocks];
        char tmp_blockinfo[n_blocks];
        memcpy(tmp_blocks, (*top_entry)->basic_block_list, sizeof(struct BasicBlock*)*n_blocks);
        (*top_entry)->basic_block_list = malloc(sizeof(struct BasicBlock*)*(n_blocks+1));
        memcpy((*top_entry)->basic_block_list, tmp_blocks, sizeof(struct BasicBlock*)*n_blocks);
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
    new_bb();
}

void leave_function()
{
    top_entry--;
    curr_instr_ptr--;
}

void append_triple(void* triple, enum QuadType type, char init_flags)
{
    (*curr_instr_ptr)->type = type;
    (*curr_instr_ptr)->instruction = triple;
    (*curr_instr_ptr)->next = malloc(sizeof(struct QuadList));
    *curr_instr_ptr = (*curr_instr_ptr)->next;
    (*curr_instr_ptr)->next = NULL;
    (*curr_instr_ptr)->flags = init_flags;
}

inline void set_triple_flag(struct QuadList* list, int flag_n)
{
    list->flags |= flag_n;
}

inline int check_triple_flag(struct QuadList* list, int flag_n)
{
    return list->flags & (1 << (flag_n - 1));
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

struct AssignQuad* gen_assignment(struct SymTab_entry* lval,  void* rval, enum SymbolType rval_type)
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

struct ParamQuad* gen_param(void* op, enum SymbolType type)
{
    struct ParamQuad* param = malloc(sizeof(struct ParamQuad));
    param->op = op;
    param->type = type;
    return param;
}

struct FuncCQuad* gen_funccall(struct SymTab_entry* lval, char* name)
{
    struct FuncCQuad* func_call = malloc(sizeof(struct FuncCQuad));
    func_call->lval = lval;
    func_call->name = name;
    return func_call;
}

struct RetQuad* gen_return(void* ret_val, enum SymbolType type)
{
    struct RetQuad* ret = malloc(sizeof(struct RetQuad));
    ret->ret_val = ret_val;
    ret->type = type;
    return ret;
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

                printf("%s = ", assign->lval->key);
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
                printf("%s = %c ", uop->result->key, op_arr[uop->operator_type]);
                if (uop->operand_type == CONSTANT)
                    printf("%s\n", uop->operand);
                else
                    printf("%s\n", ((struct SymTab_entry*)(uop->operand))->key);


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
            case QUAD_RETURN: {
                struct RetQuad* ret = curr_instr->instruction;
                printf("ret ");
                if (ret->type == CONSTANT)
                    printf("%s\n", ret->ret_val);
                else
                    printf("%s\n", ((struct SymTab_entry*)(ret->ret_val))->key);
                break;
            }
            case QUAD_PARAM: {
                struct ParamQuad* param = curr_instr->instruction;
                printf("param ");
                if (param->type == CONSTANT)
                    printf("%s\n", param->op);
                else
                    printf("%s\n", ((struct SymTab_entry*)(param->op))->key);
                break;
            }
            case QUAD_FUNC: {
                struct FuncCQuad* func = curr_instr->instruction;
                printf("%s = call %s\n", func->lval->key, func->name);
                break;
            }
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
void print_CFG()
{
    for (int i = 0; i < intermediate_code->size; i++) {
        struct IC_entry* entry = intermediate_code->entries[i];
        while (entry != NULL) {
            printf("%s: \n", entry->key);
            printr(entry->basic_block_list, 1, entry->n_blocks);
            printf("\n");
            entry = entry->next;
        }
    }
}

void destroy_instruction(struct QuadList* instruction)
{
    if (instruction->next == NULL)
        return;
    free(instruction->instruction);
    destroy_instruction(instruction->next);
}

void destroy_block(struct BasicBlock* block, char remove_jumps)
{
    destroy_instruction(block->instructions);
    if (block->jump_type == QUAD_UNCOND) {
        if (remove_jumps)
            free(block->jump);
    } else {
        free(block->condition);
        if (remove_jumps & 1)
            free(block->true);
        if (remove_jumps & 2)
            free(block->false);
    }

}


void destroyr(struct IC_entry* entry)
{
    struct BasicBlock** removed_addr[1024];
    int r_a_size = 0;
    for (int i = 0; i < entry->n_blocks; i++) {
        struct BasicBlock* block = entry->basic_block_list[i];

        if (block->jump_type == QUAD_UNCOND) {
            char remove_jumps = 1;
            struct BasicBlock** jump_label = block->jump;
            for (int i = 0; i < r_a_size; i++) {
                if (removed_addr[i] == jump_label) {
                    remove_jumps = 0;
                    goto destroy;
                }
            }
            removed_addr[r_a_size] = jump_label;
            r_a_size++;
            destroy:
            destroy_block(entry->basic_block_list[i], remove_jumps);
        } else {
            char remove_jumps = 3;
            struct BasicBlock** true_label = block->true;
            struct BasicBlock** false_label = block->false;
            for (int i = 0; i < r_a_size; i++) {
                if (removed_addr[i] == true_label)
                    remove_jumps--;
                else if (removed_addr[i] == false_label)
                    remove_jumps -= 2;
            }
            if (remove_jumps & 1) {
                removed_addr[r_a_size] = true_label;
                r_a_size++;
            }
            if (remove_jumps & 2) {
                removed_addr[r_a_size] = false_label;
                r_a_size++;
            }
            destroy_block(entry->basic_block_list[i], remove_jumps);
        }


    }
    if (entry->blockinfo != NULL)
        free(entry->blockinfo);

}
void destroy_CFG()
{
    for (int i = 0; i < intermediate_code->size; i++) {
        struct IC_entry* entry = intermediate_code->entries[i];
        for (;entry != NULL; entry = entry->next)
            destroyr(entry);
    }
}
