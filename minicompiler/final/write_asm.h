#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "parser.h"
#include "symbol_table.h"
#include "type_checker.h"
#include "intermediate_code.h"
#include "IC_table.h"
#include "code_generation.h"
#include "instruction_set.h"
#include "int_set.h"
#include "registers.h"

void init_asm_writer(FILE* out_file);
void write_asm(char* fstring, ...);
void _2nd_op_const(long val, unsigned int loged_width);
void mov_int_const_reg(unsigned int reg_n, long val, unsigned int loged_width);
void mov_int_const_mem(long val, unsigned int loged_width, char* mem_loc_str);
void get_memstr(char* memstr_loc, unsigned int isstatic, struct SymTab_entry* entry);
void mov_float_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width);
void mov_float_mem_reg(unsigned int dest, char* src, unsigned int loged_width);
void float_arith_reg_reg(enum BinOpType operator, unsigned int op1,
                                        unsigned int op2, unsigned int loged_width);
void float_arith_reg_mem(enum BinOpType operator, unsigned int op1,
                                    char* op2, unsigned int loged_width);
void int_arith_reg_reg(enum BinOpType operator, unsigned int op1, unsigned int op2, unsigned int loged_width);
void int_arith_reg_mem(enum BinOpType operator, unsigned int op1, struct SymTab_entry* entry, unsigned int loged_width);
void int_arith_reg_const(enum BinOpType operator, unsigned int op1, long val, unsigned int loged_width);
void mov_int_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width);
void mov_int_mem_reg(unsigned int dest, char* src, unsigned int loged_width);
void divmod_int_reg(unsigned int divisor, unsigned int loged_width);
void divmod_int_mem(char* divisor, unsigned int loged_width);
void shift_int_reg(enum BinOpType operator, unsigned int dest, unsigned int loged_width);
void shift_int_const(enum BinOpType operator, unsigned int dest, long val, unsigned int loged_width);
void float_neg(unsigned int op, unsigned int loged_width);
void unary_int(unsigned int op, unsigned int op_type, unsigned int loged_width);
void conv_float_float_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width);
void conv_int_float_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_float_int_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_int_int_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_float_float_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_int_float_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_float_int_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_int_int_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width);
void conv_float_float_const_reg(unsigned int dest, unsigned int offset);
void conv_float_int_const_reg(unsigned int dest, unsigned int offset, unsigned int loged_width);
void float_control_reg_reg(unsigned int op1, unsigned int op2, unsigned int loged_width);
void float_control_mem_reg(unsigned int op1, char* op2, unsigned int loged_width);
void float_control_const_reg(unsigned int op1, unsigned int offset, unsigned int loged_width);
void int_control_reg_reg(unsigned int op1, unsigned int op2, unsigned int loged_width);
void int_control_mem_reg(unsigned int op1, char* op2, unsigned int loged_width);
void int_control_const_reg(unsigned int op1, long val, unsigned int loged_width);
void cond_jump(unsigned int jump_type, void* target, unsigned int type);
void uncond_jump(void* target);
