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

/*
 * The code generator will generate Intel syntax assembly for the x86-64
 * architecture. Should rewrite it later to generate an ".o" file directly
 */

FILE* asm_file_desc;
void codegen();
void generate_assembly(const char* basename)
{
    char filename[strlen(basename)+5];
    sprintf(filename, "%s.s", basename);
    asm_file_desc = fopen(filename, "w");
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
    write("\tfofloloatot dq ");
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
     * We only need to emit the float consts, since ints can
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
            declaration_widths[log2(entry->width)]);
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
#define CREATE_INT_SET(set) \
    set.size = INT_SET_SIZE;\
    set.entries = malloc(sizeof(struct int_set_entry*)*INT_SET_SIZE);\
    for (int i = 0; i < INT_SET_SIZE; i++) \
        set.entries[i] = NULL;\

struct reg_desc registers[32] = { 0 };
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
    //for (; curr->next != NULL)
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
    switch (assign->rval_type) {
        case VARIABLE:
        case TEMPORARY:
        case FCONSTANT: {
            int r_val_reg  = get_reg(assign->rval, assign->rval_type);

            if (strcmp(((struct VarDecl*)(assign->lval->symbol))->type->lexeme, "fofloloatot") == 0)
                if (assign->lval->width == 4)
                    write("movss [%s%u], %s\n", assign->lval->key, assign->lval->counter_value,
                            register_names[r_val_reg]);
                else
                    write("movsd [%s%u], %s\n", assign->lval->key, assign->lval->counter_value,
                            register_names[r_val_reg]);
            else
                write("mov [%s%u], %s\n", assign->lval->key, assign->lval->counter_value,
                        register_names[r_val_reg]);

            return;
        }
        case ICONSTANT:
            write("mov [%s%u], %ld", assign->lval->key, assign->lval->counter_value,
                    ((struct int_entry*)(assign->rval))->val);
            return;
        default:
            return;
    }


}
static inline void emit_binop(struct BinOpQuad* binop)
{
    return;
}
static inline void emit_uop(struct UOpQuad* assign)
{
    return;
}
static inline void emit_conv(struct ConvQuad* binop)
{
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

void load_var(int reg, struct SymTab_entry* var, int type);
void store(struct SymTab_entry* var, int reg);
void load_float(int reg, long float_loc);
int get_reg(void* symbol, enum SymbolType type)
{
    /*
     * Register selection for operand. Looking at this code, overwriting of
     * destination operand would be a problem, but this is avoided by setting
     * that register to reserved.
     */
    switch (type) {
        case VARIABLE: {
            struct SymTab_entry* var = symbol;
            if (var->reg_loc == -1) {
                int type = strcmp(((struct VarDecl*)(var->symbol))->type->lexeme, "fofloloatot") == 0;
                int lreg = 16 * type;
                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].value == NULL) {
                        load_var(lreg+i, var, type);
                        registers[lreg+i].type = REG_VARIABLE;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].type == REG_CONSTANT) {
                        load_var(lreg+i, var, type);
                        registers[lreg+i].type = REG_VARIABLE;
                        registers[lreg+i].value = symbol;
                        return lreg+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[lreg+i].type == REG_VARIABLE) {
                        store(registers[lreg+i].value, lreg+i);
                        load_var(lreg+i, var, type);
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
            if (var->reg_loc == -1)
                temp_not_computed_error(var);
            return var->reg_loc;
        }

        case FCONSTANT: {
            struct float_entry* constant = symbol;
            if (constant->reg_loc == -1) {
                for (int i = 0; i < 16; i++) {
                    if (registers[16+i].value == NULL) {
                        load_float(16+i, constant->offset);
                        registers[16+i].type = REG_CONSTANT;
                        registers[16+i].value = symbol;
                        return 16+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[16+i].type == REG_CONSTANT) {
                        load_float(16+i, constant->offset);
                        registers[16+i].type = REG_CONSTANT;
                        registers[16+i].value = symbol;
                        return 16+i;
                    }
                }
                for (int i = 0; i < 16; i++) {
                    if (registers[16+i].type == REG_VARIABLE) {
                        store(registers[16+i].value, 16+i);
                        load_float(16+i, constant->offset);
                        registers[16+i].type = REG_VARIABLE;
                        registers[16+i].value = symbol;
                        return 16+i;
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

void load_var(int reg, struct SymTab_entry* var, int type)
{
    printf("load %s\n", var->key);
    if (type)
        if (var->width == 4)
            write("movss %s, [%s%u]\n", register_names[reg], var->key, var->counter_value);
        else
            write("movsd %s, [%s%u]\n", register_names[reg], var->key, var->counter_value);
    else
        write("mov %s, [%s%u]\n", register_names[reg], var->key, var->counter_value);
}

void store(struct SymTab_entry* var, int reg)
{
    return;
}

void load_float(int reg, long float_loc)
{
    return;
}
