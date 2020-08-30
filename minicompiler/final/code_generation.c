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

#define DEBUG 1
#define SCREEN 0
/*
 * The code generator will generate Intel syntax assembly for the x86-64
 * architecture. Should rewrite_asm it later to generate an ".o" file directly
 */
//---------------------------------------
void load_int_to_reg(struct int_entry* entry, unsigned int loged_width, unsigned int reg_n)
{
    write_asm("mov %s, ", register_names[loged_width][reg_n]);
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write_asm("%d", (int)entry->val);
            break;
        case 3:
            write_asm("%ld", (long)entry->val);
            break;
    }
    write_asm("\n");
}

void load_int_to_mem(long entry_val, unsigned int loged_width, char* mem_loc_str)
{
    write_asm("mov %s %s", size_spec[loged_width], mem_loc_str);
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write_asm("%d", (int)entry_val);
            break;
        case 3:
            write_asm("%ld", (long)entry_val);
            break;
    }
    write_asm("\n");
}

static inline void get_memstr(char* memstr_loc, unsigned int isstatic, struct SymTab_entry* entry)
{
    if (isstatic)
        sprintf(memstr_loc, "[%s%u]", entry->key, entry->counter_value);
    else
        sprintf(memstr_loc, "[rbp+%ld]", entry->offset);
}

static inline void mov_float_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", mov[loged_width], register_names[4][dest-16],
            size_spec[loged_width], register_names[4][src-16]);
}

static inline void mov_float_mem_reg(unsigned int dest, char* src, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", mov[loged_width], register_names[4][dest-16],
            size_spec[loged_width], src);
}

static inline void float_arith_reg_reg(enum BinOpType operator, unsigned int op1,
                                        unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", float_arithmetic[loged_width-2][operator],
            register_names[4][op1-16], register_names[4][op2-16]);
}

static inline void float_arith_reg_mem(enum BinOpType operator, unsigned int op1,
                                    char* op2, unsigned int loged_width)
{
    write_asm("%s %s, %s %s\n", float_arithmetic[loged_width-2][operator],
            register_names[4][op1-16],size_spec[loged_width], op2);
}

static inline void int_arith_reg_reg(enum BinOpType operator, unsigned int op1, unsigned int op2, unsigned int loged_width)
{
    write_asm("%s %s, %s\n", int_arithmetic[operator], register_names[loged_width][op1],
                            register_names[loged_width][op2]);
}

static inline void int_arith_reg_mem(enum BinOpType operator, unsigned int op1, struct SymTab_entry* entry, unsigned int loged_width)
{
    write_asm("%s %s, %s [%s%u]\n", int_arithmetic[operator],
             register_names[loged_width][op1], size_spec[loged_width],
            entry->key, entry->counter_value);
}

static inline void int_arith_reg_const(enum BinOpType operator, unsigned int op1, struct int_entry* entry, unsigned int loged_width)
{
    write_asm("%s %s, %s ", int_arithmetic[operator], register_names[loged_width][op1], size_spec[loged_width]);
    switch (loged_width) {
        case 0:
        case 1:
        default:
            fprintf(stderr, "Internal error:8 and 16 bit ints are currently not supported\n");
            exit(-1);
        case 2:
            write_asm("%d", (int)entry->val);
            break;
        case 3:
            write_asm("%ld", (long)entry->val);
            break;
    }
    write_asm("\n");
}

static inline void mov_int_reg_reg(unsigned int dest, unsigned int src, unsigned int loged_width)
{
    write_asm("mov %s, %s\n", register_names[loged_width][dest],
                                register_names[loged_width][src]);
}

static inline void mov_int_mem_reg(unsigned int dest, char* src, unsigned int loged_width)
{
    write_asm("mov %s, %s %s\n", register_names[loged_width][dest], size_spec[loged_width], src);
}

//---------------------------------------

FILE* asm_file_desc;
void codegen();
void generate_assembly(const char* basename)
{
    for (int i = 0; i < 9; i++)
        printf("%s\n", int_arithmetic[i]);
    char filename[strlen(basename)+5];
    sprintf(filename, "%s.asm", basename);
    #if SCREEN
    asm_file_desc = stdout;/
    #else
    asm_file_desc = fopen(filename, "w");
    #endif
    init_registers();
    codegen();
    #if !SCREEN
    fclose(asm_file_desc);
    #endif
}

void write_asm(char* fstring, ...)
{
    va_list args;
    va_start(args, fstring);
    vfprintf(asm_file_desc, fstring, args);
    va_end(args);
}

void load_int();
void load_float(int reg, long float_loc, int width);
void load(struct SymTab_entry* entry, unsigned int reg, unsigned int type, unsigned int width);
int free_reg(unsigned int reg, struct SymTab_entry* entry, unsigned int type, unsigned int loged_width);
int get_free_reg(unsigned int type);
int get_reg(unsigned int type, unsigned int not_this_reg, unsigned int width,
            void* entry, enum SymbolType entry_type);

void alloc_statics(struct SymTab* main_symbol_table);
void write_asm_code(struct IC_entry* main);
void codegen()
{
    struct IC_entry* main = IC_table_get_entry(intermediate_code, "main");
    alloc_statics(main->symbol_table);
    write_asm_code(main);
}

void allocr(struct SymTab* symbol_table);
static inline void write_asm_float_consts()
{
    write_asm("\td_consts dq ");
    if (float_table->start) {
        struct entry_list* entry_l = float_table->start;
        for (; entry_l->next != NULL; entry_l = entry_l->next)
            write_asm("%lf, ", ((struct float_entry*)(entry_l->entry))->val);
        write_asm("\n");
    }
    write_asm("negf dd 0x80000000\n");
    write_asm("negd dq 0x8000000000000000\n");
}


void alloc_statics(struct SymTab* main_symbol_table)
{
    write_asm("section .data\n");
    allocr(main_symbol_table);
    write_asm("\n");

    /*
     * We only need to emit the float and double consts, since ints can
     * be directly.
     */
    write_asm_float_consts();
    write_asm("\n");
}
void write_asm_all_variables(struct SymTab* symbol_table);
void allocr(struct SymTab* symbol_table)
{
    write_asm_all_variables(symbol_table);
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



void write_asm_all_variables(struct SymTab* symbol_table)
{
    // NOTE: for 128 bit int and floats we need 'ddq' and 'dt' as well.
    char* declaration_widths[] = {"db", "dw", "dd", "dq"};
    for (int i = 0; i < symbol_table->table_size; i++) {
        struct SymTab_entry* entry = symbol_table->entries[i];
        for (; entry != NULL; entry = entry->next) {
            if (entry->type == TEMPORARY || entry->type == FUNCTION)
                continue;
            write_asm("\t%s%u %s 0\n", entry->key, entry->counter_value,
            declaration_widths[log2(entry->width_and_type >> 2)]);
        }
    }
}

void write_asm_func(struct IC_entry* func);
void write_asm_main(struct IC_entry* main);
void write_asm_code(struct IC_entry* main)
{
    write_asm("section .text\n");
    write_asm_main(main);
    for (int i = 0; i < intermediate_code->size; i++) {
        struct IC_entry* entry = intermediate_code->entries[i];
        while (entry != NULL) {
            if (strcmp(entry->key, "main") != 0)
                write_asm_func(entry);
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
void visit_block(struct BasicBlock* block, struct SymTab* symbol_table);
void write_asm_main(struct IC_entry* main)
{

    CREATE_INT_SET(set);
    write_asm("global main\n");
    write_asm("main:\n");
    for (int i = 0; i < main->n_blocks; i++)
        visit_block(main->basic_block_list[i], main->symbol_table);



}

void write_asm_func(struct IC_entry* func)
{
    struct int_set set;
    CREATE_INT_SET(set);
    write_asm("global %s\n", func->key);
    write_asm("%s:", func->key);
    write_asm("\n\n");
}

void emit_instruction(void* instruction, enum QuadType type);
void emit_assign(struct AssignQuad* assign);
void emit_binop(struct BinOpQuad* binop);
void emit_uop(struct UOpQuad* assign);
void emit_conv(struct ConvQuad* binop);
void emit_cond(struct CondQuad* cond,
            struct BasicBlock* true, struct BasicBlock* false);
void emit_uncond(struct BasicBlock* target);
int instr_num;
void visit_block(struct BasicBlock* block, struct SymTab* symbol_table)
{
    write_asm("L%p:\n", block);
    instr_num = 0;
    struct QuadList* curr = block->instructions;
    for (; TRUE; curr = curr->next) {
        if (curr->next == NULL)
            break;
        emit_instruction(curr->instruction, curr->type);
    }

    store_allr_in_symtab(symbol_table);

    if (block->jump_type != -1)
        if (block->jump_type == QUAD_UNCOND)
            emit_uncond(*(block->jump));
        else
            emit_cond(block->condition, *(block->true), *(block->false));
    for (int i = 0; i < 32; i++)
        clear_reg(i);

}

void emit_assign(struct AssignQuad* assign);
void emit_binop(struct BinOpQuad* binop);
void emit_uop(struct UOpQuad* assign);
void emit_conv(struct ConvQuad* binop);

void emit_instruction(void* instruction, enum QuadType type)
{
    switch (type) {
        case QUAD_ASSIGN:
            #if DEBUG
            printf("%d | emit assign\n", instr_num);
            #endif
            emit_assign(instruction);
            break;
        case QUAD_BINOP:
            #if DEBUG
            printf("%d | emit binop\n", instr_num);
            #endif
            emit_binop(instruction);
            break;
        case QUAD_UOP:
            #if DEBUG
            printf("%d | emit uop\n", instr_num);
            #endif
            emit_uop(instruction);
            break;
        case QUAD_CONV:
            #if DEBUG
            printf("%d | emit conv\n", instr_num);
            #endif
            emit_conv(instruction);
            break;
        case QUAD_COND:
            break;
        case QUAD_UNCOND:
            break;
        case QUAD_RETURN:
            break;
        case QUAD_PARAM:
            break;
        case QUAD_FUNC:
            break;
    }
    instr_num++;
}

void emit_assign(struct AssignQuad* assign)
{
    unsigned long width_and_type = assign->lval->width_and_type;
    int type = width_and_type & 1;
    int width = width_and_type >> 2;
    int loged_width = log2(width);
    remove_from_regs(assign->lval);

    switch (assign->rval_type) {
        case TEMPORARY: {
            struct SymTab_entry* rval_entry = assign->rval;

            assign->lval->reg_locs = rval_entry->reg_locs;
            assign->lval->mem_loc &= ~(1 << 1);

            clear_all_locations(rval_entry);
            int type_offset = 16 * type;
            for (int i = 0; i < 16; i++)
                if (assign->lval->reg_locs & (1 << (i+type_offset)))
                    append_to_reg(i+type_offset,
                                    assign->lval->type,
                                    assign->lval);
            return;
        }
        case VARIABLE:{
            struct SymTab_entry* rval_entry = assign->rval;

            if (!in_reg(rval_entry->reg_locs)) {
                unsigned int temp_reg = get_reg(type, -1, width, rval_entry, VARIABLE);
                char memstr_buff[800];
                get_memstr(&memstr_buff[0], rval_entry->mem_loc & 1, entry);
                if (type)
                    mov_float_mem_reg(dest, memstr_buff, loged_width);
                else
                    mov_int_mem_reg(dest, memstr_buff, loged_width);
                rval_entry->reg_locs |= 1 << temp_reg;
            }

            assign->lval->reg_locs = rval_entry->reg_locs;
            assign->lval->mem_loc &= ~(1 << 1);
            return;
        }

        case FCONSTANT: {
            struct float_entry* rval_entry = assign->rval;
            int new_reg = get_reg(type, -1, width, rval_entry, FCONSTANT);
            load_float(new_reg, rval_entry->offset, width);

            assign->lval->reg_locs = 1 << new_reg;
            return;
        }

        case ICONSTANT: {
            struct int_entry* rval_entry = assign->rval;
            if (!(assign->lval_info & 2)) {
                assign->lval->mem_loc &= ~(1 << 1);
                unsigned int new_reg = get_reg(type, -1, width, rval_entry, ICONSTANT);
                load_int_to_reg(rval_entry, loged_width, new_reg);
                assign->lval->reg_locs |= 1 << new_reg;
            } else {
                assign->lval->mem_loc |= 1 << 1;
                char memstr_buff[800];
                get_memstr(&memstr_buff[0], assign->lval->mem_loc & 1, assign->lval);
                load_int_to_mem(rval_entry->val, loged_width, memstr_buff);
            }
            return;
        }

        default:
            fprintf(stderr, "Internal error:SymbolType %d shouldn't be in assign rval\n", assign->rval_type);
            exit(-1);
    }
}

#define LOAD_TEMP_TO_RES(result, entry)\
    do { \
        result = first_reg(entry);\
        clear_all_locations(entry);\
        entry->reg_locs = 0;\
        entry->mem_loc = 1 << 1;\
        append_to_reg(result, entry->type, entry);\
    } while(0) \

#define VAR_GET_CLEAR_STORE_COND(result, entry)\
    do {\
        result = least_reg(entry);\
        if (!(entry->mem_loc & ((1 << 1) + 1)))\
            store(entry, result);\
        clear_all_locations(entry);\
        entry->reg_locs = 0;\
        entry->mem_loc |= 1 << 1;\
    } while(0)\

void emit_binop(struct BinOpQuad* binop)
{
    unsigned long width_and_type = binop->result->width_and_type;
    unsigned long width = width_and_type >> 2;
    unsigned long type = width_and_type & 1;
    unsigned int loged_width = log2(width);
    unsigned int result_reg;
    unsigned char free_reg_on_exit = 0;
    if (type) {
        switch (binop->op1_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = binop->op1;
                LOAD_TEMP_TO_RES(result_reg, entry);
                break;
            }
            case VARIABLE: {
                struct SymTab_entry* entry = binop->op1;
                if (in_reg(entry->reg_locs)) {
                    if (!(used_later(binop->op1_info))) {
                        VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        result_reg = least_reg(entry);
                    } else {
                        unsigned int temp_reg = least_reg(entry);
                        result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                        mov_float_reg_reg(result_reg, temp_reg, loged_width);
                    }
                } else {
                    result_reg = get_reg(type, -1, width, entry, VARIABLE);
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    mov_float_mem_reg(result_reg, memstr_buff, loged_width);
                }
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = binop->op1;
                result_reg = get_reg(type, -1, width, entry, FCONSTANT);
                /*
                 * Should add live and use info of float consts to reduce
                 * loads of the same constant. This approach will load the same
                 * constant over and over again.
                 */
                load_float(result_reg, entry->offset, width);
                break;
            }
            default:
                fprintf(stderr, "Internal error:SymbolType %d shouldn't be in float binop1\n", binop->op1_type);
                exit(-1);
        }
        switch (binop->op2_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = binop->op2;
                unsigned int op_reg = first_reg(entry);
                clear_all_locations(entry);
                entry->reg_locs = 0;
                float_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                break;
            }
            case VARIABLE: {
                struct SymTab_entry* entry = binop->op2;
                if (in_reg(entry->reg_locs)) {
                    unsigned int op_reg;
                    if (!(used_later(binop->op2_info))) {
                        VAR_GET_CLEAR_STORE_COND(op_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        op_reg = least_reg(entry);
                    } else {
                        op_reg = get_reg(type, result_reg, width, entry, VARIABLE);
                        unsigned int temp_reg = first_reg(entry);
                        mov_float_reg_reg(op_reg, temp_reg, loged_width);
                    }
                    float_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                } else if (used_later(binop->op2_info)) {
                    unsigned int op_reg = get_reg(type, -1, width, entry, VARIABLE);
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    mov_float_mem_reg(op_reg, memstr_buff, loged_width);
                    float_arith(binop->op_typec, result_reg, op_reg, loged_width);
                } else {
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    float_arith_reg_mem(binop->op_type, result_reg, memstr_buff, loged_width);
                }
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = binop->op2;
                if (binop->op1 == binop->op2) {
                    float_arith_reg_reg(binop->op_type, result_reg, result_reg, loged_width);
                } else if (width == 4) {
                    unsigned int op_reg = get_reg(type, result_reg, width, entry, FCONSTANT);
                    load_float(op_reg, entry->offset, width);
                    float_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                    clear_reg(op_reg);
                } else {
                    char floatstr_buff[800];
                    sprintf("[d_consts+%u]", entry->offset);
                    float_arith_reg_mem(binop->op_type, result_reg, floatstr_buff, loged_width);
                }
                break;
            }
            default:
                fprintf(stderr, "Internal error:SymbolType %d shouldn't be in float binop2\n", binop->op1_type);
                exit(-1);

        }
    } else {
        switch (binop->op_type) {
            case BINOP_PLUS:
            case BINOP_MINUS:
            case BINOP_MUL:
            case BINOP_AND:
            case BINOP_OR:
            case BINOP_XOR: {
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        LOAD_TEMP_TO_RES(result_reg, entry);
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->reg_locs)) {

                            if (!(used_later(binop->op1_info))) {
                                VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                            } else if (count_registers(entry->reg_locs) >= 2) {
                                result_reg = least_reg(entry);
                            } else {
                                unsigned int temp_reg = least_reg(entry);
                                result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                                mov_int_reg_reg(result_reg, temp_reg, loged_width);
                            }
                        } else {
                            result_reg = get_reg(type, -1, width, entry, VARIABLE);
                            char memstr_buff[800];
                            get_memstr(&memstr_buff[0], rval_entry->mem_loc & 1, entry);
                            mov_int_mem_reg(dest, memstr_buff, loged_width)
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        load_int_to_reg(entry, loged_width, result_reg);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be here\n", binop->op1_type);
                        exit(-1);
                }
                switch (binop->op2_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op2;
                        unsigned int op_reg = first_reg(entry);
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
                        int_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op2;
                        if (in_reg(entry->reg_locs)) {
                            unsigned int op_reg;
                            if (!(used_later(binop->op2_info))) {
                                VAR_GET_CLEAR_STORE_COND(op_reg, entry);
                            } else {
                                op_reg = first_reg(entry);
                            }
                            int_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                        } else if (used_later(binop->op2_info)) {
                            unsigned int op_reg = get_reg(type, -1, width, entry, VARIABLE);
                            char memstr_buff[800];
                            get_memstr(&memstr_buff[0], rval_entry->mem_loc & 1, entry);
                            mov_int_mem_reg(dest, memstr_buff, loged_width)
                            int_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                            entry->reg_locs |= 1 << op_reg;

                        } else {
                            int_arith_reg_mem(binop->op_type, result_reg, entry, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        int_arith_reg_const(binop->op_type, result_reg, entry, loged_width);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be in binop2\n", binop->op2_type);
                        exit(-1);
                }
                break;
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
                unsigned int op1_in_a = free_reg(0, binop->op1, type, loged_width);
                unsigned int op1_in_d = free_reg(3, binop->op1, type, loged_width);

                if (op1_in_a)
                    goto binop_select_2nd_op;
                if (op1_in_d) {
                    mov_int_reg_reg(0, 3, loged_width);
                    goto binop_select_2nd_op;
                }
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        unsigned int dividend = first_reg(entry);
                        mov_int_reg_reg(0, dividend, loged_width);
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        int dividend;
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->reg_locs)) {
                            struct SymTab_entry* entry = binop->op1;
                            dividend = first_reg(entry);
                            mov_int_reg_reg(0, dividend, loged_width);
                        } else if (used_later(binop->op1_info) &&
                                (dividend = get_free_reg(type)) != -1) {
                            mov_int_reg_reg(0, dividend, loged_width);
                        } else {
                            char memstr_buff[800];
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            mov_int_mem_reg(dest, memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        load_int_to_reg(entry, loged_width, 0);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be in binop1\n", binop->op1_type);
                        exit(-1);
                }
            binop_select_2nd_op:
                write_asm("%s\n", widen_to_ad[loged_width]);
                switch (binop->op2_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op2;
                        unsigned int divisior = first_reg(entry);

                        write_asm("idiv %s\n", register_names[loged_width][divisior]);
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
                        break;
                    }
                    case VARIABLE: {
                        int divisor;
                        struct SymTab_entry* entry = binop->op2;
                        if (in_reg(entry->reg_locs)) {
                            struct SymTab_entry* entry = binop->op2;
                            divisor = first_reg(entry);
                            write_asm("idiv %s\n",
                                    register_names[loged_width][divisor]);
                        } else if (used_later(binop->op2_info) &&
                                (divisor = get_free_reg(type)) != -1) {
                            write_asm("idiv %s\n",
                                    register_names[loged_width][divisor]);
                        } else {
                            write_asm("idiv %s [%s%u]\n",
                                size_spec[loged_width],
                                entry->key,
                                entry->counter_value);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        unsigned int op_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        if (width == 4)
                            write_asm("mov %s, %d\n",
                                register_names[loged_width][op_reg],
                                (int)entry->val);
                        else
                            write_asm("mov %s, %ld\n",
                                register_names[loged_width][op_reg],
                                entry->val);
                        write_asm("idiv %s\n", register_names[loged_width][op_reg]);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be binop2\n", binop->op2_type);
                        exit(-1);
                }
                if (binop->op_type == BINOP_DIV)
                    clear_reg(3);
                else
                    clear_reg(0);
                break;
            }
            case BINOP_SHL:
            case BINOP_SHR: {
                unsigned int op2_in_c = 0;;
                if (binop->op2_type != ICONSTANT)
                    op2_in_c = free_reg(2, binop->op2, type, loged_width);
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        LOAD_TEMP_TO_RES(result_reg, entry);
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = binop->op1;
                        if (in_reg(entry->reg_locs)) {
                            if (!(used_later(binop->op1_info))) {
                                printf("%s is not alive\n", entry->key);

                                VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                            } else if (count_registers(entry->reg_locs) >= 2) {
                                result_reg = least_reg(entry);
                            } else {
                                unsigned int temp_reg = least_reg(entry);
                                result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                                write_asm("mov %s, %s %s\n", register_names[loged_width][result_reg],
                                            size_spec[loged_width], register_names[loged_width][temp_reg]);
                            }
                        } else {
                            result_reg = get_reg(type, -1, width, entry, VARIABLE);
                            write_asm("mov %s, %s [%s%u]\n", register_names[loged_width][result_reg],
                                    size_spec[loged_width], entry->key, entry->counter_value);

                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        if (width == 4)
                            write_asm("mov %s, %d\n", register_names[loged_width][result_reg], (int)entry->val);
                        else
                            write_asm("mov %s, %ld\n", register_names[loged_width][result_reg], entry->val);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be uop1\n", binop->op1_type);
                        exit(-1);
                }
                if (op2_in_c) {
                    write_asm("%s %s, cl\n", int_arithmetic[binop->op_type], register_names[loged_width][result_reg]);
                } else {
                    switch (binop->op2_type) {
                        case TEMPORARY: {
                            struct SymTab_entry* entry = binop->op2;
                            unsigned int temp_reg = first_reg(entry);
                            write_asm("mov %s, %s\n", register_names[loged_width][2],
                                    register_names[loged_width][temp_reg]);
                            clear_all_locations(entry);
                            entry->reg_locs = 0;
                            write_asm("%s %s, cl\n", int_arithmetic[binop->op_type],
                                    register_names[loged_width][result_reg]);
                            break;
                        }
                        case VARIABLE: {
                            struct SymTab_entry* entry = binop->op2;
                            if (in_reg(entry->reg_locs)) {
                                if (!(used_later(binop->op2_info))) {
                                    unsigned int temp_reg;
                                    VAR_GET_CLEAR_STORE_COND(temp_reg, entry);

                                    write_asm("mov %s, %s\n", register_names[loged_width][2],
                                            register_names[loged_width][temp_reg]);
                                    write_asm("%s %s, cl\n", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg]);
                                }  else {
                                    unsigned int temp_reg = first_reg(entry);
                                    write_asm("mov %s, %s\n", register_names[loged_width][2],
                                            register_names[loged_width][temp_reg]);
                                    write_asm("%s %s, cl\n", int_arithmetic[binop->op_type],
                                            register_names[loged_width][result_reg]);
                                }
                            } else if (used_later(binop->op2_info)) {
                                unsigned int temp_reg = get_reg(type, -1, width, entry, VARIABLE);
                                load(entry, temp_reg, width, type);
                                write_asm("mov %s, %s\n", register_names[loged_width][2],
                                        register_names[loged_width][temp_reg]);
                                write_asm("%s %s, cl\n", int_arithmetic[binop->op_type],
                                        register_names[loged_width][result_reg]);
                            } else {
                                write_asm("mov %s, %s [%s%u]\n",
                                        register_names[loged_width][2],
                                        size_spec[loged_width],
                                        entry->key, entry->counter_value);
                                write_asm("%s %s, cl\n", int_arithmetic[binop->op_type],
                                        register_names[loged_width][result_reg]);
                            }
                            break;
                        }
                        case ICONSTANT: {
                            struct int_entry* entry = binop->op2;
                            if (width == 4)
                                write_asm("%s %s, %d\n", int_arithmetic[binop->op_type],
                                        register_names[loged_width][result_reg],
                                        (int)entry->val);

                            else
                                write_asm("%s %s, %ld\n", int_arithmetic[binop->op_type],
                                        register_names[loged_width][result_reg],
                                        entry->val);
                            break;
                        }
                        default:
                            fprintf(stderr, "Internal error:SymbolType %d shouldn't be here\n", binop->op2_type);
                            exit(-1);
                    }
                }

            }
        }
    }
    binop->result->reg_locs = 1 << result_reg;


}

void emit_uop(struct UOpQuad* uop)
{
    unsigned long width_and_type = uop->result->width_and_type;
    unsigned long width = width_and_type >> 2;
    unsigned long type = width_and_type & 1;
    unsigned int loged_width = log2(width);
    unsigned int result_reg;
    if (type) {
        // Operator can only be negation
        switch (uop->operand_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = uop->operand;
                LOAD_TEMP_TO_RES(result_reg, entry);
                break;
            }
            case VARIABLE: {//note
                struct SymTab_entry* entry = uop->operand;
                if (in_reg(entry->reg_locs)) {
                    if (!(used_later(uop->operand_info))) {
                        VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        result_reg = least_reg(entry);
                    } else {
                        unsigned int temp_reg = least_reg(entry);
                        result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);

                        if (width == 4)
                            write_asm("movss %s, %s %s\n", register_names[4][result_reg-16],
                                    size_spec[2], register_names[4][temp_reg-16]);
                        else
                            write_asm("movsd %s, %s %s\n", register_names[4][result_reg-16],
                                    size_spec[3], register_names[4][temp_reg-16]);
                    }
                } else {
                    result_reg = get_reg(type, -1, width, entry, VARIABLE);
                    if (width == 4)
                        write_asm("movss %s, %s [%s%u]\n", register_names[4][result_reg-16],
                                size_spec[2], entry->key, entry->counter_value);
                    else
                        write_asm("movsd %s, %s [%s%u]\n", register_names[4][result_reg-16],
                                size_spec[3], entry->key, entry->counter_value);
                }
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = uop->operand;
                result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                /*
                 * Should add live and use info of float consts to reduce
                 * loads of the same constant. This approach will load the same
                 * constant over and over again.
                 */
                load_float(result_reg, entry->offset, width);
                break;
            }
            default:
                fprintf(stderr, "Internal error:SymbolType %d shouldn't be uop1\n", uop->operand_type);
                exit(-1);

        }
    if (width == 4)
        write_asm("xorps %s, %s [negf]\n", register_names[4][result_reg-16], size_spec[2]);
    else
        write_asm("xorps %s, %s [negd]\n",register_names[4][result_reg-16], size_spec[3]);
    } else {
        switch (uop->operator_type) {
            case UOP_NEG:
            case UOP_NOT:
                switch (uop->operand_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = uop->operand;
                        LOAD_TEMP_TO_RES(result_reg, entry);
                        break;
                    }
                    case VARIABLE: {
                        struct SymTab_entry* entry = uop->operand;
                        if (in_reg(entry->reg_locs)) {
                            if (!(used_later(uop->operand_info))) {
                                VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                            } else if (count_registers(entry->reg_locs) >= 2) {
                                result_reg = least_reg(entry);
                            } else {
                                unsigned int temp_reg = least_reg(entry);
                                result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                                write_asm("mov %s, %s %s\n", register_names[loged_width][result_reg],
                                            size_spec[loged_width], register_names[loged_width][temp_reg]);
                            }
                        } else {
                            result_reg = get_reg(type, -1, width, entry, VARIABLE);
                            write_asm("mov %s, %s [%s%u]\n", register_names[loged_width][result_reg],
                                    size_spec[loged_width], entry->key, entry->counter_value);

                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = uop->operand;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        if (width == 4)
                            write_asm("mov %s, %d\n", register_names[loged_width][result_reg], (int)entry->val);
                        else
                            write_asm("mov %s, %ld\n", register_names[loged_width][result_reg], entry->val);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be uop1\n", uop->operand_type);
                        exit(-1);
                }
                break;
        }
        write_asm("%s %s\n", int_unary[uop->operator_type], register_names[loged_width][result_reg]);
    }
    uop->result->reg_locs = 1 << result_reg  ;
}

void emit_conv(struct ConvQuad* conv)
{
    unsigned long width_and_type = conv->conversion_type;
    unsigned int new_type = width_and_type & 1;
    unsigned int new_width = width_and_type >> 2;
    unsigned int new_loged_width = log2(new_width);
    unsigned int result_reg;
    int clear_reg_on_exit = -1;
    switch (conv->op_type) {
        case TEMPORARY: {
            struct SymTab_entry* entry = conv->op;
            long op_width_and_type = entry->width_and_type;
            int op_type = op_width_and_type & 1;
            int op_width = op_width_and_type >> 2;
            int new_loged_width = log2(op_width);
            if (new_type) {
                if (op_type) {
                    result_reg = first_reg(entry);
                    clear_all_locations(entry);
                    write_asm("%s %s, %s\n", conv_float[new_loged_width-2],
                        register_names[4][result_reg-16],
                        register_names[4][result_reg-16]);
                } else {
                    int op_reg = first_reg(entry);
                    clear_all_locations(entry);
                    result_reg = get_reg(op_type, op_reg, new_width, entry, TEMPORARY);
                    write_asm("%s %s, %s\n", conv_float_int[new_loged_width-2],
                            register_names[new_loged_width][result_reg-16],
                            register_names[4][op_reg-16]);
                }
            } else {
                if (op_type) {
                    int op_reg = first_reg(entry);
                    clear_all_locations(entry);
                    result_reg = get_reg(op_type, op_reg, new_width, entry, TEMPORARY);
                    write_asm("%s %s, %s\n", conv_float_int[new_loged_width],
                            register_names[4][result_reg-16],
                            register_names[new_loged_width][op_reg-16]);

                } else {
                    result_reg = first_reg(entry);
                    clear_all_locations(entry);
                    if (new_width < op_width)
                        write_asm("%s %s, %s\n", conv_signed[1],
                                register_names[3][result_reg-16],
                                register_names[2][result_reg-16]);

                }
            }
            break;
        }
        case VARIABLE: {
            struct SymTab_entry* entry = conv->op;
            long op_width_and_type = entry->width_and_type;
            int op_type = op_width_and_type & 1;
            int op_width = op_width_and_type >> 2;
            int new_loged_width = log2(new_width);
            if (in_reg(entry->reg_locs)) {
                int op_reg;
                if (!(used_later(conv->op_info))) {
                    printf("stored cond\n");
                    VAR_GET_CLEAR_STORE_COND(op_reg, entry);
                } else if (count_registers(entry->reg_locs) >= 2) {
                    op_reg = least_reg(entry);
                } else {
                    unsigned int temp_reg = least_reg(entry);
                    op_reg = get_reg(op_type, temp_reg, op_width, entry, VARIABLE);
                    write_asm("%s %s, %s\n", mov[op_type*new_loged_width],
                            register_names[new_loged_width*(!op_type)+4*op_type][op_reg-16*op_type],
                            register_names[new_loged_width*(!op_type)+4*op_type][temp_reg-16*op_type]);
                    append_reg_to_symbol(entry, op_reg);
                }
                if (new_type) {
                    if (op_type) {
                        printf("new & op\n");
                        write_asm("%s %s, %s\n", conv_float[new_loged_width-2],
                            register_names[4][op_reg-16],
                            register_names[4][op_reg-16]);
                        result_reg = op_reg;
                        clear_reg(op_reg);
                    } else {
                        printf("new\n");

                        result_reg = get_reg(op_type, -1, new_width, entry, VARIABLE);
                        write_asm("%s %s, %s\n", conv_float_int[new_loged_width],
                                register_names[new_loged_width][result_reg-16],
                                register_names[4][op_reg]);
                    }
                } else {
                    if (op_type) {
                        result_reg = get_reg(new_type, -1, new_width, entry, VARIABLE);
                        write_asm("%s %s, %s\n", conv_float_int[new_loged_width-2],
                                register_names[new_loged_width][result_reg],
                                register_names[4][op_reg-16]);
                    } else {
                        printf("none \n");

                        if (new_width < op_width)
                            write_asm("%s %s, %s\n", conv_signed[1],
                                    register_names[3][op_reg],
                                    register_names[2][op_reg]);
                        result_reg = op_reg;
                        clear_reg(op_reg);
                    }
                }
            } else {
                result_reg = get_reg(new_type, -1, new_width, entry, VARIABLE);
                if (new_type) {
                    if (op_type) {
                        write_asm("%s %s, %s [%s%u]\n", conv_float[new_loged_width-2],
                            register_names[4][result_reg-16],
                            size_spec[new_loged_width], entry->key,
                            entry->counter_value);
                    } else {
                        write_asm("%s %s, %s [%s%u]\n", conv_float_int[new_loged_width-2],
                                register_names[4][result_reg-16],
                                size_spec[new_loged_width],
                                entry->key, entry->counter_value);
                    }
                } else {
                    if (op_type) {
                        write_asm("%s %s, %s [%s%u]\n", conv_float_int[new_loged_width-2],
                                register_names[4][result_reg-16],
                                size_spec[new_loged_width],
                                entry->key, entry->counter_value);

                    } else {
                        if (new_width < op_width)
                            write_asm("%s %s, %s [%s%u]\n", conv_signed[1],
                                    register_names[3][result_reg],
                                    size_spec[new_loged_width],
                                    entry->key, entry->counter_value);
                    }
                }
            }
            break;
        }
        case FCONSTANT: {
            struct float_entry* entry = conv->op;
            result_reg  = get_reg(new_type, -1, new_width, entry, FCONSTANT);
            if (new_type) {
                write_asm("cvtsd2ss %s, %s [d_consts+%u]\n",
                    register_names[4][result_reg-16],
                    size_spec[new_loged_width], entry->offset);
            } else {
                write_asm("cvtsd2si %s, %s [d_consts+%u]\n",
                        register_names[new_loged_width][result_reg],
                        size_spec[new_loged_width],
                        entry->offset);
            }
            break;
        }
        case ICONSTANT: {
            struct int_entry* entry = conv->op;
            result_reg = get_reg(new_type, -1, new_width, entry, ICONSTANT);
            if (new_type) {
                printf("iconst\n");
                unsigned int op_reg = get_reg(0, -1, new_width, entry, ICONSTANT);
                write_asm("mov %s, %d\n", register_names[2][op_reg], entry->val);
                write_asm("%s %s, %s\n", conv_float_int[new_loged_width],
                    register_names[4][result_reg-16],
                    register_names[2][op_reg]);
            } else {
                write_asm("mov %s, %d\n",
                        register_names[new_loged_width][result_reg],
                        entry->val);
            }
            break;
        }
        default:
            fprintf(stderr, "Internal error:SymbolType %d shouldn't be in conv\n", conv->op_type);
            exit(-1);
    }
    conv->result->reg_locs = 1 << result_reg;
    return;
}

#define VAR_FLOAT_OP1_REG(var, info, reg_name, width, type)\
    do {\
        struct SymTab_entry* entry = var;\
        if (in_reg(entry->reg_locs)) {\
            if (!(used_later(info))) {\
                VAR_GET_CLEAR_STORE_COND(reg_name, entry);\
            } else if (count_registers(entry->reg_locs) >= 2) {\
                reg_name = least_reg(entry);\
            } else {\
                unsigned int temp_reg = least_reg(entry);\
                reg_name = get_reg(type, temp_reg, width, entry, VARIABLE);\
                if (width == 4)\
                    write_asm("movss %s, %s %s\n", register_names[4][reg_name-16],\
                            size_spec[2], register_names[4][temp_reg-16]);\
                else\
                    write_asm("movsd %s, %s %s\n", register_names[4][reg_name-16],\
                            size_spec[3], register_names[4][temp_reg-16]);\
            }\
        } else {\
            reg_name = get_reg(type, -1, width, entry, VARIABLE);\
            if (width == 4)\
                write_asm("movss %s, %s [%s%u]\n", register_names[4][reg_name-16],\
                        size_spec[2], entry->key, entry->counter_value);\
            else\
                write_asm("movsd %s, %s [%s%u]\n", register_names[4][reg_name-16],\
                        size_spec[3], entry->key, entry->counter_value);\
        }\
    } while (0)\


void emit_cond(struct CondQuad* cond,
            struct BasicBlock* true, struct BasicBlock* false)
{
    struct SymTab_entry* op1;
    unsigned int type;
    unsigned int width;
    if (cond->op1_type == VARIABLE || cond->op1_type == TEMPORARY) {
        unsigned long w_n_t = ((struct SymTab_entry*)cond->op1)->width_and_type;
        type = w_n_t & 1;
        width = w_n_t >> 2;
    } else if (cond->op2_type == VARIABLE || cond->op2_type == TEMPORARY) {
        unsigned long w_n_t = ((struct SymTab_entry*)cond->op2)->width_and_type;
        type = w_n_t & 1;
        width = w_n_t >> 2;
    } else {
        fprintf(stderr, "Internal error:No constant folding on dumb comparisons yet huh?\n");
        exit(-1);
    }
    unsigned int loged_width = log2(width);
    if (type) {
        unsigned int op1_reg;
        switch (cond->op1_type) {

            case TEMPORARY: {
                struct SymTab_entry* entry = cond->op1;
                op1_reg = first_reg(entry);
                break;
            }
                break;
            case VARIABLE: {
                VAR_FLOAT_OP1_REG(cond->op1, cond->op1_info, op1_reg, width, type);
                break;
            }
            case FCONSTANT: {
                struct float_entry* entry = cond->op1;
                op1_reg = get_reg(type, -1, width, entry, FCONSTANT);
                load_float(op1_reg, entry->offset, width);
                break;
            }
            default:
                fprintf(stderr, "Internal error:Did not expect SymbolType %d in cond float op1 switch\n", cond->op1_type);
                exit(-1);
        }
        switch (cond->op2_type) {
            case TEMPORARY: {
                    struct SymTab_entry* entry = cond->op2;
                    unsigned int temp_reg = first_reg(entry);
                    write_asm("%s %s, %s\n", float_control[loged_width-2],
                        register_names[4][op1_reg-16], register_names[4][temp_reg-16]);
                    break;
                }

            case VARIABLE: {
                struct SymTab_entry* entry = cond->op2;
                if (in_reg(entry->reg_locs)) {
                    unsigned int op2_reg;
                    if (!(used_later(cond->op2_info))) {
                        VAR_GET_CLEAR_STORE_COND(op2_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        op2_reg = least_reg(entry);
                    } else {
                        op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                        unsigned int temp_reg = first_reg(entry);
                        write_asm("%s %s, %s\n", mov[loged_width],
                                register_names[4][op1_reg-16], register_names[4][temp_reg-16]);
                    }
                    write_asm("%s %s, %s", float_control[loged_width-2],
                        register_names[4][op1_reg-16], register_names[4][op2_reg-16]);
                } else if (used_later(cond->op2_info)) {
                    unsigned int op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                    write_asm("%s %s, %s [%s%u]\n", mov[loged_width], register_names[4][op2_reg-16],
                            size_spec[loged_width], entry->key, entry->counter_value);
                    write_asm("%s %s, %s\n", float_control[loged_width-2],
                        register_names[4][op1_reg-16], register_names[4][op2_reg-16]);
                } else {
                    write_asm("%s %s, %s [%s%u]\n", float_control[loged_width-2],
                        register_names[4][op1_reg-16], size_spec[loged_width],
                            entry->key, entry->counter_value);
                }
                break;
            }

            case FCONSTANT: {
                struct float_entry* entry = cond->op2;
                if (width == 4)
                    write_asm("%s %s, [f_consts+%u]\n", float_control[loged_width-2],
                            register_names[4][op1_reg-16], entry->offset);
                else
                    write_asm("%s %s, [d_consts+%u]\n", float_control[loged_width-2],
                            register_names[4][op1_reg-16], entry->offset);
                break;
            }
            default:
                fprintf(stderr, "Internal error:Did not expect SymbolType %d in float op2 switch\n",
                        cond->op2_type);
                exit(-1);
        }
        write_asm("%s L%p\n", cond_jumps[1][cond->op_type], true);
        write_asm("jmp L%p\n", false);
    } else {
        unsigned int op1_reg;
        switch (cond->op1_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = cond->op1;
                op1_reg = first_reg(entry);
                break;
            }

            case VARIABLE: {
                struct SymTab_entry* entry = cond->op1;
                if (in_reg(entry->reg_locs)) {
                    if (!(used_later(cond->op1_info))) {
                        VAR_GET_CLEAR_STORE_COND(op1_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        op1_reg = least_reg(entry);
                    } else {
                        unsigned int temp_reg = least_reg(entry);
                        op1_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                        write_asm("mov %s, %s %s\n", register_names[loged_width][op1_reg],
                                size_spec[2], register_names[loged_width][temp_reg]);
                    }
                } else {
                    op1_reg = get_reg(type, -1, width, entry, VARIABLE);
                    write_asm("mov %s, %s [%s%u]\n", register_names[loged_width][op1_reg],
                            size_spec[2], entry->key, entry->counter_value);
                }
                break;
            }

            case ICONSTANT: {
                struct int_entry* entry = cond->op1;
                op1_reg = get_reg(type, -1, width, entry, ICONSTANT);
                if (width == 4)
                    write_asm("mov %s, %d\n", register_names[loged_width][op1_reg], (int)entry->val);
                else
                    write_asm("mov %s, %ld\n", register_names[loged_width][op1_reg], entry->val);
                break;
            }
            default:
                fprintf(stderr, "Internal error:Did not expect SymbolType %d in int op1 switch\n",
                        cond->op1_type);
                exit(-1);
        }
        switch (cond->op2_type) {
            case TEMPORARY: {
                struct SymTab_entry* entry = cond->op2;
                unsigned int temp_reg = first_reg(entry);
                write_asm("%s %s, %s\n", float_control[0],
                    register_names[loged_width][op1_reg],
                    register_names[loged_width][temp_reg]);
                break;
            }

            case VARIABLE: {
                struct SymTab_entry* entry = cond->op2;
                if (in_reg(entry->reg_locs)) {
                    unsigned int op2_reg;
                    if (!(used_later(cond->op2_info))) {
                        VAR_GET_CLEAR_STORE_COND(op2_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        op2_reg = least_reg(entry);
                    } else {
                        op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                        unsigned int temp_reg = first_reg(entry);
                        write_asm("mov %s, %s\n", register_names[loged_width][op1_reg],
                                register_names[loged_width][temp_reg]);
                    }
                    write_asm("%s %s, %s\n", int_control[0],
                        register_names[loged_width][op1_reg],
                        register_names[loged_width][op2_reg]);
                } else if (used_later(cond->op2_info)) {
                    unsigned int op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                    write_asm("mov %s, %s [%s%u]\n", register_names[loged_width][op2_reg],
                            size_spec[loged_width], entry->key, entry->counter_value);
                    write_asm("%s %s, %s", int_control[0],
                        register_names[loged_width][op1_reg],
                        register_names[loged_width][op2_reg]);
                } else {
                    write_asm("%s %s, %s [%s%u]\n", int_control[0],
                        register_names[loged_width][op1_reg], size_spec[loged_width],
                            entry->key, entry->counter_value);
                }
                break;
            }

            case ICONSTANT: {
                struct int_entry* entry = cond->op2;
                if (width == 4)
                    write_asm("%s %s, %d\n", int_control[0],
                            register_names[loged_width][op1_reg],
                            (int)entry->val);
                else
                    write_asm("%s %s, %ld\n", int_control[0],
                            register_names[loged_width][op1_reg],
                            entry->val);
                break;
            }

            default:
            fprintf(stderr, "Internal error:Did not expect SymbolType %d in int op2 switch\n",
                    cond->op2_type);
                exit(-1);
        }
    }
    write_asm("%s L%p\n", cond_jumps[0][cond->op_type], true);
    write_asm("jmp L%p\n", false);
}

void emit_uncond(struct BasicBlock* target)
{
    write_asm("jmp L%p\n", target);
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




int get_reg(unsigned int type, unsigned int not_this_reg, unsigned int width,
            void* entry, enum SymbolType entry_type)
{
    const int type_offset = type * 16;
    for (int i = 0; i < 16; i++)
        if (type_offset+i != not_this_reg &&
            get_reg_state(type_offset+i) == REG_FREE) {
            registers[type_offset + i].reg_state = REG_OCCUPIED;
            registers[type_offset + i].reg_width = width;
            append_to_reg(type_offset + i, entry_type, entry);
            return type_offset+i;
        }


    unsigned int scores[16] = { 0 };
    for (int i = 0; i < 16; i++) {

        if (i+type_offset == not_this_reg) {
            scores[i] = MAX_UINT;
            continue;
        }
        switch (get_reg_state(type_offset+i)) {
            case REG_RESERVED:
                scores[i] = MAX_UINT;
                break;
            case REG_SAVED:
                /*
                 * Pushing and popping the stack is two memory accesses.
                 */
                scores[i] = 2;
                break;
            case REG_OCCUPIED:
                for (int j = 0; j < registers[type_offset+i].n_vals; i++) {
                    if (registers[type_offset+i].states[j] != REG_CONSTANT) {
                        struct SymTab_entry* tmp_entry = registers[type_offset+i].vals[j];
                        if (in_reg(tmp_entry->reg_locs))
                            continue;
                        scores[i]++;
                    }

                }
        }
        int min_score = scores[0];
        int min_ind = 0;
        for (int i = 1; i < 16; i++) {
            if (scores[i] < min_score) {
                min_score = scores[i];
                min_ind = i;
            }
        }
        store_all(min_ind, NULL);
        registers[type_offset + min_ind].reg_width = width;
        registers[min_ind + type_offset].reg_state = REG_OCCUPIED;
        append_to_reg(type_offset + i, entry_type, entry);
        return min_ind + type_offset;
    }
}

int get_free_reg(unsigned int type)
{
    const int type_offset = type * 16;
    for (int i = 0; i < 16; i++)
        if (get_reg_state(type_offset+i) == REG_FREE)
            return type_offset+i;
    return -1;
}

static inline void free_reg_reserved_error(unsigned int reg, unsigned int type)
{
    fprintf(stderr, "Internal error:Register '%s' needed for computation is reserved\n",
    register_names[4*type + 3*(!type)][reg- 16*type]);
}

#define MOV_REG_REG(reg1, reg2, type, loged_width)\
    do {\
        write_asm("%s %s, %s\n", mov[type*loged_width],\
            register_names[loged_width*(!type)+4*type][reg1-16*type],\
            register_names[loged_width*(!type)+4*type][reg2-16*type]);\
    } while(0)\

int free_reg(unsigned int reg, struct SymTab_entry* entry, unsigned int type, unsigned int loged_width)
{
    printf("get reg\n");
    switch (get_reg_state(reg)) {
        case REG_FREE:
            #if DEBUG
            printf("register '%d' is free\n", reg);
                    //register_names[reg >= 16 + 2][reg - 16 * (reg >= 16)]);
            #endif
            registers[reg].reg_state = REG_OCCUPIED;
            return 0;
        case REG_OCCUPIED: {
            #if DEBUG
            printf("register '%s' is occupied\n",
                register_names[loged_width*(!type)+4*type][reg-16*type]);
            #endif
            unsigned int dest = get_reg(type, reg, registers[reg].reg_width, entry, entry->type);
            int entry_in_reg = copy_reg_to_reg(dest, reg, entry);
            unsigned int loged_width = log2(registers[reg].reg_width);
            MOV_REG_REG(dest, reg, type, loged_width);
            registers[reg].reg_state = REG_OCCUPIED;
            return entry_in_reg;
        }

        case REG_SAVED: {
            // push register
            // indicate popping in epilouge
            #if DEBUG
            printf("register is saved\n");
            #endif
            registers[reg].reg_state = REG_OCCUPIED;
        }
        case REG_RESERVED: {
            #if DEBUG
            printf("register is reserved\n");
            #endif
            unsigned int type = entry->width_and_type & 1;
            free_reg_reserved_error(reg, type);
        }
    }
}


void store(struct SymTab_entry* var, int reg)
{
    long width_and_type = var->width_and_type;
    int type = width_and_type & 1;
    int width = width_and_type >> 2;
    if (type)
        if (width == 4)
            write_asm("movss [%s%u], %s\n", var->key, var->counter_value, register_names[4][reg-16]);
        else
            write_asm("movsd [%s%u], %s\n", var->key, var->counter_value, register_names[4][reg-16]);
    else
        write_asm("mov [%s%u], %s\n", var->key, var->counter_value, register_names[log2(width)][reg]);
}

void load_float(int reg, long float_loc, int width)
{
    write_asm("movsd %s, [d_consts+%ld]\n", register_names[4][reg-16], float_loc);
    if (width == 4)
        write_asm("%s %s, %s\n", conv_float[1], register_names[4][reg-16], register_names[4][reg-16]);
}

void load(struct SymTab_entry* entry, unsigned int reg,
            unsigned int type, unsigned int width)
{
    write_asm("%s %s, %s [%s%u]\n", mov[type*log2(width)],
            register_names[(!type)*log2(width) + type*4][reg-type*16],
            size_spec[log2(width)], entry->key, entry->counter_value);

}
