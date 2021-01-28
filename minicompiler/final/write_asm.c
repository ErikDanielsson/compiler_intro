#include "write_asm.h"
/*
 * Helper functions for writing assembly to file.
 */

FILE* asm_file_desc;
void init_asm_writer(FILE* out_file)
{
    asm_file_desc = out_file;
}
    
void write_asm(char* fstring, ...)
{
    va_list args;
    va_start(args, fstring);
    vfprintf(asm_file_desc, fstring, args);
    va_end(args);
}


void _2nd_op_const(long val, unsigned int loged_width)
{
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write_asm("%d", (int)val);
            break;
        case 3:
            write_asm("%ld", (long)val);
            break;
    }
}

void mov_int_const_reg(unsigned int reg_n, long val, unsigned int loged_width)
{
    write_asm("mov %s, ", register_names[loged_width][reg_n]);
    _2nd_op_const(val, loged_width);
    write_asm("\n");
}

void mov_int_const_mem(long val, unsigned int loged_width, char* mem_loc_str)
{
    write_asm("mov %s %s, ", size_spec[loged_width], mem_loc_str);
    _2nd_op_const(val, loged_width);
    write_asm("\n");
}

void get_memstr(char* memstr_loc, unsigned int isstatic, struct SymTab_entry* entry)
{
    if (isstatic)
        sprintf(memstr_loc, "[%s%u]", entry->key, entry->counter_value);
    else
        sprintf(memstr_loc, "[rbp+%ld]", entry->offset);
}

/*
 * Functions which emit instuctions into the .asm file.
 */

void mov_float_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", mov[loged_width], register_names[4][dest-16],
            size_spec[loged_width], register_names[4][src-16]);
}

void mov_float_mem_reg(unsigned int dest, char* src, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", mov[loged_width], register_names[4][dest-16],
            size_spec[loged_width], src);
}

void float_arith_reg_reg(enum BinOpType operator, unsigned int op1,
                                        unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", float_arithmetic[loged_width-2][operator],
            register_names[4][op1-16], register_names[4][op2-16]);
}

void float_arith_reg_mem(enum BinOpType operator, unsigned int op1,
                                    char* op2, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", float_arithmetic[loged_width-2][operator],
            register_names[4][op1-16],size_spec[loged_width], op2);
}

void int_arith_reg_reg(enum BinOpType operator, unsigned int op1, unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", int_arithmetic[operator], register_names[loged_width][op1],
                            register_names[loged_width][op2]);
}

void int_arith_reg_mem(enum BinOpType operator, unsigned int op1, struct SymTab_entry* entry, unsigned int loged_width)
{
    write_asm("%s %s, %s [%s%u]\n", int_arithmetic[operator],
             register_names[loged_width][op1], size_spec[loged_width],
            entry->key, entry->counter_value);
}

void int_arith_reg_const(enum BinOpType operator, unsigned int op1, long val, unsigned int loged_width)
{
    write_asm("%s %s, ", int_arithmetic[operator], register_names[loged_width][op1]);
    _2nd_op_const(val, loged_width);
    write_asm("\n");
}

void mov_int_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width)
{
    write_asm("mov %s, %s\n", register_names[loged_width][dest],
                                register_names[loged_width][src]);
}

void mov_int_mem_reg(unsigned int dest, char* src, unsigned int loged_width)
{
    write_asm("mov %s, %s %s\n", register_names[loged_width][dest], size_spec[loged_width], src);
}

void divmod_int_reg(unsigned int divisor, unsigned int loged_width)
{
    write_asm("idiv %s\n", register_names[loged_width][divisor]);
}

void divmod_int_mem(char* divisor, unsigned int loged_width)
{
    write_asm("idiv %s %s\n", size_spec[loged_width], divisor);
}

void shift_int_reg(enum BinOpType operator, unsigned int dest, unsigned int loged_width)
{
    write_asm("%s %s, cl\n", int_arithmetic[operator], register_names[loged_width][dest]);
}

void shift_int_const(enum BinOpType operator, unsigned int dest, long val, unsigned int loged_width)
{
    write_asm("%s %s, ", int_arithmetic[operator], register_names[loged_width][dest]);
    _2nd_op_const(val, loged_width);
    write_asm("\n");
}

void float_neg(unsigned int op, unsigned int loged_width)
{
    unsigned int temp_int_reg = get_reg(0, -1, 1 << loged_width, NULL, TEMPORARY);
    unsigned int temp_float_reg = get_reg(1, op, 1 << loged_width, NULL, TEMPORARY);
    if (loged_width-2) {
        mov_int_const_reg(temp_int_reg, 0x8000000000000000, loged_width);
        write_asm("movd %s, %s\n", register_names[4][temp_float_reg-16], register_names[loged_width][temp_int_reg]);
        write_asm("xorpd %s, %s\n", register_names[4][op-16], register_names[4][temp_float_reg-16]);

    }
    else {
        mov_int_const_reg(temp_int_reg, 0x80000000, loged_width);
        write_asm("movd %s, %s\n", register_names[4][temp_float_reg-16], register_names[loged_width][temp_int_reg]);
        write_asm("xorps %s, %s\n", register_names[4][op-16], register_names[4][temp_float_reg-16]);
    }
    clear_reg(temp_int_reg);
    clear_reg(temp_float_reg);
}

void unary_int(unsigned int op, unsigned int op_type, unsigned int loged_width)
{
    write_asm("%s %s\n", int_unary[op_type], register_names[loged_width][op]);
}

void conv_float_float_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width)
{
    write_asm("%s %s, %s\n", conv_float[new_loged_width-2], register_names[4][dest-16], register_names[4][src-16]);
}

void conv_int_float_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    printf("new %d old %d\n", old_loged_width, new_loged_width);
    write_asm("%s %s, %s\n", conv_float_int[new_loged_width], register_names[4][dest-16], register_names[old_loged_width][src]);
}

void conv_float_int_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    write_asm("%s %s, %s\n", conv_float_int[old_loged_width], register_names[new_loged_width][dest], register_names[4][src-16]);
}

void conv_int_int_reg_reg(unsigned int dest, unsigned int src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    if (new_loged_width > old_loged_width)
        write_asm("%s %s, %s\n", conv_signed[1], register_names[new_loged_width][dest], register_names[old_loged_width][src]);
}

void conv_float_float_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    write_asm("%s %s, %s %s\n", conv_float[new_loged_width-2], register_names[4][dest-16], size_spec[old_loged_width], src);
}

void conv_int_float_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    write_asm("%s %s, %s %s\n", conv_float_int[old_loged_width-2], register_names[4][dest-16], size_spec[old_loged_width], src);
}

void conv_float_int_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    write_asm("%s %s, %s %s\n");
}

void conv_int_int_mem_reg(unsigned int dest, char* src, unsigned int new_loged_width, unsigned int old_loged_width)
{
    if (new_loged_width < old_loged_width)
        write_asm("%s %s, %s %s\n");
}

void conv_float_float_const_reg(unsigned int dest, unsigned int offset)
{
    write_asm("%s %s, [d_const+%u]\n", conv_float[1], register_names[4][dest-16], offset);
}

void conv_float_int_const_reg(unsigned int dest, unsigned int offset, unsigned int loged_width)
{
    write_asm("%s %s, [d_const+%u]\n", conv_float_int[1], register_names[loged_width][dest], offset);
}

void float_control_reg_reg(unsigned int op1, unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", float_control[loged_width-2], register_names[4][op1-16], register_names[4][op2-16]);
}

void float_control_mem_reg(unsigned int op1, char* op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", float_control[loged_width-2], register_names[4][op1-16],op2);
}

void float_control_const_reg(unsigned int op1, unsigned int offset, unsigned int loged_width)
{
    if (loged_width == 2)
        write_asm("%s %s, [f_consts+%u]\n", float_control[loged_width-2], register_names[4][op1-16], offset);
    else
        write_asm("%s %s, [d_consts+%u]\n", float_control[loged_width-2], register_names[4][op1-16], offset);
}

void int_control_reg_reg(unsigned int op1, unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", int_control[0], register_names[loged_width][op1], register_names[loged_width][op2]);
}

void int_control_mem_reg(unsigned int op1, char* op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", int_control[0], register_names[loged_width][op1], op2);
}

void int_control_const_reg(unsigned int op1, long val, unsigned int loged_width)
{
    if (val == 0) {
        write_asm("%s %s, %s\n", int_control[1], register_names[loged_width][op1], register_names[loged_width][op1]);
    } else {
        write_asm("%s %s, ", int_control[0], register_names[loged_width][op1]);
        _2nd_op_const(val, loged_width);
        write_asm("\n");
    }
}

void cond_jump(unsigned int jump_type, void* target, unsigned int type)
{
    write_asm("%s L%p\n", cond_jumps[!type][jump_type], target);
}
void uncond_jump(void* target)
{
    write_asm("jmp L%p\n", target);
}
