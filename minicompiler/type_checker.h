#pragma once
#include "parser.h"
extern struct SymTab* basic_types;
void init_type_checker();
void enter_struct_decl(struct StructDecl* node);
void push_Symtab();
void check_vardecl(struct VarDecl* node);
void check_func_decl(struct FuncDecl* node);
