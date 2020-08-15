#pragma once
#include "symbol_table.h"
enum RegState {
    REG_RESERVED,
    REG_EXTREMLY_RESERVED,
    REG_VARIABLE,
    REG_TEMPORARY,
    REG_CONSTANT
};

struct reg_desc {
    int size;
    int n_vals;
    int* states;
    void** vals;
    char reg_width;
};
void print_registers();

extern struct reg_desc registers[32];

// Real funcs:
void init_registers();
void clear_and_set_reg(int reg_n, enum RegState new_state,
                        void* new_value);
void append_to_reg(int reg_n, enum RegState new_state,
                    void* new_value);
void remove_from_regs(struct SymTab_entry* entry);

// Should-be-inlined funcs:
static inline int max_state(int reg_n)
{
    int max_state = -1;
    for (int i = 0; i < registers[reg_n].n_vals; i++) {
        if (max_state < registers[reg_n].states[i])
            max_state = registers[reg_n].states[i];
    }
    return max_state;
}

static inline void clear_reg(int reg_n)
{
    registers[reg_n].n_vals = 0;

}

static inline void state_on_store(struct SymTab_entry* entry)
{
    entry->locs |= 1;
}

static inline void set_on_load(int reg_n, struct SymTab_entry* entry)
{
    clear_and_set_reg(reg_n, REG_VARIABLE, entry);
    entry->locs |= 1 << (reg_n+1);
}

static inline void state_on_op(int reg_n, struct SymTab_entry* entry)
{
    clear_and_set_reg(reg_n, REG_TEMPORARY, entry);
    entry->locs = 1 << (reg_n + 1);
}

static inline unsigned int lowest_set_bit(unsigned int n)
{
    int res;
    asm("bsf %1, %0"
        : "=r"(res)
        : "r"(n));
    return res;
}

static inline void state_on_assign(struct SymTab_entry* lval, struct SymTab_entry* rval)
{
    unsigned int rval_locs = rval->locs;
    rval_locs &= ~1;
    lval->locs |= rval_locs;
    for (unsigned lst = lowest_set_bit(rval_locs), real_loc = 0;
            rval_locs; lst = lowest_set_bit(rval_locs)) {

        rval_locs >>= lst+1;
        real_loc += lst+1;

        append_to_reg(real_loc-2, lval->type == TEMPORARY ? REG_TEMPORARY : REG_VARIABLE,
                        lval);
    }
}

static inline void clear_all_locations(struct SymTab_entry* entry)
{
    unsigned int rval_locs = entry->locs;
    rval_locs &= ~1;
    for (unsigned lst = lowest_set_bit(rval_locs), real_loc = 0;
            rval_locs; lst = lowest_set_bit(rval_locs)) {
        rval_locs >>= lst+1;
        real_loc += lst+1;
        printf("clear reg #%d\n", real_loc-1);
        clear_reg(real_loc-2);
    }
    entry->locs = 0;
}

static inline unsigned int first_reg(struct SymTab_entry* entry)
{
    return lowest_set_bit((entry->locs) >> 1);
}

static inline unsigned int resides_elsewhere(unsigned int locs, int this_residence)
{
    locs &= ~(1 << (this_residence+1));
    if (locs)
        return lowest_set_bit(locs);
    return 0;
}

static inline void append_reg_to_symbol(struct SymTab_entry* entry, unsigned new_reg)
{
    entry->locs |= 1 << (new_reg+1);
}
/*
    $x = y + z
    <=>
    $x = y
    $x += z;
    <=>
    MOV
    MOV RN, [y]
    ADD RN, [z]
    MOV
*/


static inline int next_use(int reg_n)
{
    unsigned int next_use = 1 << 31;
    for (int i = 0; i < registers[reg_n].n_vals; i++) {
        if (registers[reg_n].states[i] == REG_VARIABLE) {
            struct SymTab_entry* entry = ((struct SymTab_entry*)(registers[reg_n].vals[i]));
            unsigned long temp_use = entry->info;
            if ((temp_use = (temp_use >> 2)) < next_use)
                next_use = temp_use;
        }
    }
    return next_use;
}

static inline int stored_else(int reg_n)
{
    unsigned int next_use = 1 << 31;
    for (int i = 0; i < registers[reg_n].n_vals; i++) {
        if (registers[reg_n].states[i] == REG_VARIABLE) {
            struct SymTab_entry* entry = (struct SymTab_entry*)(registers[reg_n].vals[i]);
            unsigned long temp_use = entry->info;
            if ((temp_use = (temp_use >> 2)) < next_use)
                next_use = temp_use;
        }
    }
    return 0;
}
