#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "registers.h"
#include "symbol_table.h"
#include "instruction_set.h"

struct reg_desc registers[32];
void init_registers()
{
    for (int i = 0; i < 32; i++) {
        registers[i].size = 4;
        registers[i].n_vals = 0;
        registers[i].states = malloc(sizeof(enum RegValState)*4);
        registers[i].vals = malloc(sizeof(void*)*4);
    }
}

void clear_and_set_reg(int reg_n, enum RegValState new_state,
                        void* new_value)
{
    /*
     * Dumb code which clang/gcc should be able to optimize away.
     */
    //clear_reg(reg_n);
    registers[reg_n].n_vals++;
    registers[reg_n].states[0] = new_state;
    registers[reg_n].vals[0] = new_value;
}

void expand_reg_desc(int reg_n, int expansion)
{
    int size = registers[reg_n].size;
    void* vals[size];
    enum RegValState states[size];
    memcpy(states, registers[reg_n].states, sizeof(enum RegValState)*size);
    memcpy(vals, registers[reg_n].vals, sizeof(void*)*size);
    free(registers[reg_n].states);
    free(registers[reg_n].vals);
    registers[reg_n].states = malloc(sizeof(enum RegValState)*(size+expansion));
    registers[reg_n].vals = malloc(sizeof(void*)*(size+expansion));
    memcpy(registers[reg_n].states, states, sizeof(enum RegValState)*size);
    memcpy(registers[reg_n].vals, vals, sizeof(void*)*size);
}

void append_to_reg(int reg_n, enum RegValState new_state,
                    void* new_value)
{
    if (registers[reg_n].size == registers[reg_n].size)
        expand_reg_desc(reg_n, 4);
    int size = registers[reg_n].size++;
    registers[reg_n].states[size] = new_state;
    registers[reg_n].vals[size] = new_value;
}

void remove_from_reg(struct SymTab_entry* entry, unsigned reg_n)
{
    unsigned n_vals = registers[reg_n].n_vals;
    if (n_vals-1) {
        for (int i = 0; i < n_vals; i++)
        {
            if (registers[reg_n].vals[i] == entry) {
                unsigned size = n_vals-i-1;
                memcpy(registers[reg_n].vals+i, registers[reg_n].vals+i+1, sizeof(void*)*size);
                memcpy(registers[reg_n].states+i, registers[reg_n].states+i+1, sizeof(int)*size);
                return;
            }
        }
    } else {
        clear_reg(reg_n);
    }

}

void remove_from_regs(struct SymTab_entry* entry)
{
    unsigned locs = entry->locs >> 1;
    for (unsigned lst = lowest_set_bit(locs), real_loc = 0;
            locs; lst = lowest_set_bit(locs)) {
        locs >>= lst+1;
        real_loc += lst+1;
        #if DEBUG
        printf("clear reg #%d from %s\n", real_loc-1, entry->key);
        #endif
        remove_from_reg(entry, real_loc-2);
    }
}

unsigned int least_reg(unsigned int loc)
{
    int reg = -1;
    unsigned int min_n_vals = MAX_UINT ;
    loc &= ~1;
    for (unsigned lst = lowest_set_bit(loc), real_loc = 0;
            loc; lst = lowest_set_bit(loc)) {
        loc >>= lst+1;
        real_loc += lst+1;
        if (registers[real_loc-2].n_vals < min_n_vals) {
            reg = real_loc-2;
            min_n_vals = registers[real_loc-2].n_vals;
        }
    }
    if (reg == -1)
        fprintf(stderr, "Internal error:No register assigned to symbol\n");

    store_all(reg);
    return reg;
}

int copy_reg_to_reg(unsigned int dest, unsigned int orig, struct SymTab_entry* entry)
{
    int ret_val = 0;
    int n_vals = registers[orig].n_vals;
    if (registers[dest].size < n_vals)
        expand_reg_desc(dest, registers[orig].size - registers[dest].size);
    memcpy(registers[dest].states, registers[orig].states, n_vals);
    memcpy(registers[dest].vals, registers[orig].vals, n_vals);
    for (int i = 0; i < registers[orig].n_vals; i++) {
        struct SymTab_entry* tmp_entry = registers[orig].vals[i];
        ret_val = tmp_entry == entry;
        entry->locs |= 1 << (dest+1);
        entry->locs &= ~(1 << (orig+1));
    }
    return ret_val;
}


void print_registers()
{
    for (int i = 0; i < 16; i++) {
        if (!(registers[i].n_vals))
            continue;
        printf(" : %s: ", register_names[3][i]);
        for (int j = 0; j < registers[i].n_vals; j++) {
            printf("%d, ", registers[i].states[j]);
        }
        printf("\n");
    }
    for (int i = 0; i < 16; i++) {
        if (!(registers[16+i].n_vals))
            continue;
        printf("%s: ", register_names[4][i]);
        for (int j = 0; j < registers[16+i].n_vals; j++) {
            printf("%d, ", registers[16+i].states[j]);
        }
        printf("\n");
    }
}