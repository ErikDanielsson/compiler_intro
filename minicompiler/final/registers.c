#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registers.h"
#include "symbol_table.h"

struct reg_desc registers[32];
void init_registers()
{
    for (int i = 0; i < 0; i++) {
        registers[i].size = 4;
        registers[i].size = 0;
        registers[i].states = malloc(sizeof(enum RegState)*4);
        registers[i].vals = malloc(sizeof(void*)*4);
    }
}

inline int max_state(int reg_n)
{
    int max_state = -1;
    for (int i = 0; i < registers[reg_n].size; i++)
        if (max_state < registers[reg_n].states[i])
            max_state = registers[reg_n].states[i];
    return max_state;
}

inline void clear_reg(int reg_n)
{
    registers[reg_n].vals = 0;
}

void clear_and_set_reg(int reg_n, enum RegState new_state,
                        void* new_value)
{
    /*
     * Dumb code which clang/gcc should be able to optimize away.
     */
    clear_reg(reg_n);
    registers[reg_n].n_vals++;
    registers[reg_n].states[0] = new_state;
    registers[reg_n].vals[0] = new_value;
}

void expand_reg_desc(int reg_n)
{
    int size = registers[reg_n].size;
    void* vals[size];
    enum RegState states[size];
    memcpy(states, registers[reg_n].states, sizeof(enum RegState)*size);
    memcpy(vals, registers[reg_n].vals, sizeof(void*)*size);
    free(registers[reg_n].states);
    free(registers[reg_n].vals);
    registers[reg_n].states = malloc(sizeof(enum RegState)*(size+4));
    registers[reg_n].vals = malloc(sizeof(void*)*(size+4));
    memcpy(registers[reg_n].states, states, sizeof(enum RegState)*size);
    memcpy(registers[reg_n].vals, vals, sizeof(void*)*size);
}

void append_to_reg(int reg_n, enum RegState new_state,
                    void* new_value)
{
    if (registers[reg_n].size == registers[reg_n].size)
        expand_reg_desc(reg_n);
    int size = registers[reg_n].size++;
    registers[reg_n].states[size] = new_state;
    registers[reg_n].vals[size] = new_value;
}

inline void state_on_store(struct SymTab_entry* entry)
{
    entry->locs |= 1;
}

inline void state_on_load(int reg_n, struct SymTab_entry* entry)
{
    clear_and_set_reg(reg_n, REG_VARIABLE, entry);
    entry->locs |= (1 << reg_n);
}

inline void state_on_op(int reg_n, struct SymTab_entry* entry)
{
    clear_and_set_reg(reg_n, REG_TEMPORARY, entry);
    entry->locs = 1 << reg_n;
}

static inline unsigned int lowest_set_bit(unsigned int n)
{
    int res;
    asm("bsf %1, %0",
        : "=r"(res),
        : "r"(n));
    return res;
}

inline void state_on_assign(struct SymTab_entry* lval, struct SymTab_entry* rval)
{
    unsigned int rval_locs = rval->locs;
    rval_locs &= ~1
    lval->locs |= rval_locs;
    for (unsigned lst = lowest_set_bit(rval_locs), real_loc = 0;
            rval_locs; lst = lowest_set_bit(rval_locs)) {
        rval_locs >>= lst+1;
        real_loc += lst+1;
        append_to_reg(real_loc1-1, lval->type == TEMPORARY ? REG_TEMPORARY : REG_VARIABLE,
                        lval);
    }
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


inline int next_use(int reg_n)
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

inline int stored_else(int reg_n)
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
}
