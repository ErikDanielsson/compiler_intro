#pragma once
#include <stdio.h>
void fprintf_w_indent(FILE* stream, int indent, const char* fstring, ...);
void print_w_indent(int indent, const char* fstring, ...);
void print_bin(unsigned long number, int n_bits);
