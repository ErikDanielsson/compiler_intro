#pragma once

enum RegState {
    REG_RESERVED,
    REG_CONSTANT,
    REG_VARIABLE
};

struct reg_desc {
    enum RegState type;
    void* value;
};

enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
int get_reg(void* symbol, enum SymbolType type);
