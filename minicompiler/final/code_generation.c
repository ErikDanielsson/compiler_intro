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


const char* int_registers[] = {
    "rax", "rcx", "rdx", "rbx",
    "rsp", "rbp", "rsi", "rdi",
    "r8",  "r9",  "r10", "r11",
    "r12", "r13", "r14", "r15"
};

const char* float_registers[] = {
    "xmm0", "xmm1", "xmm2", "xmm3",
    "xmm4", "xmm5", "xmm6", "xmm7",
    "xmm8", "xmm9", "xmm10","xmm11",
    "xmm12","xmm13","xmm14","xmm15"
};

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



void write_main(struct IC_entry* main)
{
    write("global main\n");
    write("main:");
    write("\n\n");
}

void write_func(struct IC_entry* func)
{
    write("global %s\n", func->key);
    write("%s:", func->key);
    write("\n\n");
}

// Instruction set: A small subset of the entire x86-64 instruction set.

char* mov[] = {
    "mov",      // <dest>, <src>
    "lea",      // <reg64>, <mem>   Place address of <mem> in <reg64>
    "movss",    // <RXdest>, <src>  Place <src> (32 bit) in <RXdest>
    "movsd"     // <RXdest>, <src>  Place <src> (64 bit) in <RXdest>
};

char* conv_in_a[] = {
    "cbw",      // byte@al -> word@ax
    "cwd",      // word@ax -> dword@ax:dx
    "cwde",     // word@ax -> dword@eax
    "cdq",      // dword@eax -> qword@eax:edx
    "cdqe",     // dword@eax -> qword@rax
    "cqo"       // dword@rax -> qword@rax:rdx
};

char* conv_signed[] = {
    "movsx",        // <reg16-64>, <op8-16>     Convert to size of register
    "movsx"         // <reg64>, <op32>
};

char* conv_float[] = {
    "cvtss2sd",     // float -> double
    "cvtss2ss"      // double -> float
};

char* conv_float_int[] = {
    "cvtss2si",     // float -> int
    "cvtsd2si",     // double -> int
    "cvtsi2ss",     // int -> float
    "cvtsi2sd"      // int -> double
};

char* int_arithmetic[] = {
    "add",      // <dest>, <src>
    "sub",      // <dest>, <src>
    "imul",     // <dest>, <src>    Mul <dest> with <src> and place in <dest>
                // <src>            Mul 'a' register with <src>, result in a:d
    "idiv",     // <op>             Div a:d by <op> -- res in 'a', rem in 'd'
    "and",      // <dest>, <src>
    "or",       // <dest>, <src>
    "xor",      // <dest>, <src>
    "not",      // <dest>, <src>
    "sar",      // <dest>, <imm>    Max of cl and <imm> is 64.
                // <dest>, cl
    "sal",      // <dest>, <imm>    Max of cl and <imm> is 64.
                // <dest>, cl
    "inc",      // <op>
    "dec"       // <op>
};

char* float_arithmetic[] = {
    "addss",    //
    "addsd",
    "subss",
    "subsd",
    "mulss",
    "mulsd",
    "divss",
    "divsd",
};

char* int_control[] = {
    "cmp"       // <op1>, <op2>. Compare op1 and op2, result in rFlags
    "test"      // <op1>, <op2>. 'and' op1 and op2, result in rFlags.
                // Will only be used for zero comparisions.
};

char* float_control[] = {
    "ucomiss",  // <op32>, <op32>.  Compare op1 and Compare op1 and op2,
                // result in rFlags
    "ucomisd"   // <op64>, <op64>.  Compare op1 and Compare op1 and op2,
                // result in rFlags
};

char* cond_jumps[] = {
    "je",       // Based on flags set by comparision: <op1> == <op2>
    "jne"       // Based on flags set by comparision: <op1> != <op2>

    // For signed ints:
    "jl",       // Based on flags set by comparision: <op1> < <op2>
    "jle",      // Based on flags set by comparision: <op1> <= <op2>
    "jge",      // Based on flags set by comparision: <op1> > <op2>
    "jg",       // Based on flags set by comparision: <op1> >= <op2>

    // For unsigned ints and floats:
    "jb",       // Based on flags set by comparision: <op1> <  <op2>
    "jbe",      // Based on flags set by comparision: <op1> <=  <op2>
    "ja",       // Based on flags set by comparision: <op1> > <op2>
    "jae",      // Based on flags set by comparision: <op1> >= <op2>

};

char* stack[] = {
    "push"      // <op64>. Push <op64> onto stack and adjust rsp.
    "pop"       // <op64>. Pop <op64> from stack and adjust rsp.
};

char* function[] = {
    "call"      // <funcname>. Push rip and jump to <funcname>
    "ret"       // Pop rip.
};
