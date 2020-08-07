#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "lexer.h"
#include "parser.h"
#include "type_checker.h"
#include "symbol_table.h"
#include "tree_climber.h"
#include "lexer.h"
#include "hashing.h"
#include "symbol_table.h"
#include "IC_table.h"
#include "intermediate_code.c"

/*
 *  Intermediate code generation
 */

//struct IC intermediate_code;

FILE* IC_file_desc;
char* ic_filename = "IR_file.tmp";

int temp_num = 0;
#define MAX_LABEL_LENGTH 10
char* newtemp()
{
    char* temp = malloc(MAX_LABEL_LENGTH+2);
    sprintf(temp, "t%d", temp_num);
    temp_num++;
    return temp;
}

int label_num = 0;
char* newlabel()
{
    char* label = malloc(sizeof(char)*MAX_LABEL_LENGTH+2);
    sprintf(label, "L%d", label_num);
    label_num++;
    return label;
}

void generate_IC(struct CompStmt* node)
{

    IC_file_desc = fopen(ic_filename, "w");
    fprintf(IC_file_desc, "\n\nCode begin\n\n");
    visit_CompStmt(node);
    fprintf(IC_file_desc, "\n\nCode end\n\n");
    fclose(IC_file_desc);
}

void emit(char* instr, ...)
{
    va_list args;
    fprintf(IC_file_desc, "\t");
    va_start(args, instr);
    vfprintf(IC_file_desc, instr, args);
    va_end(args);
    fprintf(IC_file_desc, "\n");
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
    check_type_defined(type1);
    check_type_defined(type2);

    char* caster = max(type1, type2);
    char* temp = newtemp();
    append_triple(gen_conv(caster, addr_a, temp), QUAD_CONV);
    emit("%s = (%s)%s", temp, caster, addr_a);
    return temp;
}

char* relop_inv(char* relop)
{
    /*
     * Determines inverse by predetermined hash values
     */
    switch(max_hash(relop)) {
        case 0x390caefb:
            // Inv of '<'
            return "=>";
        case 0x3b0cb221:
            // Inv of '>'
            return "=<";
        case 0x8ff4db3c:
            // Inv of '=<'
            return ">";
        case 0x91f4de62:
            // Inv of '=>'
            return "<";
        case 0x90f4dccf:
            // Inv of '=='
            return "!=";
        case 0x90c34003:
            // Inv of '!='
            return "==";
        default:
            fprintf(stderr, "Internal error:Hash of %s does not match hash of any relop\n", relop);
            exit(-1);
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
            if (node->next == NULL)
                node->next = newlabel();
            ((struct IEEStmt*)(node->stmt))->next = node->next;

            visit_IEEStmt(node->stmt);
            emitlabel(node->next);

            return;
        case WHILE_LOOP:
            if (node->next == NULL)
                node->next = newlabel();
            ((struct CondStmt*)(node->stmt))->next = node->next;
            visit_WLoop(node->stmt);
            emitlabel(node->next);
            return;
        case FOR_LOOP:
            if (node->next == NULL)
                node->next = newlabel();
            ((struct FLoop*)(node->stmt))->next = node->next;
            visit_FLoop(node->stmt);
            emitlabel(node->next);
            return;
        case SCOPE:
            push_Env();
            visit_CompStmt(node->stmt);
            pop_Env();
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
    char* var_type = node->type->lexeme;
    check_type_defined(var_type);
    if (node->expr != NULL) {
        char* expr_type = visit_Expr_rval(node->expr);
        if (strcmp(max(var_type, expr_type), var_type) != 0)
            type_error(TRUE, "Cannot assign expression of type '%s' to variable '%s' of type '%s'",
                        expr_type, node->name->lexeme, var_type);
        char* curr_addr = widen(node->expr->addr, var_type, expr_type);
        check_and_set_var(node);
        emit("%s = %s", node->name->lexeme, curr_addr);
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

char* visit_Expr_rval(struct Expr* node)
{
    switch (node->type) {
        case EXPR_RELOP:{
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            char* a1 = widen(node->left->addr, type1, type);
            char* a2 = widen(node->right->addr, type2, type);
            char* true = newlabel();
            char* next = newlabel();
            char* t = newtemp();
            emit("if %s %s %s goto %s",
                a1, node->binary_op->lexeme,
                a2, true);
            emit("%s = 0", t);
            emit("goto %s", next);
            emitlabel(true);
            emit("%s = 1", t);
            emitlabel(next);
            node->addr = t;
            return "inontot";
        }
        case EXPR_AND: {
            char* true = newlabel();
            char* false = newlabel();
            char* next = newlabel();

            node->left->true = "fall";
            node->left->false = false;
            node->right->true = true;
            node->right->false = "fall";

            visit_Expr_jump(node->left);
            visit_Expr_jump(node->right);
            emitlabel(false);
            char* t = newtemp();
            emit("%s = 0", t);
            emit("goto %s", next);
            emitlabel(true);
            emit("%s = 1", t);
            emitlabel(next);
            node->addr = t;
            return "inontot";
        }
        case EXPR_OR: {
            char* true = newlabel();
            char* false = newlabel();
            char* next = newlabel();
            node->left->false = "fall";
            node->left->true = true;
            node->right->true = "fall";
            node->right->false = false;

            visit_Expr_jump(node->left);
            visit_Expr_jump(node->right);
            emitlabel(true);
            char* t = newtemp();
            emit("%s = 1", t);
            emit("goto %s", next);
            emitlabel(false);
            emit("%s = 0", t);
            emitlabel(next);
            node->addr = t;
            return "inontot";
        }
        case EXPR_NOT: {
            char* true = newlabel();
            char* next = newlabel();
            printf("%s", next);
            node->expr->true = "fall";
            node->expr->false = true;
            visit_Expr_jump(node->expr);
            char* t = newtemp();
            emit("%s = 0", t);
            emit("goto %s", next);
            emitlabel(true);
            emit("%s = 1", t);
            emitlabel(next);
            node->addr = t;
            return "inontot";
        }

        case EXPR_BINOP: {
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            char* a1 = widen(node->left->addr, type1, type);
            char* a2 = widen(node->right->addr, type2, type);
            node->addr = newtemp();
            emit("%s = %s %c %s", node->addr, a1, node->binary_op->c_val, a2);
            return type;

        }
        case EXPR_UOP: {
            char* type = visit_Expr_rval(node->expr);
            node->addr = newtemp();
            emit("%s = neg %s", node->addr, node->expr->addr);
            return type;
        }
        case EXPR_CONST: {
            switch (node->val->type) {

                case ICONST:
                    node->addr = malloc(12);
                    sprintf(node->addr, "%ld", node->val->i_val);
                    return "inontot";
                case FCONST:
                    node->addr = malloc(11);
                    sprintf(node->addr, "%lf", node->val->f_val);
                    return "fofloloatot";
                case SCONST:

                    node->addr = node->val->lexeme;
                    return "sostotrorinongog";
                default:
                    fprintf(stderr,
                            "internal error:Did not expect TokenType: %d, expr EXPR_CONST switch",
                            node->val->type);
                    exit(-1);
            }
        }
        case EXPR_FUNCCALL: {
            char* type = visit_FuncCall(node->function_call);
            node->addr = node->function_call->addr;
            return type;
        }
        case EXPR_VARACC:
            node->addr = node->variable_access->variable->lexeme;
            return visit_VarAcc(node->variable_access);
    }
}

void visit_Expr_jump(struct Expr* node)
{
    switch (node->type) {
        case EXPR_RELOP:{
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            char* a1 = widen(node->left->addr, type1, type);
            char* a2 = widen(node->right->addr, type2, type);
            if (strcmp(node->true, "fall") == 0) {
                if (strcmp(node->false, "fall") != 0)
                    emit("if %s %s %s goto %s", a1,
                            relop_inv(node->binary_op->lexeme), a2,
                            node->false);
            } else if (strcmp(node->false, "fall") == 0) {
                emit("if %s %s %s goto %s", a1,
                        node->binary_op->lexeme, a2,
                        node->true);
            } else {
                emit("if %s %s %s goto %s", a1,
                        node->binary_op->lexeme, a2,
                        node->true);
                emit("goto %s", node->false);
            }
            return;
        }
        case EXPR_AND:
            node->left->true = "fall";
            node->right->true = node->true;
            node->right->false = node->false;
            if (strcmp(node->false, "fall") == 0) {
                node->left->false = newlabel();
                visit_Expr_jump(node->left);
                visit_Expr_jump(node->right);
                emitlabel(node->left->false);
            } else {
                node->left->false = node->false;
                visit_Expr_jump(node->left);
                visit_Expr_jump(node->right);
            }
            return;
        case EXPR_OR:
            node->left->false = "fall";
            node->right->true = node->true;
            node->right->false = node->false;
            if (strcmp(node->true, "fall") == 0) {
                node->left->true = newlabel();
                visit_Expr_jump(node->left);
                visit_Expr_jump(node->right);
                emitlabel(node->left->true);
            } else {
                node->left->true = node->true;
                visit_Expr_jump(node->left);
                visit_Expr_jump(node->right);
            }
            return;
        case EXPR_NOT:
            node->expr->true = node->false;
            node->expr->false = node->true;
            visit_Expr_jump(node->expr);
            return;
        case EXPR_BINOP: {
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            char* a1 = widen(node->left->addr, type1, type);
            char* a2 = widen(node->right->addr, type2, type);
            node->addr = newtemp();
            emit("%s = %s %c %s", node->addr, a1, node->binary_op->c_val, a2);
            if (strcmp(node->true, "fall") != 0) {
                if (strcmp(node->false, "fall") != 0)
                    emit("if %s == 0 goto %s", node->addr, node->false);
            } else if (strcmp(node->false, "fall") != 0) {
                emit("if %s != 0 goto %s", node->addr, node->true);
            } else {
                emit("if %s != 0 goto %s", node->addr, node->true);
                emit("goto %s", node->false);
            }
            return;
        }
        case EXPR_UOP: {
            visit_Expr_rval(node->expr);
            /*
             * Since the truth value of a number doesn't depend on it's
             * sign, we can omit the instruction for sign change.
             */
            if (strcmp(node->true, "fall") == 0) {
                if (strcmp(node->false, "fall") != 0)
                    emit("if %s == 0 goto %s", node->expr->addr, node->false);
            } else if (strcmp(node->false, "fall") == 0) {
                emit("if %s != 0 goto %s", node->expr->addr, node->true);
            } else {
                emit("if %s != 0 goto %s", node->expr->addr, node->true);
                emit("goto %s", node->false);
            }
            return;
        }
        case EXPR_CONST: {
            switch (node->val->type) {
                /*
                 * Since the truth value of constant is known at compile time
                 * we can simply output an unconditional jump.
                 */
                case ICONST:
                    if (node->val->i_val) {
                        if (strcmp(node->true, "fall") != 0)
                            emit("goto %s", node->true);
                    } else {
                        if (strcmp(node->false, "fall") != 0)
                            emit("goto %s", node->false);
                    }
                    return;
                case FCONST:
                    if (node->val->f_val) {
                        if (strcmp(node->true, "fall") != 0)
                            emit("goto %s", node->true);
                    } else {
                        if (strcmp(node->false, "fall") != 0)
                            emit("goto %s", node->false);
                    }
                    return;
                case SCONST:
                    if (node->val->lexeme[0] == 0x00) {
                        if (strcmp(node->true, "fall") != 0)
                            emit("goto %s", node->true);
                    } else {
                        if (strcmp(node->false, "fall") != 0)
                            emit("goto %s", node->false);
                    }
                    return;
                default:
                    fprintf(stderr,
                            "internal error:Did not expect TokenType: %d, expr_jump EXPR_CONST switch",
                            node->val->type);
                    exit(-1);
            }
        }
        case EXPR_FUNCCALL:
            visit_FuncCall(node->function_call);
            if (strcmp(node->true, "fall") == 0) {
                if (strcmp(node->false, "fall") == 0)
                    emit("if %s == 0 goto %s", node->function_call->addr, node->false);
            } else if (strcmp(node->false, "fall") != 0) {
                emit("if %s != 0 goto %s", node->function_call->addr, node->true);
            } else {
                emit("if %s != 0 goto %s", node->function_call->addr, node->true);
                emit("goto %s", node->false);
            }
            return;
        case EXPR_VARACC:
            visit_VarAcc(node->variable_access);
            if (strcmp(node->true, "fall") == 0) {
                if (strcmp(node->false, "fall") != 0)
                    emit("if %s == 0 goto %s", node->variable_access->variable->lexeme, node->false);
            } else if (strcmp(node->false, "fall") == 0) {
                emit("if %s != 0 goto %s", node->variable_access->variable->lexeme, node->true);
            } else {
                emit("if %s != 0 goto %s", node->variable_access->variable->lexeme, node->true);
                emit("goto %s", node->false);
            }
            return;
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
        char* arg_type = visit_Expr_rval(node->args[i]);
        widen(node->args[i]->addr, decl->params[i]->type->lexeme, arg_type);
    }
    for (int i = 0; i < node->n_args; i++) {
        emit("param %s", node->args[i]->addr);
    }
    char* temp = newtemp();
    emit("%s = call %s",temp,  node->func->lexeme);
    node->addr = temp;
    return decl->type->lexeme;
}

void visit_AStmt(struct AStmt* node)
{
    char* var_name = node->variable_access->variable->lexeme;
    char* var_type = visit_VarAcc(node->variable_access);
    if (node->assignment_type->type == SUFFIXOP) {
        char* temp = newtemp();
        switch (node->assignment_type->lexeme[0]) {
            case '+':
                emit("%s = %s + 1", temp, var_name);
                emit("%s = %s", var_name, temp);
                break;
            case '-':
                emit("%s = %s - 1", temp, var_name);
                emit("%s = %s", var_name, temp);
                break;
            case '*':
                emit("%s = %s << 1", temp, var_name);
                emit("%s = %s", var_name, temp);
                break;
            case '/':
                emit("%s = %s >> 1", temp, var_name);
                emit("%s = %s", var_name, temp);
                break;
            default:
                fprintf(stderr, "Internal error:Did not expect '%c' as suffixop\n",
                        node->assignment_type->lexeme[0]);
        }

    } else {
        char* expr_type = visit_Expr_rval(node->expr);
        if (strcmp(max(var_type, expr_type), var_type) != 0)
            type_error(TRUE, "Cannot assign expression of type '%s' to variable '%s' of type '%s'",
                        expr_type, node->variable_access->variable->lexeme, var_type);
        char* addr = widen(node->expr->addr, expr_type, var_type);
        if (node->assignment_type->type != '=') {
            /*
             * This is a bit of a hack
             */
            char* temp = newtemp();
            emit("%s = %s %c %s", temp, var_name,
                node->assignment_type->lexeme[0], addr);
            emit("%s = %s", var_name, temp);
        } else {
            emit("%s = %s", var_name, addr);
        }

    }
}



void visit_IEEStmt(struct IEEStmt* node)
{
    if (node->_else != NULL) {
        char* next = node->next;
        if_with_else(node->if_stmt, next);
        for (int i = 0; i < node->n_elifs; i++)
            if_with_else(node->elif_list[i], next);
        push_Env();
        node->_else->next = next;
        visit_CompStmt(node->_else);
        pop_Env();
    } else if (node->n_elifs == 0) {
        node->if_stmt->boolean->true = "fall";
        node->if_stmt->boolean->false = node->if_stmt->body->next = node->next;
        visit_Expr_jump(node->if_stmt->boolean);
        push_Env();
        visit_CompStmt(node->if_stmt->body);
        pop_Env();
    } else {
        char* next = node->next;
        if_with_else(node->if_stmt, next);
        int n_elifs_minus_1 = node->n_elifs-1;
        for (int i = 0; i < n_elifs_minus_1; i++)
            if_with_else(node->elif_list[i], next);
        struct CondStmt* last_elif = node->elif_list[n_elifs_minus_1];
        last_elif->boolean->true = "fall";
        last_elif->boolean->false = node->if_stmt->body->next = node->next;
        visit_Expr_jump(last_elif->boolean);
        push_Env();
        visit_CompStmt(last_elif->body);
        pop_Env();
    }
}

void if_with_else(struct CondStmt* node, char* next)
{
    node->boolean->true = "fall";
    char* falsel = newlabel();
    node->boolean->false = falsel;
    node->body->next = next;
    visit_Expr_jump(node->boolean);
    push_Env();
    visit_CompStmt(node->body);
    pop_Env();
    emit("goto %s", next);
    emitlabel(falsel);
}

void visit_WLoop(struct CondStmt* node)
{
    char* begin = newlabel();

    emitlabel(begin);
    node->boolean->true = "fall";
    node->boolean->false = node->next;
    visit_Expr_jump(node->boolean);
    node->body->next = begin;
    push_Env();
    visit_CompStmt(node->body);
    pop_Env();
    emit("goto %s", begin);
}

void visit_FLoop(struct FLoop* node)
{
    push_Env();
    if (node->type == VARIABLE_DECLARATION)
        visit_VarDecl(node->init_stmt);
    else
        visit_AStmt(node->init_stmt);
    char* begin = newlabel();
    emitlabel(begin);
    node->boolean->true = "fall";
    node->boolean->false = node->next;
    visit_Expr_jump(node->boolean);


    visit_CompStmt(node->body);
    pop_Env();
    visit_AStmt(node->update_statement);
    emit("goto %s", begin);
}


void visit_ReturnStmt(struct Expr* node)
{
    if (!in_function)
        return_not_in_func_error();
    char* type = visit_Expr_rval(node);
    widen(node->addr, type, function_type);

}
