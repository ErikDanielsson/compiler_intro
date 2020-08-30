#pragma once
enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
void store(struct SymTab_entry* var, int reg);
void write_asm(char* fstring, ...);

void write_farith(enum BinopType operator, unsigned int loged_width,
                unsigned int dest, unsigned int src)
{
    write_asm("%s %s, %s\n", float_arithmetic[loged_width-2][operator],
                register_names[4][dest-16], register_names[4][src-16]);
}
