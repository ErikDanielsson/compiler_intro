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
#include "write_asm.h"

#define DEBUG 1
#define SCREEN 0
/*
 * The code generator will generate Intel syntax assembly for the x86-64
 * architecture. Should rewrite it later to generate an ".o" file directly
 */

void codegen();
void generate_assembly(const char* basename)
{
    for (int i = 0; i < 9; i++)
        printf("%s\n", int_arithmetic[i]);
    char filename[strlen(basename)+5];
    sprintf(filename, "%s.asm", basename);
    #if SCREEN
    FILE* asm_file_desc = stdout;
    #else
    FILE* asm_file_desc = fopen(filename, "w");
    #endif
    init_asm_writer(asm_file_desc);
    init_registers();
    codegen();
    #if !SCREEN
    fclose(asm_file_desc);
    #endif
}

void load_int();
void load_float(int reg, long float_loc, int width);
void load(struct SymTab_entry* entry, unsigned int reg, unsigned int type, unsigned int width);

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
     * be used directly.
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

/*
 * DISCLAIMER: Codegen for funcs is not implemented yet due to some difficultites
 * proc call which are in line with calling conventions. The IC does not support this yet.
 */
//void prologue(struct IC_entry* func);
//void epilogue(struct IC_entry* func);
void write_asm_func(struct IC_entry* func)
{
    struct int_set set;
    CREATE_INT_SET(set);
    write_asm("global %s\n", func->key);
    write_asm("%s:", func->key);
    write_asm("\n\n");
    //prologue(func);
    //for (int i = 0; i < func->n_blocks; i++)
    //    visit_block(func->basic_block_list[i], func->symbol_table);
    //epilogue(func);
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
unsigned int global_param_counter[2] = {0, 0};
void visit_block(struct BasicBlock* block, struct SymTab* symbol_table)
{
//    global_param_counter = 0;
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
void emit_uop(struct UOpQuad* uop);
void emit_conv(struct ConvQuad* conv);
void emit_param(struct ParamQuad* param);

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
        write_asm("ret\n");
            break;
        case QUAD_PARAM:
            emit_param(instruction);
            write_asm("param\n");
            //global_param_counter++;
            break;
        case QUAD_FUNC:
        write_asm("call func\n");
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
                get_memstr(&memstr_buff[0], rval_entry->mem_loc & 1, rval_entry);
                if (type)
                    mov_float_mem_reg(temp_reg, memstr_buff, loged_width);
                else
                    mov_int_mem_reg(temp_reg, memstr_buff, loged_width);
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
                mov_int_const_reg(new_reg, rval_entry->val, loged_width);
                assign->lval->reg_locs |= 1 << new_reg;
            } else {
                assign->lval->mem_loc |= 1 << 1;
                char memstr_buff[800];
                get_memstr(&memstr_buff[0], assign->lval->mem_loc & 1, assign->lval);
                mov_int_const_mem(rval_entry->val, loged_width, memstr_buff);
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
/*
 * Emits binop. Special cases for div, mod and shift due to architecture.
 */
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
                    float_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
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
                    sprintf(floatstr_buff, "[d_consts+%u]", entry->offset);
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
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            mov_int_mem_reg(result_reg, memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        mov_int_const_reg(result_reg, entry->val, loged_width);
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
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            mov_int_mem_reg(op_reg, memstr_buff, loged_width);
                            int_arith_reg_reg(binop->op_type, result_reg, op_reg, loged_width);
                            entry->reg_locs |= 1 << op_reg;

                        } else {
                            int_arith_reg_mem(binop->op_type, result_reg, entry, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        int_arith_reg_const(binop->op_type, result_reg, entry->val, loged_width);
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
                 * Division requires the 'a' and 'd' registers, hence they must be freed.
                 */
                unsigned int op1_in_a = free_reg(0, binop->op1, type, loged_width);
                unsigned int op1_in_d = free_reg(3, binop->op1, type, loged_width);

                if (op1_in_a){
                    if (binop->op1_type == TEMPORARY) {
                        clear_all_locations(binop->op1);
                        struct SymTab_entry* entry = binop->op1;
                        entry->reg_locs = 0;
                    }
                    goto binop_select_2nd_op;
                }

                if (op1_in_d) {
                    if (binop->op1_type == TEMPORARY) {
                        struct SymTab_entry* entry = binop->op1;
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
                    }
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
                            mov_int_mem_reg(0, memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        mov_int_const_reg(0, entry->val, loged_width);
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
                        unsigned int divisor = first_reg(entry);
                        divmod_int_reg(divisor, loged_width);
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
                            divmod_int_reg(divisor, loged_width);
                        } else if (used_later(binop->op2_info) &&
                                (divisor = get_free_reg(type)) != -1) {
                            divmod_int_reg(divisor, loged_width);
                        } else {
                            char memstr_buff[800];
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            divmod_int_mem(memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op2;
                        unsigned int divisor = get_reg(type, -1, width, entry, ICONSTANT);
                        mov_int_const_reg(divisor, entry->val, loged_width);
                        divmod_int_reg(divisor, loged_width);
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
                /*
                 * Similar to div. we need to perform the shift operation in the 'c' register.
                 */
                unsigned int op2_in_c = 0;
                if (binop->op2_type != ICONSTANT)
                    op2_in_c = free_reg(2, binop->op2, type, loged_width);
                switch (binop->op1_type) {
                    case TEMPORARY: {
                        struct SymTab_entry* entry = binop->op1;
                        LOAD_TEMP_TO_RES(result_reg, entry);
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
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
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            mov_int_mem_reg(result_reg, memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = binop->op1;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        mov_int_const_reg(result_reg, entry->val, loged_width);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be uop1\n", binop->op1_type);
                        exit(-1);
                }
                if (op2_in_c) {
                    write_asm("%s %s, cl\n", int_arithmetic[binop->op_type], register_names[loged_width][result_reg]);
                    if (binop->op2_type == TEMPORARY) {
                        struct SymTab_entry* entry = binop->op2;
                        clear_all_locations(entry);
                        entry->reg_locs = 0;
                    }
                } else {
                    switch (binop->op2_type) {
                        case TEMPORARY: {
                            struct SymTab_entry* entry = binop->op2;
                            unsigned int temp_reg = first_reg(entry);
                            if (temp_reg != 2) {
                                mov_int_reg_reg(2, temp_reg, loged_width);
                                entry->reg_locs |= 1 << 2;
                            }
                            clear_all_locations(entry);
                            entry->reg_locs = 0;
                            print_registers();
                            shift_int_reg(binop->op_type, result_reg, loged_width);
                            break;
                        }
                        case VARIABLE: {
                            struct SymTab_entry* entry = binop->op2;
                            if (in_reg(entry->reg_locs)) {
                                if (!(used_later(binop->op2_info))) {
                                    unsigned int temp_reg;
                                    VAR_GET_CLEAR_STORE_COND(temp_reg, entry);
                                    if (temp_reg != 2)
                                        mov_int_reg_reg(2, temp_reg, loged_width);
                                    shift_int_reg(binop->op_type, result_reg, loged_width);
                                }  else {
                                    unsigned int temp_reg = first_reg(entry);
                                    if (temp_reg != 2)
                                        mov_int_reg_reg(2, temp_reg, loged_width);
                                    shift_int_reg(binop->op_type, result_reg, loged_width);
                                }
                            } else if (used_later(binop->op2_info)) {
                                unsigned int temp_reg = get_reg(type, -1, width, entry, VARIABLE);
                                load(entry, temp_reg, width, type);
                                if (temp_reg != 2)
                                    mov_int_reg_reg(2, temp_reg, loged_width);
                                shift_int_reg(binop->op_type, result_reg, loged_width);
                            } else {
                                char memstr_buff[800];
                                get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                                mov_int_mem_reg(2, memstr_buff, loged_width);
                                shift_int_reg(binop->op_type, result_reg, loged_width);
                            }
                            break;
                        }
                        case ICONSTANT: {
                            struct int_entry* entry = binop->op2;
                            shift_int_const(binop->op_type, result_reg, entry->val, loged_width);
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
            case VARIABLE:{
                struct SymTab_entry* entry = uop->operand;
                if (in_reg(entry->reg_locs)) {
                    if (!(used_later(uop->operand_info))) {
                        VAR_GET_CLEAR_STORE_COND(result_reg, entry);
                    } else if (count_registers(entry->reg_locs) >= 2) {
                        result_reg = least_reg(entry);
                    } else {
                        unsigned int temp_reg = least_reg(entry);
                        result_reg = get_reg(type, temp_reg, width, entry, VARIABLE);
                        char memstr_buff[800];
                        get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                        mov_float_mem_reg(result_reg, memstr_buff, loged_width);
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
    float_neg(result_reg, loged_width);
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
                                mov_int_reg_reg(result_reg, temp_reg, loged_width);
                            }
                        } else {
                            result_reg = get_reg(type, -1, width, entry, VARIABLE);
                            char memstr_buff[800];
                            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                            mov_int_mem_reg(result_reg, memstr_buff, loged_width);
                        }
                        break;
                    }
                    case ICONSTANT: {
                        struct int_entry* entry = uop->operand;
                        result_reg = get_reg(type, -1, width, entry, ICONSTANT);
                        mov_int_const_reg(result_reg, entry->val, loged_width);
                        break;
                    }
                    default:
                        fprintf(stderr, "Internal error:SymbolType %d shouldn't be uop1\n", uop->operand_type);
                        exit(-1);
                }
                break;
        }
        unary_int(result_reg, uop->operator_type, loged_width);
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
            int op_loged_width = log2(op_width);
            if (new_type) {
                if (op_type) {
                    result_reg = first_reg(entry);
                    clear_all_locations(entry);
                    conv_float_float_reg_reg(result_reg, result_reg, new_loged_width);
                } else {
                    int op_reg = first_reg(entry);
                    clear_all_locations(entry);
                    result_reg = get_reg(new_type, op_reg, new_width, entry, TEMPORARY);
                    conv_int_float_reg_reg(result_reg, op_reg, new_loged_width, op_loged_width);
                }
            } else {
                if (op_type) {
                    int op_reg = first_reg(entry);
                    clear_all_locations(entry);
                    result_reg = get_reg(op_type, op_reg, new_width, entry, TEMPORARY);
                    conv_float_int_reg_reg(result_reg, op_reg, new_loged_width, op_loged_width);
                } else {
                    result_reg = first_reg(entry);
                    clear_all_locations(entry);
                    conv_int_int_reg_reg(result_reg, result_reg, new_loged_width, op_loged_width);
                }
            }
            break;
        }
        case VARIABLE: {
            struct SymTab_entry* entry = conv->op;
            long op_width_and_type = entry->width_and_type;
            int op_type = op_width_and_type & 1;
            int op_width = op_width_and_type >> 2;
            int op_loged_width = log2(op_width);
            if (in_reg(entry->reg_locs)) {
                int op_reg;
                if (!(used_later(conv->op_info))) {
                    VAR_GET_CLEAR_STORE_COND(op_reg, entry);
                } else if (count_registers(entry->reg_locs) >= 2) {
                    op_reg = least_reg(entry);
                } else {
                    unsigned int temp_reg = least_reg(entry);
                    op_reg = get_reg(op_type, temp_reg, op_width, entry, VARIABLE);
                    if (op_type)
                        mov_float_reg_reg(op_reg, temp_reg, op_loged_width);
                    else
                        mov_int_reg_reg(op_reg, temp_reg, op_loged_width);
                    append_reg_to_symbol(entry, op_reg);
                }
                if (new_type) {
                    if (op_type) {
                        conv_float_float_reg_reg(op_reg, op_reg, new_loged_width);
                        result_reg = op_reg;
                        clear_reg(op_reg);
                    } else {
                        result_reg = get_reg(new_type, -1, new_width, entry, VARIABLE);
                        conv_int_float_reg_reg(result_reg, op_reg, new_loged_width, op_loged_width);
                    }
                } else {
                    if (op_type) {
                        result_reg = get_reg(new_type, -1, new_width, entry, VARIABLE);
                        conv_float_int_reg_reg(result_reg, op_reg, new_loged_width, op_loged_width);
                    } else {
                        conv_int_int_reg_reg(op_reg, op_reg, new_loged_width, op_loged_width);
                        result_reg = op_reg;
                        clear_reg(op_reg);
                    }
                }
            } else {
                result_reg = get_reg(new_type, -1, new_width, entry, VARIABLE);
                char memstr_buff[800];
                get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                if (new_type)
                    if (op_type)
                        conv_float_float_mem_reg(result_reg, memstr_buff, new_loged_width, op_loged_width);
                    else
                        conv_int_float_mem_reg(result_reg, memstr_buff, new_loged_width, op_loged_width);
                else
                    if (op_type)
                        conv_float_int_mem_reg(result_reg, memstr_buff, new_loged_width, op_loged_width);
                    else
                        conv_int_int_mem_reg(result_reg, memstr_buff, new_loged_width, op_loged_width);
            }
            break;
        }
        case FCONSTANT: {
            struct float_entry* entry = conv->op;
            result_reg  = get_reg(new_type, -1, new_width, entry, FCONSTANT);
            if (new_type) {
                conv_float_float_const_reg(result_reg, entry->offset);
            } else {
                conv_float_int_const_reg(result_reg, entry->offset, new_loged_width);
            }
            break;
        }
        case ICONSTANT: {
            struct int_entry* entry = conv->op;
            result_reg = get_reg(new_type, -1, new_width, entry, ICONSTANT);
            if (new_type) {
                unsigned int op_reg = get_reg(0, -1, new_width, entry, ICONSTANT);
                mov_int_const_reg(op_reg, entry->val, 3);
                conv_int_float_reg_reg(result_reg, op_reg, new_loged_width, 3);
            } else {
                mov_int_const_reg(result_reg, entry->val, new_loged_width);
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

#define VAR_FLOAT_OP1_REG(var, info, reg_name, width, loged_width, type)\
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
                mov_float_reg_reg(reg_name, temp_reg, loged_width);\
            }\
        } else {\
            reg_name = get_reg(type, -1, width, entry, VARIABLE);\
            char memstr_buff[800];\
            get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);\
            mov_float_mem_reg(reg_name, memstr_buff, loged_width);\
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
                VAR_FLOAT_OP1_REG(cond->op1, cond->op1_info, op1_reg, width, loged_width, type);
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
                    float_control_reg_reg(op1_reg, temp_reg, loged_width);
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
                        mov_float_reg_reg(op2_reg, temp_reg, loged_width);
                    }
                    float_control_reg_reg(op1_reg, op2_reg, loged_width);
                } else if (used_later(cond->op2_info)) {
                    unsigned int op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    mov_float_mem_reg(op2_reg, memstr_buff, loged_width);
                    float_control_reg_reg(op1_reg, op2_reg, loged_width);
                } else {
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    float_control_mem_reg(op1_reg, memstr_buff, loged_width);
                }
                break;
            }

            case FCONSTANT: {
                struct float_entry* entry = cond->op2;
                float_control_const_reg(op1_reg, entry->offset, loged_width);
                break;
            }
            default:
                fprintf(stderr, "Internal error:Did not expect SymbolType %d in float op2 switch\n",
                        cond->op2_type);
                exit(-1);
        }
        cond_jump(cond->op_type, true, 1);
        uncond_jump(false);
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
                        mov_int_reg_reg(op1_reg, temp_reg, loged_width);
                    }
                } else {
                    op1_reg = get_reg(type, -1, width, entry, VARIABLE);
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    mov_int_mem_reg(op1_reg, memstr_buff, loged_width);
                }
                break;
            }

            case ICONSTANT: {
                struct int_entry* entry = cond->op1;
                op1_reg = get_reg(type, -1, width, entry, ICONSTANT);
                mov_int_const_reg(op1_reg, entry->val, loged_width);
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
                int_control_reg_reg(op1_reg, temp_reg, loged_width);
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
                        mov_int_reg_reg(op1_reg, temp_reg, loged_width);
                    }
                    int_control_reg_reg(op1_reg, op2_reg, loged_width);
                } else if (used_later(cond->op2_info)) {
                    unsigned int op2_reg = get_reg(type, op1_reg, width, entry, VARIABLE);
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    mov_int_mem_reg(op2_reg, memstr_buff, loged_width);
                    entry->reg_locs |= 1 << op2_reg;
                    int_control_reg_reg(op1_reg, op2_reg, loged_width);
                } else {
                    char memstr_buff[800];
                    get_memstr(&memstr_buff[0], entry->mem_loc & 1, entry);
                    int_control_mem_reg(op1_reg, memstr_buff, loged_width);
                }
                break;
            }

            case ICONSTANT: {
                struct int_entry* entry = cond->op2;
                int_control_const_reg(op1_reg, entry->val, loged_width);
                break;
            }

            default:
            fprintf(stderr, "Internal error:Did not expect SymbolType %d in int op2 switch\n",
                    cond->op2_type);
                exit(-1);
        }
    }
    cond_jump(cond->op_type, true, 1);
    uncond_jump(false);
}

void emit_uncond(struct BasicBlock* target)
{
    uncond_jump(target);
}

/*
 * Params still need to be fully implemented to align with calling conventions.
 * Otherwise we can't interface with libs in other libraries, i.e. we are toast.
 */
void emit_param(struct ParamQuad* param)
{
    switch (param->type) {
        case TEMPORARY: {
            struct SymTab_entry* entry = param->op;
            long w_n_t = entry->width_and_type;
            int type = w_n_t & 1;
            int width = w_n_t >> 2;
            unsigned int loged_width = log2(width);
            unsigned int p_n = global_param_counter[type]++;
            unsigned int param_in_reg = 0;
            if (type && p_n < 7 )
                param_in_reg = free_reg(p_n, param->op, type, loged_width);
            else if (p_n < 5)
                ;
                //param_in_reg = free_reg(int_arg_regs[p_n], param->op, type, loged_width);
            else
                ;
                //push_on_stack(param->op, type, loged_width);
            if (!param_in_reg) {
                clear_all_locations(param->op);
                entry->reg_locs = 0;
                return;
            }
        }
        case VARIABLE:
        case FCONSTANT:
        case ICONSTANT:
                        ;
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
