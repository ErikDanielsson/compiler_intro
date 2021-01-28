#pragma once
#include "symbol_table.h"
#include "code_generation.h"
#include "consts.h"
#include "io.h"

enum RegValState {
    REG_VARIABLE,
    REG_TEMPORARY,
    REG_CONSTANT
};

enum RegState {
    REG_FREE,
    REG_OCCUPIED,
    REG_SAVED,
    REG_RESERVED
};

struct reg_desc {
    unsigned int size;
    unsigned int n_vals;
    enum RegState reg_state;
    int* states;
    void** vals;
    char reg_width;
};
void print_registers();

extern struct reg_desc registers[32];

// Real funcs:
void init_registers();
void clear_and_set_reg(int reg_n, enum RegValState new_state,
                        void* new_value);
void append_to_reg(int reg_n, enum SymbolType new_state,
                    void* new_value);
void remove_from_regs(struct SymTab_entry* entry);
int copy_reg_to_reg(unsigned int dest, unsigned int orig,
                    struct SymTab_entry* entry);
unsigned int least_reg(struct SymTab_entry* entry);
void print_reg_str(unsigned int i);
void store_allr_in_symtab(struct SymTab* symbol_table);

int free_reg(unsigned int reg, struct SymTab_entry* entry, unsigned int type, unsigned int loged_width);
int get_free_reg(unsigned int type);
int get_reg(unsigned int type, unsigned int not_this_reg, unsigned int width,
            void* entry, enum SymbolType entry_type);



// Should-be-inlined funcs:
static inline int get_reg_state(int reg_n)
{
    return registers[reg_n].reg_state;
}

static inline void clear_reg(int reg_n)
{
    registers[reg_n].n_vals = 0;
    registers[reg_n].reg_state = REG_FREE;
}

static inline unsigned int lowest_set_bit(unsigned int n)
{
    int res;
    asm("bsf %1, %0"
        : "=r"(res)
        : "r"(n));
    return res;
}

static inline void clear_all_locations(struct SymTab_entry* entry)
{

    unsigned int rval_locs = entry->reg_locs;
    for (unsigned lst = lowest_set_bit(rval_locs), real_loc = 0;
            rval_locs; lst = lowest_set_bit(rval_locs)) {
        rval_locs >>= lst+1;
        real_loc += lst+1;
        #if DEBUG
        printf("clear reg #%d\n", real_loc);
        #endif
        clear_reg(real_loc-1);
    }
    entry->reg_locs = 0;
}

static inline unsigned int first_reg(struct SymTab_entry* entry)
{
    return lowest_set_bit(entry->reg_locs);
}

static inline unsigned int resides_elsewhere(unsigned int locs, int this_residence)
{
    locs &= ~(1 << this_residence);
    if (locs)
        return lowest_set_bit(locs);
    return 0;
}

static inline unsigned int in_reg(unsigned int locs)
{
    return locs;
}

static inline void append_reg_to_symbol(struct SymTab_entry* entry, unsigned new_reg)
{
    printf("append :: ");
    print_bin(entry->reg_locs, 16);

    entry->reg_locs |= 1 << new_reg;

}

static inline unsigned int n_set_bits(unsigned int in)
{
    unsigned int res;
    asm("popcnt %1, %0"
        : "=r"(res)
        : "r"(in));
    return res;
}

static inline unsigned int count_registers(unsigned int locs)
{
    return n_set_bits(locs);
}

static inline unsigned int used_later(unsigned int info)
{
    return (info & 2) != 0;
}

static inline void store_all(unsigned int reg_n, struct SymTab_entry* entry)
{
    for (int i = 0; i < registers[reg_n].n_vals; i++)
        if (registers[reg_n].vals[i] != entry &&
            registers[reg_n].states[i] == REG_VARIABLE)
            store(registers[reg_n].vals[i], reg_n);
}
