#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "type_checker.h"
#include "symbol_table.h"
#include "type_table.h"
#include "constant_table.h"

#define TABLESIZE 127
#define VERBOSE 0
/*
 *  The type checker enters type names during parsing, and checks whether they
 *  are used correctly during treewalk of IC generation
 */


/*
 * Functions for handling errors that might be found during type checking.
 */
void type_error(int fatal, char* string, ...)
{
    va_list args;
    fprintf(stderr, "\033[1;31merror\033[0m:");
    va_start(args, string);
    vfprintf(stderr, string, args);
    va_end(args);
    fprintf(stderr, "\n");
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
    fprintf(stderr, "\033[1;31merror\033[0m:Use of undeclared func '%s' at %d:%d\n",
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



struct TypeTab* type_table;

/*
 * Checking and setting symbols into symbols tables. During typechecking,
 * the symbol tables are linked "child->parent" for easy access to parents
 * symbols. However, when the type checking of a scope is finished the symbol
 * table are relinked "parent->childs" to facilitate top down traversal
 * for later stages of the compiler (or at least cleanup).
 */

 struct SymTab* symbol_table_stack[NESTINGDEPTH];
 struct SymTab** top_symtab = symbol_table_stack;

long offset_stack[NESTINGDEPTH];
long* offset = offset_stack;

struct SymTab* get_main_SymTab()
{
    return symbol_table_stack[0];
}

void push_Env(char* name)
{
    if (top_symtab-symbol_table_stack == NESTINGDEPTH-1)
        nesting_error();
    top_symtab++;
    *top_symtab = create_SymTab(TABLESIZE, *(top_symtab-1), name);
    offset++;
    (*offset) = 0;
}

void pop_Env()
{
    /*
     * Set parent pointer to NULL to avoid dangling pointer errors
     */
    #if VERBOSE
    SymTab_dump(*top_symtab, "leaving");
    #endif
    (*top_symtab)->parent = NULL;
    SymTab_append_child(*(top_symtab-1), *top_symtab);
    top_symtab--;

    offset--;
    #if VERBOSE
    SymTab_dump(*top_symtab, "entering");
    #endif
}

struct SymTab* pop_Env_func()
{
    /*
     * Set parent pointer to NULL to avoid dangling pointer errors
     */
    #if VERBOSE
    SymTab_dump(*top_symtab, "leaving");
    #endif
    (*top_symtab)->parent = NULL;
    top_symtab--;

    offset--;
    #if VERBOSE
    SymTab_dump(*top_symtab, "entering");
    #endif
    return *(top_symtab+1);
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


void print_Env_tree_helper(struct SymTab* env, int indent)
{
    SymTab_dump(env, indent);
    for (int i = 0; i < env->n_childs; i++)
        print_Env_tree_helper(env->childs[i], indent+1);
}

void print_Env_tree()
{
    printf("Symbol tables for scopes\n");
    struct SymTab* env = symbol_table_stack[0];
    SymTab_dump(env, 0);
    for (int i = 0; i < env->n_childs; i++)
        print_Env_tree_helper(env->childs[i], 1);
}

void destroy_Env_tree()
{
    SymTab_destroyr(symbol_table_stack[0]);
}



void enter_type_def(char* type_name, struct SymTab* struct_env)
{
    if (TypeTab_check_and_set(type_table, type_name, struct_env))
        type_error(TRUE, "Redefinition of type '%s'\n", type_name);
}


void enter_temp_var(char* temp_name, char* type)
{
    int width_and_type = get_type_and_width(type_table, type);
    SymTab_check_and_set(*top_symtab, temp_name, TEMPORARY, NULL, 0, width_and_type);
}

void check_type_defined(char* type_name)
{
    if (!TypeTab_check_defined(type_table, type_name))
        type_error(TRUE, "type '%s' is not declared", type_name);
}


void check_and_set_var(struct VarDecl* node)
{
    unsigned long width_and_type = get_type_and_width(type_table, node->type->lexeme);
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, VARIABLE, node, *offset, width_and_type))
        type_error(TRUE, "Redefinition of variable '%s' at %d:%d\n", node->name->lexeme,
                    node->name->line, node->name->column);
    *offset += (width_and_type >> 2);
}

void check_and_set_func(struct FuncDecl* node)
{
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, FUNCTION, node, 0, 0))
        type_error(TRUE, "Redefinition of function '%s' at %d:%d\n",
                    node->name->lexeme,
                    node->name->line,
                    node->name->column);
}

char* check_var_declared(struct VarAcc* varacc)
{
    struct SymTab_entry* entry = SymTab_getr(*top_symtab, varacc->variable->lexeme, VARIABLE);
    if (entry == NULL)
        undeclared_var_error(varacc);
    struct VarDecl* node = entry->symbol;


    return node->type->lexeme;
}

struct FuncDecl* check_func_declared(struct FuncCall* funccall)
{
    struct SymTab_entry* entry = SymTab_getr(*top_symtab, funccall->func->lexeme, FUNCTION);
    if (entry == NULL)
        undeclared_func_error(funccall);
    struct FuncDecl* node = entry->symbol;
    return node;
}

struct SymTab_entry* get_curr_name_entry(char* name)
{
    struct SymTab_entry* entry = SymTab_getr(*top_symtab, name, VARIABLE);
    if (entry != NULL)
        return entry;
    return SymTab_getr(*top_symtab, name, TEMPORARY);
}

unsigned long get_type_info(char* type)
{
    return get_type_and_width(type_table, type);
}
void int_operator_on_float_error(enum TokenType op)
{
    char op_string[3];
    switch (op) {
        case '~':
        case '%':
        case '^':
        case '&':
        case '|':
            sprintf(op_string, "%c", op);
            break;
        case SHR:
            sprintf(op_string, ">>");
            break;
        case SHL:
            sprintf(op_string, "<<");
            break;
    }
    fprintf(stderr, "\033[1;31merror\033[0m:Operator '%s' cannot be applied on float operands\n", op_string);
    exit(-1);
}

void check_binop_and_types(enum TokenType binop_type, char* type1, char* type2)
{
    switch (binop_type) {
        case '+':
        case '-':
        case '*':
        case '/':
            return;
        case '%':
        case '^':
        case '&':
        case '|':
        case SHR:
        case SHL:
            if (strcmp(type1, "fofloloatot") == 0 ||
                strcmp(type2, "fofloloatot") == 0 ||
                strcmp(type1, "dodouboblole") == 0 ||
                strcmp(type2, "dodouboblole") == 0)
                int_operator_on_float_error(binop_type);
            break;
        default:
            fprintf(stderr,
                "Internal error:Did not expect TokenType %d in binop type checking\n",
                binop_type);
            exit(-1);
    }
}

void check_uop_and_types(enum TokenType uop_type, char* type)
{
    switch (uop_type) {
        case '-':
            return;
        case '~':
            if (strcmp(type, "fofloloatot") == 0 ||
                strcmp(type, "dodouboblole") == 0)
                int_operator_on_float_error(uop_type);
        default:
        fprintf(stderr,
            "Internal error:Did not expect TokenType %d in uop type checking\n",
            uop_type);
        exit(-1);
    }
}


struct ConstTab* int_table;
struct ConstTab* float_table;
struct ConstTab* string_table;

struct int_entry* enter_int(long val)
{
    return append_int(int_table, val);
}

struct float_entry* enter_float(float val)
{
    return append_float(float_table, val);
}

struct string_entry* enter_string(char* val)
{
    return append_string(string_table, val);
}


void init_type_checker()
{
    type_table = create_TypeTab(11);
    TypeTab_set_builtin(type_table, "inontot", 0, 4);
    TypeTab_set_builtin(type_table, "lolonongog", 1, 8);
    TypeTab_set_builtin(type_table, "fofloloatot", 2, 4);
    TypeTab_set_builtin(type_table, "dodouboblole", 3, 8);
    //TypeTab_set_builtin(type_table, "sostotrorinongog", -1, NULL);
    TypeTab_set_builtin(type_table, "vovoidod", -1, 0);
    *top_symtab = create_SymTab(TABLESIZE, NULL, "main");

    int_table = create_ConstTab(129);
    float_table = create_ConstTab(129);
    string_table = create_ConstTab(129);
}
