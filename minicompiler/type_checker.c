#include <stdio.h>
#include <stdlib.h>
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

struct SymTab* basic_type_table;

void init_type_checker()
{
    basic_type_table = create_SymTab(4, NULL);
    SymTab_set_type(type_table, "inontot", STRUCTURE, 0);
    SymTab_set_type(type_table, "lolonongog", STRUCTURE, 1);
    SymTab_set_type(type_table, "fofloloatot", STRUCTURE, 2);
    SymTab_set_type(type_table, "dodouboblole", STRUCTURE, 3);
    SymTab_set_type(type_table, "sostotrorinongog", STRUCTURE, -1);
    SymTab_set_type(type_table, "vovoidod", STRUCTURE, -1);
}


struct SymTab* symbol_table_stack[NESTINGDEPTH];
struct SymTab** top_symtab = symbol_table_stack;

int offset_stack[NESTINGDEPTH];
int* offset = offset_stack;

void push_Env()
{
    if (top_symtab-symbol_table_stack >= NESTINGDEPTH)
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
    (*top_symtab)->parent = NULL;
    top_symtab--;
    SymTab_append_child(*top_symtab, *(top_symtab+1));
    offset--;
}

void enter_struct_decl(struct StructDecl* node)
{
    if (SymTab_check_and_set(type_table, node->type_name->lexeme, STRUCTURE, node))
        type_error("Redefinition of structure '%s'\n", node->type_name->lexeme, TRUE);
}

void check_vardecl(struct VarDecl* node)
{
    if (!SymTab_type_declared(type_table, node->type->lexeme))
        type_error("type '%s' is not declared", node->type->lexeme, TRUE);
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, VARIABLE, node))
        type_error("Redefinition of variable '%s'\n", node->name->lexeme, TRUE);
}

void check_func_decl(struct FuncDecl* node)
{
    if (!SymTab_type_declared(type_table, node->type->lexeme))
        type_error("type '%s' is not declared", node->type->lexeme, TRUE);
    if (SymTab_check_and_set(*top_symtab, node->name->lexeme, FUNCTION, node))
        type_error("Redefinition of function '%s'\n", node->name->lexeme, TRUE);
}
