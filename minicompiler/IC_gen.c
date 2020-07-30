#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "IC_gen.h"

/*
 *  Intermediate code generation
 */

int label_num = 0;
#define MAX_LABEL_LENGTH 10
char* get_temp()
{
    char temp[MAX_LABEL_LENGTH+1];
    sprintf(temp, "t%10d", label_num);
    label_num++;
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

/*
 * The following two functions don't generate any code
 * since declarations are not included in the IC, and are instead
 * presevered in the symbol table.
 */
void visit_VarDecl(struct VarDecl* node)
{
    return;
}

void visit_StructDecl(struct StructDecl* node)
{
    return;
}

void visit_FuncDecl(struct FuncDecl* node)
{

}

void visit_VarAcc(struct VarAcc* node)
{

}

void visit_Expr(struct Expr* node)
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

void visit_FuncCall(struct FuncCall* node)
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
