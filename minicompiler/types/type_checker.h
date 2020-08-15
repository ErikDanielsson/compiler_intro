#pragma once
#include "parser.h"
#include "constant_table.h"

extern struct TypeTab* type_table;
void type_error(int fatal, char* string, ...);
void init_type_checker();
void nested_function_error(struct FuncDecl* node);
void return_not_in_func_error();
void mismatching_params_error(struct FuncCall* funccall, struct FuncDecl* funcdecl);

void push_Env();
void pop_Env();
struct SymTab* pop_Env_func();
struct SymTab* pop_Env_struct();
void print_Env_tree();
void destroy_Env_tree();
struct SymTab* get_main_SymTab();
void enter_type_def(char* type_name, struct SymTab* struct_env);
void check_type_defined(char* type_name);
enum SymbolType get_var_type(char* var_name);
void enter_temp_var(char* temp_name, char* type);
void check_and_set_var(struct VarDecl* node);
void check_and_set_func(struct FuncDecl* node);
char* check_var_declared(struct VarAcc* varacc);
struct FuncDecl* check_func_declared(struct FuncCall* funccall);
struct SymTab_entry* get_curr_name_entry(char* name);
unsigned long get_type_info(char* type);
void check_binop_and_types(enum TokenType binop_type, char* type1, char* type2);

extern struct ConstTab* int_table;
extern struct ConstTab* float_table;
extern struct ConstTab* string_table;

struct int_entry* enter_int(long val);
struct float_entry* enter_float(float val);
struct string_entry* enter_string(char* val);
