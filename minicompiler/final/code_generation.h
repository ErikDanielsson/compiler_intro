#pragma once
enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
void store(struct SymTab_entry* var, int reg);
void write_asm(char* fstring, ...);
