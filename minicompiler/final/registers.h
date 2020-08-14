#pragma once
enum RegState {
    REG_RESERVED,
    REG_VARIABLE,
    REG_TEMPORARY,
    REG_CONSTANT,
    REG_INTERMEDIARY,
};

struct reg_desc {
    int size;
    int n_vals;
    enum RegState* states;
    void** vals;
    char reg_width;
};

extern struct reg_desc registers[32];
void init_registers();
inline int max_state(int reg_n);
inline void clear_reg(int reg_n);
void clear_and_set_reg(int reg_n, enum RegState new_state,
                        void* new_value);
void append_to_reg(int reg_n, enum RegState new_state,
                    void* new_value);
