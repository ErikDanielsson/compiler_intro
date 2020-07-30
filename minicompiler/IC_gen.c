#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "type_checker.h"
#include "symbol_table.h"
#include "IC_gen.h"

/*
 *  Intermediate code generation
 */

int label_num = 0;
#define MAX_LABEL_LENGTH 10
char temp[MAX_LABEL_LENGTH+1];
char* get_temp()
{
    sprintf(temp, "t%10d", label_num);
    label_num++;
    return temp;
}

const char* ic_filename = "IR_file.tmp";
FILE* IC_file_desc;

/*
 * Generates a simple three address code, written to chosen output
 * Variable names are not presevered, since the same name can appear
 * in different scopes. Instead a hex value is emitted in the code,
 * and the correspnding name is entered into an array
 */
void generate_IC(struct CompStmt* node)
{
    IC_file_desc = fopen(ic_filename, "w");
    visit_CompStmt(node);
    fclose(IC_file_desc);
}

void emit(char* instr)
{
    fprintf(IC_file_desc, "\t%s\n", instr);
}

void emitlabel(char* label)
{
    fprintf(IC_file_desc, "%s:", label);
}

void widening_error(char* type1, char* type2)
{
    fprintf(stderr, "\033[1;31merror\033[0m:Unable cast '%s' to '%s' or vice versa\n", type1, type2);
    exit(-1);
}

int max(char* type1, char* type2)
{
    int a = get_widening_type(type1);
    int b = get_widening_type(type2);
    return (a - b) * (a != b);
}

char* widen(char* addr_a, char* type1, char* type2)
{
    if (strcmp(type1, type2) == )
        return addr_a;
    if (SymTab_type_declared(basic_types, type1) &&
        SymTab_type_declared(basic_types, type1)) {
            char* caster;
            caster = type1 ? max(type1, type2) > 0 : type2;
            char* temp = get_temp();
            char instr[strlen(caster)+strlen(temp)+strlen(addr_a)+2+3];
            sprintf(instr, "%s = (%s)%s", temp, caster, addr_a)
            emit(instr);
            return temp;

    } else {
        widening_error(type1, type2);
        return NULL;
    }

}


void visit_CompStmt(struct CompStmt* node)
{
    for (int i = 0; i < node->n_statements; i++)
        visit_Stmt(node->statement_list[i]);
}

void visit_Stmt(struct Stmt* node)
{
    switch (node->statement_type) {
        case VARIABLE_DECLARATION:
            visit_VarDecl(node->stmt);
            return;
        case FUNCTION_DECLARATION:
            visit_FuncDecl(node->stmt);
            return;
        case STRUCT_DECLARATION:
            visit_StructDecl(node->stmt);
            return;
        case ASSIGNMENT_STATEMENT:
            visit_AStmt(node->stmt);
            return;
        case FUNCTION_CALL:
            visit_FuncCall(node->stmt);
            return;
        case IF_ELIF_ELSE_STATEMENT:
            visit_IEEStmt(node->stmt);
            return;
        case WHILE_LOOP:
            visit_WLoop(node->stmt);
            return;
        case FOR_LOOP:
            visit_FLoop(node->stmt);
            return;
        case SCOPE:
            visit_CompStmt(node->stmt);
            return;
        case RETURN_STATEMENT:
            visit_ReturnStmt(node->stmt);
            return;
        default:
            fprintf(stderr, "Internal error:Did not expect NodeType: %d, in visit_Stmt",
                    node->statement_type);
            return;
    }
}


void visit_VarDecl(struct VarDecl* node)
{

}

void visit_StructDecl(struct StructDecl* node)
{

}

void visit_FuncDecl(struct FuncDecl* node)
{

}

char* visit_VarAcc(struct VarAcc* node)
{

}

char* visit_Expr(struct Expr* node)
{
    switch (node->type) {
        case BINOP:
            visit_Expr(node->left);
            printf("\n");
            visit_Expr(node->right);
            return;
        case UOP:
            printf("\n");
            visit_Expr(node->expr);
            return;
        case CONST:
            printf("\n");
            return;
        case FUNCCALL:
            printf("\n");
            return;
        case VARACC:
            printf("\n");
            return;
    }
}

void visit_AStmt(struct AStmt* node)
{

}

char* visit_FuncCall(struct FuncCall* node)
{

}

void visit_IEEStmt(struct IEEStmt* node)
{

}

void visit_CondStmt(struct CondStmt* node)
{

}

void visit_WLoop(struct CondStmt* node)
{

}

void visit_FLoop(struct FLoop* node)
{

}


void visit_ReturnStmt(struct Expr* node)
{

}
