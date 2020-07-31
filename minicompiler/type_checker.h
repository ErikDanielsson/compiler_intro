#pragma once
#include "parser.h"
extern struct SymTab* type_table;
void init_type_checker();
void nested_function_error(struct FuncDecl* node);
void return_not_in_func_error();
char* widen(char* addr_a, char* type1, char* type2);
char* max(char* type1, char* type2);
void push_Env();
void pop_Env();
struct SymTab* pop_Env_struct();
void enter_type_def(char* type_name, struct SymTab* struct_env);
void check_type_defined(char* type_name);
void check_and_set_var(struct VarDecl* node);
void check_and_set_func(struct FuncDecl* node);
char* check_var_declared(struct VarAcc* varacc);
