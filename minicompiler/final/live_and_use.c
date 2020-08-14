#include <stdio.h>
#include <stdlib.h>
#include "intermediate_code.h"
#include "hashing.h"

#define DEBUG 0

struct SymTab_entry_entry;
struct SymTab_entry_table;

struct SymTab_entry_table* create_SymTab_entry_table(int size);

void insert_entry(struct SymTab_entry_table* table,
                    struct SymTab_entry* real_entry,
                    char live, unsigned int next_use);

long get_info(struct SymTab_entry_table* table,
            struct SymTab_entry* real_entry);

void clear_entries(struct SymTab_entry_table* table);


struct SymTab_entry_table* set;
void walk_blocks(struct IC_entry* entry);
void live_and_use()
{
    set = create_SymTab_entry_table(129);
    for (int i = 0; i < intermediate_code->size; i++) {
        struct IC_entry* entry = intermediate_code->entries[i];
        while (entry != NULL) {
            walk_blocks(entry);
            entry = entry->next;
        }
    }
}
void walk_block(struct BasicBlock* block);
void walk_blocks(struct IC_entry* entry)
{
    for (int i = 0; i < entry->n_blocks; i++)
        walk_block(entry->basic_block_list[i]);
}

/*
 * Forward declaration...
 */
void visit_assign(struct AssignQuad* instruction, int n);
void visit_binop(struct BinOpQuad* instruction, int n);
void visit_uop(struct UOpQuad* instruction, int n);
void visit_conv(struct ConvQuad* instruction, int n);
void visit_return(struct RetQuad* instruction, int n);
void visit_param(struct ParamQuad* instruction, int n);
void visit_func(struct FuncCQuad* instruction, int n);
void visit_cond(struct CondQuad* instruction, int n);

void visit_QuadList(struct QuadList* instruction, int n, struct CondQuad* final);
void walk_block(struct BasicBlock* block)
{
    clear_entries(set);
    if (block->jump_type == QUAD_COND)
        visit_QuadList(block->instructions, 0, block->condition);
    else
        visit_QuadList(block->instructions, 0, NULL);
}


void visit_QuadList(struct QuadList* instruction, int n, struct CondQuad* final)
{
    if (instruction->next == NULL) {
        if (final != NULL)
            visit_cond(final, n);
        return;
    }
    visit_QuadList(instruction->next, n+1, final);
    switch (instruction->type) {
        case QUAD_ASSIGN:
            visit_assign(instruction->instruction, n);
            return;
        case QUAD_BINOP:
            visit_binop(instruction->instruction, n);
            return;
        case QUAD_UOP:
            visit_uop(instruction->instruction, n);
            return;
        case QUAD_CONV:
            visit_conv(instruction->instruction, n);
            return;
        case QUAD_RETURN:
            visit_return(instruction->instruction, n);
            return;
        case QUAD_PARAM:
            visit_param(instruction->instruction, n);
            return;
        case QUAD_FUNC:
            visit_func(instruction->instruction, n);
            return;
    }
}

void visit_assign(struct AssignQuad* instruction, int n)
{
    instruction->lval_info = get_info(set, instruction->lval);
    insert_entry(set, instruction->lval, 0, -1);
    if (instruction->rval_type == VARIABLE ||
        instruction->rval_type == TEMPORARY) {
        instruction->rval_info = get_info(set, instruction->rval);
        insert_entry(set, instruction->rval, 1, n);
    } else {
        instruction->rval_info = 0;
    }
}
void visit_binop(struct BinOpQuad* instruction, int n)
{
    instruction->result_info = get_info(set, instruction->result);
    insert_entry(set, instruction->result, 0, -1);
    if (instruction->op1_type == VARIABLE ||
        instruction->op1_type == TEMPORARY) {
        instruction->op1_info = get_info(set, instruction->op1);
        insert_entry(set, instruction->op1, 1, n);
    } else {
        instruction->op1_info = 0;
    }
    if (instruction->op2_type == VARIABLE ||
        instruction->op2_type == TEMPORARY) {
        instruction->op2_info = get_info(set, instruction->op2);
        insert_entry(set, instruction->op2, 1, n);
    } else {
        instruction->op2_info = 0;
    }
}

void visit_uop(struct UOpQuad* instruction, int n)
{
    instruction->result_info = get_info(set, instruction->result);
    insert_entry(set, instruction->result, 0, -1);
    if (instruction->operand_type == VARIABLE ||
        instruction->operand_type == TEMPORARY) {
        instruction->operand_info = get_info(set, instruction->operand);
        insert_entry(set, instruction->operand, 1, n);
    } else {
        instruction->operand_info = 0;
    }
}

void visit_conv(struct ConvQuad* instruction, int n)
{
    instruction->result_info = get_info(set, instruction->result);
    insert_entry(set, instruction->result, 0, -1);
    if (instruction->op_type == VARIABLE ||
        instruction->op_type == TEMPORARY) {
        instruction->op_info = get_info(set, instruction->op);
        insert_entry(set, instruction->op, 1, n);
    } else {
        instruction->op_info = 0;
    }
}

void visit_return(struct RetQuad* instruction, int n)
{
    if (instruction->type == VARIABLE ||
        instruction->type == TEMPORARY) {
        instruction->ret_val_info = get_info(set, instruction->ret_val);
        insert_entry(set, instruction->ret_val, 1, n);
    } else {
        instruction->ret_val_info = 0;
    }
}

void visit_param(struct ParamQuad* instruction, int n)
{
    if (instruction->type == VARIABLE ||
        instruction->type == TEMPORARY) {
        instruction->op_info = get_info(set, instruction->op);
        insert_entry(set, instruction->op, 0, -1);
    } else {
        instruction->op_info = 0;
    }
}

void visit_func(struct FuncCQuad* instruction, int n)
{
    instruction->lval_info = get_info(set, instruction->lval);
    insert_entry(set, instruction->lval, 0, n);
}

void visit_cond(struct CondQuad* instruction, int n)
{
    if (instruction->op1_type == VARIABLE ||
        instruction->op1_type == TEMPORARY) {
        get_info(set, instruction->op1);
        instruction->op1_info = get_info(set, instruction->op1);
        insert_entry(set, instruction->op1, 1, n);
    } else {
        instruction->op1_info = 0;
    }
    if (instruction->op2_type == VARIABLE ||
        instruction->op2_type == TEMPORARY) {
        instruction->op2_info = get_info(set, instruction->op2);
        insert_entry(set, instruction->op2, 1, n);
    } else {
        instruction->op2_info = 0;
    }
}



struct SymTab_entry_entry {
    struct SymTab_entry* real_entry;
    struct SymTab_entry_entry* next;
    /*
     * The first bit of info denotes whether the info should be considered.
     * The second bit denotes liveness.
     * The bits above is an int denoting next use.
     */
    unsigned long info;
};

struct SymTab_entry_table {
    int size;
    struct SymTab_entry_entry** entries;
};

struct SymTab_entry_table* create_SymTab_entry_table(int size)
{
    struct SymTab_entry_table* table = malloc(sizeof(struct SymTab_entry_table)) ;
    table->size = size;
    table->entries = malloc(sizeof(struct SymTab_entry_entry*)*size);
    for (int i = 0; i < size; i++)
        table->entries[i] = NULL;
    return table;
}

#define STORE_INFO(live, next_use) \
    ((unsigned long)1) + ((unsigned long)live << 1) + (((unsigned long)next_use) << 2);\

void insert_entry(struct SymTab_entry_table* table,
                    struct SymTab_entry* real_entry,
                    char live, unsigned int next_use)
{
    unsigned int hashv = ptr_hash(real_entry, table->size);
    struct SymTab_entry_entry* t_entry = table->entries[hashv];
    struct SymTab_entry_entry* prev;
    if (t_entry == NULL) {
        t_entry = malloc(sizeof(struct SymTab_entry_entry));
        t_entry->next = NULL;
        t_entry->real_entry = &(*real_entry);
        t_entry->info = STORE_INFO(live, next_use);
        table->entries[hashv] = t_entry;
        return;
    } else if (t_entry->real_entry == NULL) {
        t_entry->real_entry = real_entry;
        t_entry->info =  STORE_INFO(live, next_use);
        return;
    } else if (t_entry->real_entry == real_entry) {
        t_entry->info = STORE_INFO(live, next_use);
        return;
    }
    while (TRUE) {
        prev = t_entry;
        t_entry = prev->next;
        if (t_entry == NULL) {
            prev->next = malloc(sizeof(struct SymTab_entry_entry));
            prev->next->next = NULL;
            prev->next->real_entry = real_entry;
            prev->next->info = STORE_INFO(live, next_use);
            return;
        } else if (t_entry->real_entry == NULL) {
            prev->next->real_entry = real_entry;
            prev->next->info = STORE_INFO(live, next_use) ;
            return;
        } else if (t_entry->real_entry == real_entry) {
            prev->next->info = STORE_INFO(live, next_use);
            return;
        }
    }
}

long get_info(struct SymTab_entry_table* table,
            struct SymTab_entry* real_entry)
{
    unsigned int hashv = ptr_hash(real_entry, table->size);
    struct SymTab_entry_entry* t_entry = table->entries[hashv];
    while (t_entry != NULL) {
        if (t_entry->real_entry == real_entry) {
            #if DEBUG
            printf("%s: live: %d, use: %d\n", real_entry->key, t_entry->info & 1, t_entry->info >> 1);
            #endif
            return t_entry->info;
        }
        t_entry = t_entry->next;
    }
    return 0;
}

void clear_entries(struct SymTab_entry_table* table)
{
    #if DEBUG
    printf("\nclear\n\n");
    #endif
    for (int i = 0; i < table->size; i++) {
        struct SymTab_entry_entry* entry = table->entries[i];
        while (entry != NULL) {
            entry->real_entry = NULL;
            entry->info = 0;
            entry = entry->next;
        }
    }
}

inline void reattach_info(struct SymTab_entry* entry, unsigned long info)
{
    entry->info = info;
}
