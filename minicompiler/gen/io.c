#include <stdio.h>
#include <stdarg.h>
#define INDENT 4
void fprintf_w_indent(FILE* stream, int indent, const char* fstring, ...)
{
    for (int i = 0; i < indent; i++)
        fprintf(stream, "%*c", INDENT, ' ');
    va_list args;
    va_start(args, fstring);
    vfprintf(stream, fstring, args);
    va_end(args);
}
void print_w_indent(int indent, const char* fstring, ...)
{
    for (int i = 0; i < indent; i++)
        printf("%*c", INDENT, ' ');
    va_list args;
    va_start(args, fstring);
    vprintf(fstring, args);
    va_end(args);
}

void print_bin(unsigned long number, int n_bits)
{
    for (int i = 0; i < n_bits; i++)
        printf("%u", 0 != (number & (1 << (n_bits-1-i))));
    printf("\n");
}
