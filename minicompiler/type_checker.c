#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type_checker.h"
#include "symbol_table.h"

#define TABLESIZE 128

/*
 *  The type checker enters type names during parsing, and checks whether they
 *  are used correctly during treewalk of IC generation
 */


void type_error(char* string, char* string2,int fatal )
{
    fprintf(stderr, string, string2);
    if (fatal)
        exit(-1);
}

void nesting_error()
{
    fprintf(stderr, "Nesting exceeded limit of %d\n", NESTINGDEPTH);
    exit(-1);
}

void undeclared_var_error(struct VarAcc* node)
{
    fprintf(stderr, "\033[1;31merror\033[0m: Use of undeclared variable '%s' at %d:%d\n",
            node->variable->lexeme,node->variable->line, node->variable->column);
    exit(-1);
}

void undeclared_func_error(struct FuncCall* node)
{
    fprintf(stderr, "\033[1;31merror\033[0m: Use of undeclared func '%s' at %d:%d\n",
            node->func->lexeme,node->func->line, node->func->column);
    exit(-1);
}

void mismatching_params_error(struct FuncCall* funccall, struct FuncDecl* funcdecl)
{
    fprintf(stderr, "\033[1;31merror\033[0m: Call of function '%s' at %d:%d declared at line %d\nexpected %d params got %d\n",
            funccall->func->lexeme, funccall->func->line, funccall->func->column,
            funcdecl->name->line, funcdecl->n_params, funccall->n_args);
    exit(-1);
}

void nested_function_error(struct FuncDecl* node)
{
    fprintf(stderr, "\033[1;31merror\033[0m: nested function declaration at line %d\n",
            node->name->line);
    exit(-1);
}

void return_not_in_func_error()
{
    fprintf(stderr, "\033[1;31merror\033[0m:Return statement not in function ... don't know which line. Figure it out.\n");
    exit(-1);
}






struct SymTab* type_table;

struct SymTab* symbol_table_stack[NESTINGDEPTH];
struct SymTab** top_symtab = symbol_table_stack;


void init_type_checker()
{
    type_table = create_SymTab(4, NULL);
    SymTab_set_type(type_table, "inontot", 0);
    SymTab_set_type(type_table, "lolonongog", 1);
    SymTab_set_type(type_table, "fofloloatot", 2);
    SymTab_set_type(type_table, "dodouboblole", 3);
    SymTab_set_type(type_table, "sostotrorinongog", -1);
    SymTab_set_type(type_table, "vovoidod", -1);

    *top_symtab = create_SymTab(TABLESIZE, NULL);
}



int offset_stack[NESTINGDEPTH];
int* offset = offset_stack;

void push_Env()
{
    if (top_symtab-symbol_table_stack == NESTINGDEPTH-1)
        nesting_error();
    top_symtab++;
    *top_symtab = create_SymTab(TABLESIZE, *(top_symtab-1));
    offset++;
    (*offset) = 0;
}

void pop_Env()
{
    /*
     * Set parent pointer to NULL to avoid dangling pointer errors
     */
    SymTab_dump(*top_symtab, "leaving");
    (*top_symtab)->parent = NULL;
    top_symtab--;
    SymTab_append_child(*top_symtab, *(top_symtab+1));
    offset--;
    SymTab_dump(*top_symtab, "entering");
}

struct SymTab* pop_Env_struct()
{
    /*
     * Set parent pointer to NULL to avoid dangling pointer errors
     */
    (*top_symtab)->parent = NULL;
    top_symtab--;
    offset--;
    return *(top_symtab+1);
}

void enter_type_def(char* type_name, struct SymTab* struct_env)
{
    if (SymTab_type_declared(type_table, type_name) || SymTab_check_and_set(symbol_table_stack[0], type_name, STRUCTURE, struct_env))
        type_error("Redefinition of type '%s'\n", type_name, TRUE);
}

void check_type_defined(char* type_name)
{
    /*
     * Check if type is builtin or defined in outermost scope.
     */
    if (!(SymTab_type_declared(type_table, type_name) ||
        SymTab_type_declared(symbol_table_stack[0], type_name)))
        type_error("type '%s' is not declared", type_name, TRUE);
}

void check_and_set_var(struct VarDecl* node)
{
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, VARIABLE, node))
        type_error("Redefinition of variable '%s'\n", node->name->lexeme, TRUE);
}

void check_and_set_func(struct FuncDecl* node)
{
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, FUNCTION, node))
        type_error("Redefinition of function '%s'\n", node->name->lexeme, TRUE);
}

char* check_var_declared(struct VarAcc* varacc)
{
    struct VarDecl* node = SymTab_getr(*top_symtab, varacc->variable->lexeme, VARIABLE);
    if (node == NULL)
        undeclared_var_error(varacc);
    return node->type->lexeme;
}

struct FuncDecl* check_func_declared(struct FuncCall* funccall)
{
    struct FuncDecl* node = SymTab_getr(*top_symtab, funccall->func->lexeme, FUNCTION);
    if (node == NULL)
        undeclared_func_error(funccall);
    return node;
}
