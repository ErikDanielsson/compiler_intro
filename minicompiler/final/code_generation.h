#pragma once

enum syscall_const {
    SYSEXIT
};

void generate_assembly(const char* basename);
unsigned int get_reg(void* symbol, enum SymbolType type, int width);
