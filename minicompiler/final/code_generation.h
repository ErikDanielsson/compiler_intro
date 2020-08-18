#pragma once

enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
void store(struct SymTab_entry* var, int reg);
int get_reg(unsigned int type, unsigned int not_this_reg);  
