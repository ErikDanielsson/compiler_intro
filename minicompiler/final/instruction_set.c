
/*
 * This file help the codegen translate IC to x86 assembly. 
 * All register and instruction names are kept in this file.
 */
const char* register_names[5][16] = {
    {
    // General purpose registers (8 bit section)
        "al", "bl", "cl", "dl"
        "spl", "bpl", "sil", "dil",
        "r8b",  "r9b",  "r10b", "r11b",
        "r12b", "r13b", "r14b", "r15b",
    },
    {
    // General purpose registers (16 bit section)
        "ax", "bx", "cx", "dx",
        "sp", "bp", "si", "di",
        "r8w",  "r9w",  "r10w", "r11w",
        "r12w", "r13w", "r14w", "r15w",
    },
    {
    // General purpose registers (32 bit section)
        "eax", "ebx", "ecx", "edx",
        "esp", "ebp", "esi", "edi",
        "r8d",  "r9d",  "r10d", "r11d",
        "r12d", "r13d", "r14d", "r15d",
    },
    {
    // General purpose registers (64 bit section)
        "rax", "rbx", "rcx", "rdx",
        "rsp", "rbp", "rsi", "rdi",
        "r8",  "r9",  "r10", "r11",
        "r12", "r13", "r14", "r15",
    },
    {
        // Float registers (64 and 32 bit (really SIMD...but that's a different story))
        "xmm0", "xmm1", "xmm2", "xmm3",
        "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10","xmm11",
        "xmm12","xmm13","xmm14","xmm15"
    }
};

/*
 * The first 6 integer arguments to a function is placed in the following
 * registers. If the function takes more arguments they are placed on the stack.
 * Floating points arguments are placed in xmm0...7, and then on the stack.
 */
unsigned char int_arg_regs = {7, 6, 3, 2, 8, 9};

const char* size_spec[4] = {
    "byte",
    "word",
    "dword",
    "qword"
};

// Instruction set: A small subset of the entire x86-64 instruction set.
const char* mov[] = {
    "mov",      // <dest>, <src>
    "lea",      // <reg64>, <mem>   Place address of <mem> in <reg64>
    "movss",    // <RXdest>, <src>  Place <src> (32 bit) in <RXdest>
    "movsd"     // <RXdest>, <src>  Place <src> (64 bit) in <RXdest>
};

const char* widen_to_ad[] = {
    "cbw",      // byte@al -> word@ax
    "cwd",      // word@ax -> dword@ax:dx
    "cdq",      // dword@eax -> qword@eax:edx
    "cqo"       // dword@rax -> qword@rax:rdx
};

const char* conv_in_a[] = {
    "cbw",      // byte@al -> word@ax
    "cwde",     // word@ax -> dword@eax
    "cdqe"      // dword@eax -> qword@rax
};

const char* conv_signed[] = {
    "movsx",        // <reg16-64>, <op8-16>     Convert to size of register
    "movsxd"         // <reg64>, <op32>
};

const char* conv_float[] = {
    "cvtss2sd",     // float -> double
    "cvtsd2ss"      // double -> float
};

const char* conv_float_int[] = {
    "cvtss2si",     // float -> int
    "cvtsd2si",     // double -> int
    "cvtsi2ss",     // int -> float
    "cvtsi2sd"      // int -> double
};



const char* int_unary[] = {
    "neg",
    "not"
};
const char* int_arithmetic[] = {
    "add",      // <dest>, <src>
    "sub",      // <dest>, <src>
    "imul",     // <dest>, <src>    Mul <dest> with <src> and place in <dest>
                // <src>            Mul 'a' register with <src>, result in a:d
    "idiv",     // <op>             Div a:d by <op> -- res in 'a', rem in 'd'
    "idiv",     // <op>             Div a:d by <op> -- res in 'a', rem in 'd'
    "and",      // <dest>, <src>
    "or",       // <dest>, <src>
    "xor",      // <dest>, <src>
    "sar",      // <dest>, <imm>    Max of cl and <imm> is 64.
                // <dest>, cl
    "sal"       // <dest>, <imm>    Max of cl and <imm> is 64.
                // <dest>, cl
};
const char* inc_dec[] = {
    "inc",      // <op>
    "dec"       // <op>
};

const char* float_arithmetic[2][4] = {
    {
        "addss",    //
        "subss",
        "mulss",
        "divss"
    },
    {
        "addsd",
        "subsd",
        "mulsd",
        "divsd"
    }
};

const char* int_control[] = {
    "cmp",       // <op1>, <op2>. Compare op1 and op2, result in rFlags
    "test"      // <op1>, <op2>. 'and' op1 and op2, result in rFlags.
                // Will only be used for zero comparisions.
};

const char* float_control[] = {
    "ucomiss",  // <op32>, <op32>.  Compare op1 and op2,
                // result in rFlags
    "ucomisd"   // <op64>, <op64>.  Compare op1 and op2,
                // result in rFlags
};

const char* cond_jumps[2][6] = {
    {
        "je",       // Based on flags set by comparision: <op1> == <op2>
        "jne",       // Based on flags set by comparision: <op1> != <op2>
        // For signed ints:
        "jl",       // Based on flags set by comparision: <op1> < <op2>
        "jle",      // Based on flags set by comparision: <op1> <= <op2>
        "jge",      // Based on flags set by comparision: <op1> > <op2>
        "jg",       // Based on flags set by comparision: <op1> >= <op2>
    },
    {
        "je",       // Based on flags set by comparision: <op1> == <op2>
        "jne",       // Based on flags set by comparision: <op1> != <op2>
        // For unsigned ints and floats:
        "jb",       // Based on flags set by comparision: <op1> <  <op2>
        "jbe",      // Based on flags set by comparision: <op1> <=  <op2>
        "ja",       // Based on flags set by comparision: <op1> > <op2>
        "jae",      // Based on flags set by comparision: <op1> >= <op2>
    }
};

const char* stack[] = {
    "push"      // <op64>. Push <op64> onto stack and adjust rsp.
    "pop"       // <op64>. Pop <op64> from stack and adjust rsp.
};

const char* function[] = {
    "call"      // <funcname>. Push rip and jump to <funcname>
    "ret"       // Pop rip.
};
