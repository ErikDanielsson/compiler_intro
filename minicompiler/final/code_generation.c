#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "symbol_table.h"
#include "intermediate_code.h"
#include "IC_table.h"
#include "code_generation.h"
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

const char* int_registers[] = {"rax", "rcx", "rdx", "rbx",
                               "rsp", "rbp", "rsi", "rdi",
                            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"};

const char* float_registers[] = {"xmm0", "xmm1", "xmm2", "xmm3",
                                 "xmm4", "xmm5", "xmm6", "xmm7",
                                 "xmm8", "xmm9", "xmm10", "xmm11",
                                "xmm12", "xmm13", "xmm14", "xmm15"};
void alloc_statics(struct SymTab* main_symbol_table);
void codegen()
{
    struct IC_entry* main = IC_table_get_entry(intermediate_code, "main");

    alloc_statics(main->symbol_table);
}

void allocr(struct SymTab* symbol_table);
void alloc_statics(struct SymTab* main_symbol_table)
{
    fprintf(asm_file_desc, "section .data\n");
    allocr(main_symbol_table);

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
            fprintf(asm_file_desc, "\t%s%d %s 0\n", entry->key, entry->counter_value,
            declaration_widths[log2(entry->width)]);
        }
    }
}
void enter_data()
{

}
void enter_text()
{
    fprintf(asm_file_desc, "section .text\n");
}
