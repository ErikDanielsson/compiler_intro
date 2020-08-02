#pragma once
#include "parser.h"
extern struct SymTab* type_table;
void type_error(int fatal, char* string, ...);
void init_type_checker();
void nested_function_error(struct FuncDecl* node);
void return_not_in_func_error();
void push_Env();
void pop_Env();
struct SymTab* pop_Env_struct();
void print_Env_tree();

void enter_type_def(char* type_name, struct SymTab* struct_env);
void check_type_defined(char* type_name);
void check_and_set_var(struct VarDecl* node);
void check_and_set_func(struct FuncDecl* node);
char* check_var_declared(struct VarAcc* varacc);
struct FuncDecl* check_func_declared(struct FuncCall* funccall);
void mismatching_params_error(struct FuncCall* funccall, struct FuncDecl* funcdecl);
