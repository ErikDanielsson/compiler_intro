#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "type_checker.h"
#include "symbol_table.h"
#include "IC_gen.h"

/*
 *  Intermediate code generation
 */

int label_num = 0;
#define MAX_LABEL_LENGTH 10
char* newtemp()
{
    char* temp = malloc(MAX_LABEL_LENGTH+2);
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

char* max(char* type1, char* type2)
{
    int a = get_widening_type(type_table, type1);
    int b = get_widening_type(type_table, type2);
    return (a > b) ? type1 : type2;
}

char* widen(char* addr_a, char* type1, char* type2)
{
    if (strcmp(type1, type2) == 0)
        return addr_a;
    if (SymTab_type_declared(type_table, type1) &&
        SymTab_type_declared(type_table, type1)) {

            char* caster = max(type1, type2);
            //strcpy(temp2, addr_a);
            //char* temp = get_temp();
            //char instr[strlen(caster)+strlen(temp)+strlen(addr_a)+2+3];
            //sprintf(instr, "%s = (%s)%s", temp, caster, temp2);
            //emit(instr);
            return NULL;
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
            fprintf(stderr, "internal error:Did not expect NodeType: %d, in visit_Stmt",
                    node->statement_type);
            return;
    }
}


void visit_VarDecl(struct VarDecl* node)
{
    char* type_name = node->type->lexeme;
    check_type_defined(type_name);
    if (node->expr != NULL) {
        char* expr_type_name = visit_Expr(node->expr);
        char* curr_addr = widen(node->expr->addr, type_name, expr_type_name);
        check_and_set_var(node);
        char instr[strlen(node->name->lexeme)+strlen(curr_addr)+3];
        sprintf(instr, "%s = %s", node->name->lexeme, curr_addr);
        emit(instr);
    } else {
        check_and_set_var(node);
    }
}

void visit_StructDecl(struct StructDecl* node)
{
    push_Env();
    for (int i = 0; i < node->n_decl; i++)
        visit_VarDecl(node->fields[i]);
    char* type_name = node->type_name->lexeme;
    enter_type_def(type_name, pop_Env_struct());
}

char in_function = FALSE;
char* function_type;
void visit_FuncDecl(struct FuncDecl* node)
{
    if (in_function)
        nested_function_error(node);
    in_function = TRUE;
    function_type = node->type->lexeme;
    check_type_defined(function_type);
    check_and_set_func(node);
    push_Env();
    for (int i = 0; i < node->n_params; i++)
        visit_VarDecl(node->params[i]);
    visit_CompStmt(node->body);
    pop_Env();
    in_function = FALSE;
}

char* visit_Expr(struct Expr* node)
{
    switch (node->type) {
        case BINOP: {
            enum TokenType binop_type = node->binary_op->typ;
            if (binop_type == RELOP)
                return visit_relop(node);
            else if (binop_type == AND)
                return visit_and(node);
            else if (binop_type == OR)
                return visit_or(node);
            else if (binop_type == '!')
                return visit_not(node);
            else {
                char* type1 = visit_Expr(node->left);
                char* type2 = visit_Expr(node->right);
                char* type = max(type1, type2);
                char* a1 = widen(node->left->addr, type1, type);
                char* a2 = widen(node->right->addr, type2, type);
                node->addr = newtemp();
                char instr[(MAX_LABEL_LENGTH+1)*3+5+1];
                if (binop_type < 128)
                    sprintf(instr, "%s = %s %c %s", node->addr, a1,
                            node->binary_op->c_val, a2);
                else
                    sprintf(instr, "%s = %s %s %s", node->addr, a1,
                            node->binary_op->lexeme, a2);
                emit(instr);
                return type;
            }
        }
        case UOP: {
            printf("expr\n");
            print_Expr(node->expr ,0,0,0);
            char* type = visit_Expr(node->expr);
            printf("end\n");
            node->addr = newtemp();
            char instr[(MAX_LABEL_LENGTH+1)*2+7+1];
            sprintf(instr, "%s = neg %s", node->addr, node->expr->addr);
            emit(instr);
            return type;
        }
        case CONST: {
            switch (node->val->type) {

                case ICONST:
                    node->addr = malloc(12);
                    sprintf(node->addr, "%11ld", node->val->i_val);
                    printf("in i const: %s", node->addr);
                    return "inontot";
                case FCONST:
                    node->addr = malloc(11);
                    sprintf(node->addr, "%11lf", node->val->f_val);
                    return "fofloloatot";
                case SCONST:

                    node->addr = node->val->lexeme;
                    return "sostotrorinongog";
                default:
                    fprintf(stderr,
                            "internal error:Did not expect TokenType: %d, expr CONST switch",
                            node->val->type);
                    exit(-1);
            }


        }
        case FUNCCALL:
            node->addr = newtemp();
            return visit_FuncCall(node->function_call);
        case VARACC:
            node->addr = node->variable_access->variable->lexeme;
            return visit_VarAcc(node->variable_access);
    }
}
char* visit_VarAcc(struct VarAcc* node)
{
    char* type_name = check_var_declared(node);
    // Code code
    return type_name;
}

char* visit_FuncCall(struct FuncCall* node)
{
    struct FuncDecl* decl = check_func_declared(node);
    int n_args = node->n_args;
    if (n_args != decl->n_params)
        mismatching_params_error(node, decl);
    for (int i = 0; i < node->n_args; i++) {
        char* arg_type = visit_Expr(node->args[i]);
        widen(node->args[i]->addr, decl->params[i]->type->lexeme, arg_type);
    }

    for (int i = 0; i < node->n_args; i++) {
        char instr[MAX_LABEL_LENGTH+1+6+1];
        sprintf(instr, "param %s", node->args[i]->addr);
        emit(instr);
    }
    char instr[5+strlen(node->func->lexeme)];
    sprintf(instr, "call %s", node->func->lexeme);
    emit(instr);
    return decl->type->lexeme;
}

void visit_AStmt(struct AStmt* node)
{
    char* type1 = visit_VarAcc(node->variable_access);
    char* type2 = visit_Expr(node->expr);
    char* tt = widen(node->expr->addr, type1, type2);
    char* var_name = node->variable_access->variable->lexeme;
    int var_name_len =strlen(var_name);
    if (node->assignment_type->type != '=')
    {
        char instr[(MAX_LABEL_LENGTH+1)*2+var_name_len+3];
        sprintf(instr, "%s = %s %c %s", node->expr->addr,
                var_name,
                node->assignment_type->lexeme[0],
                node->expr->addr);
        emit(instr);
    }
    char instr[(MAX_LABEL_LENGTH+1)*2+3];
    sprintf(instr, "%-11s = %s", var_name,node->expr->addr);
    emit(instr);
}



void visit_IEEStmt(struct IEEStmt* node)
{
    visit_CondStmt(node->if_stmt);
    for (int i = 0; i < node->n_elifs; i++)
        visit_CondStmt(node->elif_list[i]);
    if (node->_else != NULL) {
        push_Env();
        visit_CompStmt(node->_else);
        pop_Env();
    }
}

void visit_CondStmt(struct CondStmt* node)
{

    push_Env();
    visit_Expr(node->boolean);
    visit_CompStmt(node->body);
    pop_Env();
}

void visit_WLoop(struct CondStmt* node)
{
    visit_CondStmt(node);
}

void visit_FLoop(struct FLoop* node)
{
    if (node->type == VARIABLE_DECLARATION)
        visit_VarDecl(node->init_stmt);
    else
        visit_AStmt(node->init_stmt);
    visit_Expr(node->boolean);
    visit_AStmt(node->update_statement);
    visit_CompStmt(node->body);
}


void visit_ReturnStmt(struct Expr* node)
{
    if (!in_function)
        return_not_in_func_error();
    char* type = visit_Expr(node);
    widen(node->addr, type, function_type);

}
