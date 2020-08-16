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

/*
 * The code generator will generate Intel syntax assembly for the x86-64
 * architecture. Should rewrite it later to generate an ".o" file directly
 */

FILE* asm_file_desc;
void codegen();
void generate_assembly(const char* basename)
{
    char filename[strlen(basename)+5];
    sprintf(filename, "%s.asm", basename);
    asm_file_desc = fopen(filename, "w");
    init_registers();
    codegen();
    fclose(asm_file_desc);
}

void write(char* fstring, ...)
{
    va_list args;
    va_start(args, fstring);
    vfprintf(asm_file_desc, fstring, args);
    va_end(args);
}




void alloc_statics(struct SymTab* main_symbol_table);
void write_code(struct IC_entry* main);
void codegen()
{
    struct IC_entry* main = IC_table_get_entry(intermediate_code, "main");
    alloc_statics(main->symbol_table);
    write_code(main);
}

void allocr(struct SymTab* symbol_table);
static inline void write_float_consts()
{
    write("\tdodouboblole dd ");
    struct entry_list* entry_l = float_table->start;
    for (; entry_l->next != NULL; entry_l = entry_l->next)
        write("%lf, ", ((struct float_entry*)(entry_l->entry))->val);
    write("\n");
}


void alloc_statics(struct SymTab* main_symbol_table)
{
    write("section .data\n");
    allocr(main_symbol_table);
    write("\n");

    /*
     * We only need to emit the float and double consts, since ints can
     * be directly.
     */
    write_float_consts();
    write("\n");
}
void write_all_variables(struct SymTab* symbol_table);
void allocr(struct SymTab* symbol_table)
{
    write_all_variables(symbol_table);
    for (int i = 0; i < symbol_table->n_childs; i++)
        allocr(symbol_table->childs[i]);

}

int log2(int n)
{
    // Instruction 'bsr' checks the highest set bit, i.e log2 for ints.
    asm("bsr %1, %0"
        : "=r"(n)
        : "r"(n));
    return n;
}



void write_all_variables(struct SymTab* symbol_table)
{
    // NOTE: for 128 bit int and floats we need 'ddq' and 'dt' as well.
    char* declaration_widths[] = {"db", "dw", "dd", "dq"};
    for (int i = 0; i < symbol_table->table_size; i++) {
        struct SymTab_entry* entry = symbol_table->entries[i];
        for (; entry != NULL; entry = entry->next) {
            if (entry->type == TEMPORARY || entry->type == FUNCTION)
                continue;
            write("\t%s%u %s 0\n", entry->key, entry->counter_value,
            declaration_widths[log2(entry->width_and_type >> 2)]);
        }
    }
}

void write_func(struct IC_entry* func);
void write_main(struct IC_entry* main);
void write_code(struct IC_entry* main)
{
    write("section .text\n");
    write_main(main);
    for (int i = 0; i < intermediate_code->size; i++) {
        struct IC_entry* entry = intermediate_code->entries[i];
        while (entry != NULL) {
            if (strcmp(entry->key, "main") != 0)
                write_func(entry);
            entry = entry->next;
        }
    }
}


#define INT_SET_SIZE 131
#define CREATE_INT_SET(set)\
    set.size = INT_SET_SIZE;\
    set.entries = malloc(sizeof(struct int_set_entry*)*INT_SET_SIZE);\
    for (int i = 0; i < INT_SET_SIZE; i++)\
        set.entries[i] = NULL;\

struct int_set set;
void visit_block(struct BasicBlock* block);
void write_main(struct IC_entry* main)
{

    CREATE_INT_SET(set);
    write("global main\n");
    write("main:\n");
    visit_block(main->basic_block_list[0]);
    write("\n\n");
}

void write_func(struct IC_entry* func)
{
    struct int_set set;
    CREATE_INT_SET(set);
    write("global %s\n", func->key);
    write("%s:", func->key);
    write("\n\n");
}

void emit_instruction(void* instruction, enum QuadType type);
void visit_block(struct BasicBlock* block)
{
    struct QuadList* curr = block->instructions;
    for (; curr->next != NULL; curr = curr->next)
        emit_instruction(curr->instruction, curr->type);
}

static inline void emit_assign(struct AssignQuad* assign);
static inline void emit_binop(struct BinOpQuad* binop);
static inline void emit_uop(struct UOpQuad* assign);
static inline void emit_conv(struct ConvQuad* binop);
void emit_instruction(void* instruction, enum QuadType type)
{
    switch (type) {
        case QUAD_ASSIGN:
            emit_assign(instruction);
            return;
        case QUAD_BINOP:
            emit_binop(instruction);
            return;
        case QUAD_UOP:
            emit_uop(instruction);
            return;
        case QUAD_CONV:
            emit_conv(instruction);
            return;
        case QUAD_COND:
            return;
        case QUAD_UNCOND:
            return;
        case QUAD_RETURN:
            return;
        case QUAD_PARAM:
            return;
        case QUAD_FUNC:
            return;
    }
}

static inline void emit_assign(struct AssignQuad* assign)
{
    unsigned long width_and_type = assign->lval->width_and_type;
    int type = width_and_type & 1;
    int width = width_and_type >> 2;
    remove_from_regs(assign->lval);
    if (type) {
        switch (assign->rval_type) {
            case TEMPORARY: {
                struct SymTab_entry* rval_entry = assign->rval;

                assign->lval->locs = rval_entry->locs & ~1;
                return;
            }
            case VARIABLE: {
                struct SymTab_entry* rval_entry = assign->rval;
                if (!resides_elsewhere(rval_entry->locs, -1)) {
                    unsigned int new_reg;// = get_reg(..);
                    append_reg_to_symbol(rval_entry, new_reg);
                }
                assign->lval->locs = rval_entry->locs & ~1;
            }
            case FCONSTANT: {
                struct float_entry* rval_entry = assign->rval;
                if (!resides_elsewhere(rval_entry->locs, -1)) {
                    unsigned int new_reg;// = get_reg(..);
                    append_reg_to_symbol(rval_entry, new_reg);
                }
                assign->lval->locs = rval_entry->locs & ~1;
            }
        }
    } else {
        switch (assign->rval_type) {
            case TEMPORARY: {
                struct SymTab_entry* rval_entry = assign->rval;
                assign->lval->locs = rval_entry->locs & ~1;
                return;
            }
            case VARIABLE: {
                struct SymTab_entry* rval_entry = assign->rval;
                if (!resides_elsewhere(rval_entry->locs, -1)) {
                    unsigned int new_reg;// = get_reg(..);
                    append_reg_to_symbol(rval_entry, new_reg);
                }
                assign->lval->locs = rval_entry->locs & ~1;
            }
            case ICONSTANT: {
                struct int_entry* rval_entry = assign->rval;
                if (assign->lval->info & 1) {
                    assign->lval->locs = 0;
                    unsigned int new_reg;// = get_reg(..);
                    append_reg_to_symbol(assign->lval, new_reg);
                    if (width == 4)
                        write("mov %s, %d\n", register_names[2][new_reg], (int)rval_entry->val);
                    else
                        write("mov %s, %ld\n", register_names[3][new_reg], rval_entry->val);
                } else {
                    assign->lval->locs = 1;
                    if (width == 4)
                        write("mov %s [%s%u], %d\n", size_spec[2], assign->lval->key, assign->lval->counter_value, (int)rval_entry->val);
                    else
                        write("mov %s [%s%u], %ld\n", size_spec[3], assign->lval->key, assign->lval->counter_value, rval_entry->val);
                }

            }
        }
    }
}
void write_int_arith(const char* operator, char* destination, void* operand, enum SymbolType type, int width);
void write_int_mov(char* destination, void* operand, enum SymbolType type, int width) ;

static inline void emit_binop(struct BinOpQuad* binop)
{
    unsigned long width_and_type = binop->result->width_and_type;
    unsigned long width = width_and_type >> 2;
    unsigned long type = width_and_type & 1;
    unsigned int loged_width = loged_width;
    unsigned int result_reg;
    if (type) {
        switch (binop->op1_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = binop->op1;
                result_reg = first_reg(entry);
                clear_all_locations(entry);
                entry->locs = 0;
                break;
            }
            case VARIABLE: {
                struct SymTab_entry* entry = binop->op1;
                if (in_reg(entry->locs)) {
                    if (!(used_later(entry->info))) {
                        result_reg = least_reg(entry);
                        if (!(entry->locs & 1))
                            store(entry);
                        clear_all_locations(entry);
                        entry->locs = 1;
                    } else if (count_registers(entry->locs) >= 2) {
                        result_reg = least_reg(entry);
                    } else {
                        result_reg = get_reg(entry, ..);
                        unsigned int least_reg = least_reg(entry);
                        if (width == 4)
                            write("movss %s, %s %s", register_names[4][result_reg],
                                    size_spec[2], register_names[4][least_reg]);
                        else
                            write("movsd %s, %s %s", register_names[4][result_reg],
                                    size_spec[3], register_names[4][least_reg]);
                    }
                } else {
                    result_reg = get_reg(entry, ..);
                    if (width == 4)
                        write("movss %s, %s [%s%u]", register_names[4][result_reg],
                                size_spec[2], entry->key, entry->counter_value);
                    else
                        write("movsd %s, %s [%s%u]", register_names[4][result_reg],
                                size_spec[3], entry->key, entry->counter_value);
                }
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = binop->op1;
                result_reg = get_reg(entry, ..);
                /*
                 * Should add live and use info of float consts to reduce
                 * loads of the same constant. This approach will load the same
                 * constant over and over again.
                 */
                load_float(result_reg, width, entry->offset);
                break;
            }
        }
        switch (binop->op2_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = binop->op2;
                unsigned int op_reg = first_reg(entry);
                clear_all_locations(entry);
                entry->locs = 0;
                write("%s %s, %s", float_arithmetic[width == 4][binop->op_type],
                        register_names[4][result_reg], register_names[4][op_reg]);
                break;
            }
            case VARIABLE: {
                struct SymTab_entry* entry = binop->op2;
                if (in_reg(entry->locs)) {
                    if (!(used_later(entry->info))) {
                        result_reg = least_reg(entry);
                        if (!(entry->locs & 1))
                            store(entry);
                        clear_all_locations(entry);
                        entry->locs = 1;
                    }  else {
                        unsigned int op_reg = first_reg(entry);
                        write("%s %s, %s", float_arithmetic[width == 4][binop->op_type],
                                register_names[4][result_reg], register_names[4][op_reg]);
                    }
                } else if (used_later(entry->info)) {
                    unsigned int op_reg = get_reg(entry, ..);
                    load(entry, op_reg, width);
                    write("%s %s, %s", float_arithmetic[width == 4][binop->op_type],
                            register_names[4][result_reg], register_names[4][op_reg]);
                } else {
                    write("%s %s, %s [%s%u]", float_arithmetic[width == 4][binop->op_type],
                             register_names[4][result_reg], size_spec[loged_width],
                            entry->key, entry->counter_value);
                }
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = binop->op2;
                write("%s %s, %s [dodouboblole%u]", float_arithmetic[width == 4][binop->op_type],
                        register_names[4][result_reg], size_spec[loged_width],
                        entry->offset);
            }

        }
    } else {
        switch (binop->op_type) {
            case BINOP_ADD:
            case BINOP_SUB:
            case BINOP_MUL:
            case BINOP_AND:
            case BINOP_OR:
            case BINOP_XOR: {
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        result_reg = first_reg(entry);
                        clear_all_locations(entry);
                        entry->locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->locs)) {
                            if (!(used_later(entry->info))) {
                                result_reg = least_reg(entry);
                                if (!(entry->locs & 1))
                                    store(entry);
                                clear_all_locations(entry);
                                entry->locs = 1;
                            } else if (count_registers(entry->locs) >= 2) {
                                result_reg = least_reg(entry);
                            } else {
                                result_reg = get_reg(entry, ..);
                                unsigned int least_reg = least_reg(entry);
                                write("mov %s, %s %s", register_names[loged_width][result_reg],
                                            size_spec[loged_width], register_names[loged_width][least_reg]);
                            }
                        } else {
                            result_reg = get_reg(entry, ..);
                            write("mov %s, %s [%s%u]", register_names[loged_width][result_reg],
                                    size_spec[loged_width], entry->key, entry->counter_value);

                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(entry, ..);
                        if (width == 4)
                            write("mov %s, %d\n", registers[loged_width][result_reg], (int)entry->val);
                        else
                            write("mov %s, %ld\n", registers[loged_width][result_reg], entry->val);
                        break;
                    }
                }
                switch (binop->op2_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op2;
                        unsigned int op_reg = first_reg(entry);
                        clear_all_locations(entry);
                        entry->locs = 0;
                        write("%s %s, %s", int_arithmetic[width == 4][binop->op_type],
                                register_names[4][result_reg], register_names[4][op_reg]);
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op2;
                        if (in_reg(entry->locs)) {
                            if (!(used_later(entry->info))) {
                                result_reg = least_reg(entry);
                                if (!(entry->locs & 1))
                                    store(entry);
                                clear_all_locations(entry);
                                entry->locs = 1;
                            }  else {
                                unsigned int op_reg = first_reg(entry);
                                write("%s %s, %s", int_arithmetic[width == 4][binop->op_type],
                                        register_names[4][result_reg], register_names[4][op_reg]);
                            }
                        } else if (used_later(entry->info)) {
                            unsigned int op_reg = get_reg(entry, ..);
                            load(entry, op_reg, width);
                            write("%s %s, %s", int_arithmetic[width == 4][binop->op_type],
                                    register_names[4][result_reg], register_names[4][op_reg]);
                        } else {
                            write("%s %s, %s [%s%u]", int_arithmetic[width == 4][binop->op_type],
                                     register_names[4][result_reg], size_spec[loged_width],
                                    entry->key, entry->counter_value);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        if (width == 4)
                            write("%s %s, %d", int_arithmetic[width == 4][binop->op_type],
                                    register_names[loged_width][result_reg], size_spec[loged_width],
                                    (int)entry->offset);
                        else
                            write("%s %s, %ld", int_arithmetic[width == 4][binop->op_type],
                                    register_names[loged_width][result_reg], size_spec[loged_width],
                                    (int)entry->offset);
                    }
                }
            }
            case BINOP_DIV:
            case BINOP_MOD: {

                if (binop->op_type == BINOP_DIV)
                    result_reg = 0;
                else
                    result_reg = 3;
                /*
                 * Free 'a' and 'd' registers to be able to perform division
                 */
                unsigned int op1_in_a = free_reg(0, binop->op1, binop->op1_type);
                unsigned int op1_in_d = free_reg(3, binop->op1, binop->op1_type);
                if (op1_in_a)
                    goto binop_select_2nd_op;
                if (op1_in_d) {
                    write("mov %s, %s\n", register_names[loged_width][0], register_names[loged_width][3]);
                    goto binop_select_2nd_op;
                }
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        unsigned int dividend = first_reg(entry);
                        write("mov %s, %s", register_names[loged_width][0],
                                            register_names[loged_width][dividend]);
                        clear_all_locations(entry);
                        entry->locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        int dividend;
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->locs)) {
                            struct SymTab_entry* entry = binop->op1;
                            dividend = first_reg(entry);
                            write("mov %s, %s\n",
                                    register_names[loged_width][0],
                                    register_names[loged_width][dividend]);
                        } else if (used_later(entry->info) &&
                                (dividend = get_free_reg(entry) != -1) {
                            write("mov %s, %s\n",
                                    register_names[loged_width][0],
                                    register_names[loged_width][dividend]);
                        } else {
                            write("mov %s, %s [%s%u]\n",
                                register_names[loged_width][0],
                                size_spec[loged_width],
                                entry->key,
                                entry->counter_value);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        if (width == 4)
                            write("mov %s, %d\n",
                                register_names[loged_width][0],
                                (int)entry->val);
                        else
                            write("mov %s, %ld\n",
                                register_names[loged_width][0],
                                entry->val);
                    }
                }
            binop_select_2nd_op:
                write("%s\n", conv_to_ad[loged_width]);
                switch (binop->op2_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op2;
                        unsigned int dividend = first_reg(entry);
                        write("idiv %s\n", register_names[loged_width][divisior]);
                        clear_all_locations(entry);
                        entry->locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        int divisor;
                        struct SymTab_entry* entry = binop->op2;
                        if (in_reg(entry->locs)) {
                            struct SymTab_entry* entry = binop->op2;
                            divisor = first_reg(entry);
                            write("idiv %s\n",
                                    register_names[loged_width][divisor]);
                        } else if (used_later(entry->info) &&
                                (divisor = get_free_reg(entry) != -1) {
                            write("idiv %s\n",
                                    register_names[loged_width][divisor]);
                        } else {
                            write("idiv %s [%s%u]\n",
                                size_spec[loged_width],
                                entry->key,
                                entry->counter_value);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        unsigned int op_reg = get_reg(entry, ..);
                        if (width == 4)
                            write("mov %s, %d\n",
                                register_names[loged_width][op_reg],
                                (int)entry->val);
                        else
                            write("mov %s, %ld\n",
                                register_names[loged_width][op_reg],
                                entry->val);
                        write("idiv %s\n", register_names[loged_width][op_reg]);
                    }
                }
            }
            case BINOP_SHL:
            case BINOP_SHR: {
                unsigned int op2_in_c;
                if (binop->op2_type1 != ICONSTANT)
                    op2_in_c = free_reg(2, binop->op2, binop->op2_type);
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        result_reg = first_reg(entry);
                        clear_all_locations(entry);
                        entry->locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->locs)) {
                            if (!(used_later(entry->info))) {
                                result_reg = least_reg(entry);
                                if (!(entry->locs & 1))
                                    store(entry);
                                clear_all_locations(entry);
                                entry->locs = 1;
                            } else if (count_registers(entry->locs) >= 2) {
                                result_reg = least_reg(entry);
                            } else {
                                result_reg = get_reg(entry, ..);
                                unsigned int least_reg = least_reg(entry);
                                write("mov %s, %s %s", register_names[loged_width][result_reg],
                                            size_spec[loged_width], register_names[loged_width][least_reg]);
                            }
                        } else {
                            result_reg = get_reg(entry, ..);
                            write("mov %s, %s [%s%u]", register_names[loged_width][result_reg],
                                    size_spec[loged_width], entry->key, entry->counter_value);

                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(entry, ..);
                        if (width == 4)
                            write("mov %s, %d\n", registers[loged_width][result_reg], (int)entry->val);
                        else
                            write("mov %s, %ld\n", registers[loged_width][result_reg], entry->val);
                        break;
                    }
                    if (op2_in_c) {
                        write("%s %s, cl\n", int_arithmetic[binop->op_type], register_names[loged_width][result_reg]);
                    } else {
                        switch (binop->op2_type) {
                            case TEMPORARY: {
                                struct SymTab_entry* entry = binop->op2;
                                unsigned int temp_reg = first_reg(entry);
                                write("mov %s, %s\n", registers_names[loged_width][2],
                                        registers_names[loged_width][temp_reg]);
                                clear_all_locations(entry);
                                entry->locs = 0;
                                write("%s %s, cl\n", int_arithmetic[binop->op_type],
                                        register_names[loged_width][result_reg]);
                                break;
                            }
                            case VARIABLE: {
                                struct SymTab_entry* entry = binop->op2;
                                if (in_reg(entry->locs)) {
                                    if (!(used_later(entry->info))) {
                                        unsigned int temp_reg = least_reg(entry);
                                        if (!(entry->locs & 1))
                                            store(entry);
                                        clear_all_locations(entry);
                                        write("mov , %s\n", registers_names[loged_width][2],
                                                registers_names[loged_width][temp_reg]);
                                        write("%s %s, cl\n", int_arithmetic[binop->op_type],
                                                register_names[loged_width][result_reg]);
                                        entry->locs = 1;
                                    }  else {
                                        unsigned int temp_reg = first_reg(entry);
                                        write("mov %s, %s\n", registers_names[loged_width][2],
                                                registers_names[loged_width][temp_reg]);
                                        write("%s %s, cl\n", int_arithmetic[binop->op_type],
                                                register_names[loged_width][result_reg]);
                                    }
                                } else if (used_later(entry->info)) {
                                    unsigned int temp_reg = get_reg(entry, ..);
                                    load(entry, temp_reg, width);
                                    write("mov %s, %s\n", registers_names[loged_width][2],
                                            registers_names[loged_width][temp_reg]);
                                    write("%s %s, cl\n", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg]);
                                } else {
                                    write("mov %s, %s [%s%u]",
                                            register_names[loged_width][2],
                                             ize_spec[loged_width],
                                            entry->key, entry->counter_value);
                                    write("%s %s, cl\n", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg]);

                                }
                                break;
                            }
                            case ICONSTANT: {
                                struct int_entry* entry = binop->op2;
                                if (width == 4)
                                    write("%s %s, %d", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg], size_spec[loged_width],
                                            (int)entry->offset);
                                else
                                    write("%s %s, %ld", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg], size_spec[loged_width],
                                            (int)entry->offset);
                            }
                        }
                    }
                }
            }
        }
    }
    binop->result->locs = result_reg << 1;
}







    unsigned int res_reg = get_reg(binop->result, TEMPORARY, width);
    write_int_mov(register_names[loged_width][res_reg], binop->op1, binop->op1_type, width);


    switch (binop->op_type) {
        case BINOP_PLUS:
        case BINOP_MINUS:
            write_int_arith(int_arithmetic[binop->op_type],
                            register_names[loged_width][res_reg],
                            binop->op2, binop->op2_type, width);
            break;
        case BINOP_MUL:
            break;
        case BINOP_DIV:
            break;
        case BINOP_MOD:
            break;
        case BINOP_XOR:
            break;
        case BINOP_AND:
            break;
        case BINOP_OR:
            break;
        case BINOP_SHR:
            break;
        case BINOP_SHL:
            break;
    }

    //unsigned int reg1 = get_reg(binop->op1, binop->op1_type, 0);
    //unsigned int reg2 = get_reg(binop->op1, binop->op1_type, 0);
}

void write_int_arith(const char* operator, char* destination, void* operand, enum SymbolType type, int width)
{
    write("%s %s, ", operator, destination);
    switch (type) {
        case VARIABLE: {
            struct SymTab_entry* entry = operand;
            write("[%s%u]\n", entry->key, entry->counter_value);
            return;
        }
        case TEMPORARY: {
            struct SymTab_entry* entry = operand;
            unsigned int reg_n = first_reg(entry);
            write("%s\n", register_names[loged_width][reg_n]);
            printf("\nbefore\n");
            print_registers();
            clear_all_locations(entry);
            printf("\nafter\n");
            print_registers();
            return;
        }
        case ICONSTANT: {
            struct int_entry* entry = operand;
            if (width == 4)
                write("dword %d\n", (int)(entry->val));
            else
                write("qword %ld\n", entry->val);
            return;
        }
    }
}

void write_int_mov(char* destination, void* operand, enum SymbolType type, int width)
{
    write("mov %s, ",destination);
    switch (type) {
        case VARIABLE: {
            struct SymTab_entry* entry = operand;
            write("[%s%u]\n", entry->key, entry->counter_value);
            return;
        }
        case TEMPORARY: {
            struct SymTab_entry* entry = operand;
            unsigned int reg_n = first_reg(entry);
            write("%s\n", register_names[loged_width][reg_n]);
            print_registers();
            clear_all_locations(entry);
            print_registers();
            return;
        }
        case ICONSTANT: {
            struct int_entry* entry = operand;
            if (width == 4)
                write("dword %d\n", (int)(entry->val));
            else
                write("qword %ld\n", entry->val);
            return;
        }
    }
}

static inline void emit_uop(struct UOpQuad* assign)
{
    return;
}

static inline void emit_conv(struct ConvQuad* conv)
{
    switch (conv->op_type) {
        case VARIABLE:
        case TEMPORARY:
        case FCONSTANT: {
            long width_and_type = conv->conversion_type;
            int type = width_and_type & 1;
            int width = width_and_type >> 2;
            int op_reg  = get_reg(conv->op, conv->op_type, width);
            /*
             * We only need to set the first elem of the regs states as reserved
             * since that makes the whole register reserved.
             */
            enum RegState temp_state = registers[op_reg].states[0];
            registers[op_reg].states[0] = REG_RESERVED;
            int res_reg = get_reg(conv->result, TEMPORARY, 0);
            registers[op_reg].states[0] = temp_state;
            printf("res: %lx, op: %lx\n", conv->result_info, conv->op_info);
            printf("%s = conv %s\n", register_names[3][res_reg], register_names[3][op_reg]);
        }
    }
    return;
}



void all_registers_are_reserved_error()
{
    fprintf(stderr, "internal error:All registers are reserved....\n");
    exit(-1);
}

void temp_not_computed_error(struct SymTab_entry* entry)
{
    fprintf(stderr, "internal error:Temporary '%s' has not been computed but wants register\n", entry->key);
    exit(-1);
}



void load_var(int reg, struct SymTab_entry* var, int type, int width);
void store(struct SymTab_entry* var, int reg);
void load_float(int reg, long float_loc, int width);

static inline unsigned int in_reg(unsigned int n)
{
    return n & ~1;
}

unsigned int get_reg(void* symbol, enum SymbolType type, int width)
{
    switch (type) {
        case VARIABLE: {
            struct SymTab_entry* entry = symbol;
            if (in_reg(entry->locs)) {
                printf("'%s%u' is in reg\n", entry->key, entry->counter_value);
                return first_reg(entry);
            }

            long width_and_type = entry->width_and_type;
            char type = width_and_type & 1;
            int width = width_and_type >> 2;
            int lreg = 16 * type;
            for (int i = 0; i < 16;) {
                if (max_state(lreg+i) == -1) {
                    printf("found uninit reg\n");
                    load_var(lreg+i, entry, type, width);
                    set_on_load(lreg+i, entry);
                    return lreg+i;
                }
            }
            unsigned int scores[16] = { 0 };
            for (int i = 0; i < 16; ) {
                for (int j = 0; j < registers[lreg+i].n_vals; ) {
                    switch (registers[lreg+i].states[j]) {
                        case REG_RESERVED:
                            scores[i] = 1 << 31;
                            goto outer;
                        case REG_VARIABLE: {
                            struct SymTab_entry* tmp = registers[lreg+i].vals[j];
                            if (resides_elsewhere(tmp->locs, lreg+i))
                                break;
                            /*
                             * Since we always assign to a new temporary,
                             * there is no need to check whether the lval
                             * resides in this register.
                             */
                            scores[i]++;
                            break;
                        }
                        case REG_TEMPORARY: {
                            /*
                             * A temporary can be used a most once and is
                             * cleared from the register once its value has been
                             * used, thus:
                             */
                            scores[i] = 1 << 31;
                            goto outer;
                        }
                        case REG_CONSTANT:
                            goto inner;
                    }
                    inner:
                    j++;
                }
                outer:
                i++;
            }
            int min_score = scores[0];
            int min_ind = 0;
            for (int i = 1; i < 16; i++) {
                if (scores[i] < min_score) {
                    min_score = scores[i];
                    min_ind = i;
                }
            }
            printf("var\n");
            //unload_reg(min_ind);
            return min_ind;
        }
        case TEMPORARY: {
            printf("temp\n");
            struct SymTab_entry* entry = symbol;
            if (in_reg(entry->locs))
                return first_reg(entry);
            long width_and_type = entry->width_and_type;
            char type = width_and_type & 1;
            int width = width_and_type >> 2;
            int lreg = 16 * type;
            for (int i = 0; i < 16; i++) {
                if (max_state(lreg+i) == -1) {
                    printf("reg name: %s\n", register_names[type+2][i]);
                    set_on_load(lreg+i, entry);
                    return lreg+i;
                }
            }
        }
        case FCONSTANT: {
            struct float_entry* entry = symbol;
            int lreg = 16;
            for (int i = 0; i < 16;) {
                if (max_state(lreg+i) == -1) {
                    printf("found uninit reg\n");
                    load_float(i, entry->offset, width);
                    //set_on_load(lreg+i, symbol);
                    return i;
                }
            }
        }
        default:
            printf("other %d\n", type);
            return  0;
    }
}
/*
int get_reg(void* symbol, enum SymbolType type, int width)
{
    /*
     * Register selection for operand. Looking at this code, overwriting of
     * destination operand would be a problem, but this is avoided by setting
     * that register to reserved.
     *//*
    printf("get reg\n");
    switch (type) {
        case VARIABLE: {
            struct SymTab_entry* var = symbol;
            if () {
                long width_and_type = var->width_and_type;
                char type = width_and_type & 1;
                int width = width_and_type >> 2;
                int lreg = 16 * type;
                for (int i = 0; i < 16; i++) {
                    if (max_state(lreg+i) == -1) {
                        load_var(lreg+i, var, type, width);
                        clear_and_set_reg(lreg+i, REG_VARIABLE, symbol);
                        return lreg+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (max_state(lreg+i) == REG_CONSTANT) {
                        load_var(lreg+i, var, type, width);
                        clear_and_set_reg(lreg+i, REG_VARIABLE, symbol);
                        return lreg+i;
                    }
                }

                // unsigned int usage[16] <- next usage
                for (int i = 0; i < 16; i++) {
                    if (max_state(lreg+i) == REG_VARIABLE) {
                        for (int j = 0; j < registers[lreg+i].n_vals; j++)
                            if (registers[lreg+i].states[j] == REG_VARIABLE)
                                store(registers[lreg+i].vals[i], lreg+i);
                        load_var(lreg+i, var, type, width);
                        registers[lreg+i].type = REG_VARIABLE;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                all_registers_are_reserved_error();
            } else {
                return var->reg_loc;
            }
        }
        case TEMPORARY: {
            struct SymTab_entry* var = symbol;
            if (var->reg_loc == -1){
                long width_and_type = var->width_and_type;
                char type = width_and_type & 1;
                int width = width_and_type >> 2;
                int lreg = 16 * type;

                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].value == NULL) {
                        registers[lreg+i].type = REG_TEMP;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].type == REG_CONSTANT) {
                        registers[lreg+i].type = REG_TEMP;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].type == REG_VARIABLE) {
                        store(registers[lreg+i].value, lreg+i);
                        registers[lreg+i].type = REG_TEMP;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                all_registers_are_reserved_error();
            } else {
                return var->reg_loc;
            }
        }

        case FCONSTANT: {
            struct float_entry* constant = symbol;
            if (constant->reg_loc == -1) {
                for (int i = 0; i < 16; i++) {
                    if (registers[i].value == NULL) {
                        load_float(i, constant->offset, width);
                        registers[i].type = REG_CONSTANT;
                        registers[i].value = symbol;
                        return i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[i].type == REG_CONSTANT) {
                        load_float(i, constant->offset, width);
                        registers[i].type = REG_CONSTANT;
                        registers[i].value = symbol;
                        return i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[i].type == REG_VARIABLE) {
                        store(registers[i].value, i);
                        load_float(i, constant->offset, 4);
                        registers[i].type = REG_VARIABLE;
                        registers[i].value = symbol;
                        return i;
                    }
                }
                all_registers_are_reserved_error();
            } else {
                return constant->reg_loc;
            }
        }
        case ICONSTANT:
        case SCONSTANT:
        case FUNCTION:
            fprintf(stderr, "SymbolType %d should'nt be in get_reg switch\n", type);
            exit(-1);
    }
}
*/
int get_a_and_d(struct SymTab_entry* var)
{

}

void load_var(int reg, struct SymTab_entry* var, int type, int width)
{
    if (type)
        if (width == 4)
            write("movss %s, [%s%u]\n", register_names[4][reg], var->key, var->counter_value);
        else
            write("movsd %s, [%s%u]\n", register_names[4][reg], var->key, var->counter_value);
    else
        write("mov %s, [%s%u]\n", register_names[loged_width][reg], var->key, var->counter_value);
}

void store(struct SymTab_entry* var, int reg)
{
    long width_and_type = var->width_and_type;
    int type = width_and_type & 1;
    int width = width_and_type >> 2;
    if (type)
        if (width == 4)
            write("movss [%s%u], %s\n", var->key, var->counter_value, register_names[4][reg]);
        else
            write("movsd [%s%u], %s\n", var->key, var->counter_value, register_names[4][reg]);
    else
        write("mov [%s%u], %s\n", var->key, var->counter_value, register_names[loged_width][reg]);
}

void load_float(int reg, long float_loc, int width)
{
    write("movsd %s, [dodouboblole+%ld]\n", register_names[4][reg], float_loc);
    if (width == 4)
        write("%s %s, %s\n", conv_float[1], register_names[4][reg], register_names[4][reg]);
}
