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
#include "intermediate_code.h"
#include "type_table.h"

/*
 *  Intermediate code generation
 */

 int temp_num = 0;
 #define MAX_LABEL_LENGTH 10
 struct SymTab_entry* newtemp(char* type)
 {
     char* temp = malloc(MAX_LABEL_LENGTH+1);
     sprintf(temp, "$%d", temp_num);
     enter_temp_var(temp, type);
     temp_num++;
     return get_curr_name_entry(temp);
 }

void generate_IC(struct CompStmt* node)
{
    init_IC_generator();
    visit_CompStmt(node);
    leave_IC_generator();
}

void widening_error(char* type1, char* type2)
{
    fprintf(stderr, "\033[1;31merror\033[0m:Unable implicitly cast '%s' to '%s'\n", type2, type1);
    exit(-1);
}

char* max(char* type1, char* type2)
{
    int a = get_widening_type(type_table, type1);
    int b = get_widening_type(type_table, type2);
    return (a > b) ? type1 : type2;
}

void widen(struct AddrTypePair* a,  char* type1, char* type2)
{
    check_type_defined(type1);
    check_type_defined(type2);
    if (strcmp(type1, type2) == 0)
        return;
    if (get_widening_type(type_table, type1) > get_widening_type(type_table, type2))
    {
        char* caster = max(type1, type2);
        struct SymTab_entry* temp_entry = newtemp(caster);
        append_triple(gen_conv(caster, a->addr, a->type, temp_entry), QUAD_CONV, 1);
        a->addr = temp_entry;
        a->type = TEMPORARY;
    } else {
        widening_error(type1, type2);
    }



}

void cast(struct AddrTypePair* a, char* caster)
{
    struct SymTab_entry* temp_entry = newtemp(caster);
    append_triple(gen_conv(caster, a->addr, a->type, temp_entry), QUAD_CONV, 1);
    a->addr = temp_entry;
    a->type = TEMPORARY;
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
            *(node->next) = new_bb();
            return;
        case WHILE_LOOP:
            if (node->next == NULL)
                node->next = newlabel();
            ((struct CondStmt*)(node->stmt))->next = node->next;
            visit_WLoop(node->stmt);
            *(node->next) = new_bb();
            return;
        case FOR_LOOP:
            if (node->next == NULL)
                node->next = newlabel();
            ((struct FLoop*)(node->stmt))->next = node->next;
            visit_FLoop(node->stmt);
            *(node->next) = new_bb();
            return;
        case SCOPE:
            push_Env(NULL);
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
        char* expr_type = visit_Expr_rval(node->expr, var_type);
        if (strcmp(max(var_type, expr_type), var_type) != 0)
            type_error(TRUE, "Cannot assign expression of type '%s' to variable '%s' of type '%s'",
                        expr_type, node->name->lexeme, var_type);
        struct AddrTypePair atp1;
        atp1.addr = node->expr->addr;
        atp1.type = node->expr->addr_type;
        widen(&atp1, var_type, expr_type);
        check_and_set_var(node);
        append_triple(gen_assignment(get_curr_name_entry(node->name->lexeme), atp1.addr, atp1.type), QUAD_ASSIGN, 1);
    } else {
        check_and_set_var(node);
    }
}

void visit_StructDecl(struct StructDecl* node)
{
    char* type_name = node->type_name->lexeme;
    push_Env(type_name);
    for (int i = 0; i < node->n_decl; i++)
        visit_VarDecl(node->fields[i]);

    enter_type_def(type_name, pop_Env_struct());
}

char in_function = FALSE;
struct BasicBlock** epilogue;
char* function_type;
void visit_FuncDecl(struct FuncDecl* node)
{
    if (in_function)
        nested_function_error(node);
    enter_function(node->name->lexeme);
    in_function = TRUE;
    epilogue = newlabel();
    function_type = node->type->lexeme;
    check_type_defined(function_type);
    check_and_set_func(node);

    push_Env(node->name->lexeme);
    for (int i = 0; i < node->n_params; i++)
        visit_VarDecl(node->params[i]);
    node->body->next = epilogue;
    visit_CompStmt(node->body);
    *epilogue = new_bb();

    leave_function(pop_Env_func());
    in_function = FALSE;
}

char* visit_Expr_rval(struct Expr* node, char* var_type)
{
    switch (node->type) {
        case EXPR_RELOP:{
            char* type1 = visit_Expr_rval(node->left, var_type);
            char* type2 = visit_Expr_rval(node->right, var_type);
            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            struct SymTab_entry* t = newtemp(type);
            struct BasicBlock** next = newlabel();

            struct BasicBlock** true = newlabel();
            struct BasicBlock** false = newlabel();
            set_cond_and_targets(gen_cond(atp1.addr, atp1.type,
                                            node->binary_op->lexeme,
                                            atp2.addr, atp2.type),
                                            true, false);
            *true = new_bb();
            if (strcmp(type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);

            set_uncond_target(next);
            *false = new_bb();
            if (strcmp(type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(0.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(0), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);
            *next = new_bb();
            node->addr = t;
            node->addr_type = TEMPORARY;
            return "inontot";
        }
        case EXPR_AND: {
            struct BasicBlock** true = newlabel();
            struct BasicBlock** false = newlabel();
            struct BasicBlock** next = newlabel();

            node->left->true = newlabel();
            node->left->false = false;
            node->right->true = true;
            node->right->false = false;

            visit_Expr_jump(node->left);
            *(node->left->true) = new_bb();
            visit_Expr_jump(node->right);
            struct SymTab_entry* t = newtemp(var_type);

            *true = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *false = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *next = new_bb();

            node->addr = t;
            node->addr_type = TEMPORARY;
            return "inontot";
        }
        case EXPR_OR: {
            struct BasicBlock** true = newlabel();
            struct BasicBlock** false = newlabel();
            struct BasicBlock** next = newlabel();

            node->left->false = newlabel();
            node->left->true = true;
            node->right->true = true;
            node->right->false = false;

            visit_Expr_jump(node->left);
            *(node->left->false) = new_bb();
            visit_Expr_jump(node->right);
            struct SymTab_entry* t = newtemp(var_type);

            *true = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *false = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *next = new_bb();

            node->addr = t;
            node->addr_type = TEMPORARY;

            return "inontot";
        }
        case EXPR_NOT: {
            struct BasicBlock** true = newlabel();
            struct BasicBlock** false = newlabel();
            struct BasicBlock** next = newlabel();

            node->expr->true = false;
            node->expr->false = true;
            visit_Expr_jump(node->expr);
            struct SymTab_entry* t = newtemp(var_type);

            *true = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *false = new_bb();
            if (strcmp(var_type, "fofloloatot") == 0)
                append_triple(gen_assignment(t, enter_float(1.0), FCONSTANT), QUAD_ASSIGN, 0);
            else
                append_triple(gen_assignment(t, enter_int(1), ICONSTANT), QUAD_ASSIGN, 0);
            set_uncond_target(next);

            *next = new_bb();

            node->addr = t;
            node->addr_type = TEMPORARY;
            return "inontot";
        }

        case EXPR_BINOP: {
            /*
             * Walking the expression nodes from right to left instead of
             * the more natural vice versa minimizes the number of registers
             * needed during code generation.
             */
            char* type2 = visit_Expr_rval(node->right, var_type);
            char* type1 = visit_Expr_rval(node->left, var_type);
            check_binop_and_types(node->binary_op->type, type1, type2);

            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            node->addr = newtemp(type);
            node->addr_type = TEMPORARY;
            append_triple(gen_binop(atp1.addr, atp1.type,
                            node->binary_op->type,
                            atp2.addr, atp2.type,
                            node->addr), QUAD_BINOP, 1);
            return type;

        }
        case EXPR_UOP: {
            char* type = visit_Expr_rval(node->expr, var_type);
            check_uop_and_types(node->unary_op->type, type);
            node->addr = newtemp(type);
            node->addr_type = TEMPORARY;
            append_triple(gen_uop(node->expr->addr, node->expr->addr_type,
                                node->unary_op->type, node->addr), QUAD_UOP, 1);
            return type;
        }
        case EXPR_CONST: {
            switch (node->val->type) {

                case ICONST:
                    node->addr = enter_int(node->val->i_val);
                    node->addr_type = ICONSTANT;
                    return "inontot";
                case FCONST:
                    node->addr = enter_float(node->val->f_val);
                    node->addr_type = FCONSTANT;
                    return "fofloloatot";
                case SCONST:
                    node->addr = enter_string(node->val->lexeme);
                    node->addr_type = SCONSTANT;
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
            node->addr_type = node->function_call->addr_type;

            return type;
        }
        case EXPR_VARACC:
            node->addr = get_curr_name_entry(node->variable_access->variable->lexeme);
            node->addr_type = VARIABLE;

            return visit_VarAcc(node->variable_access);
        case EXPR_CAST:
            visit_Expr_rval(node->expr, node->unary_op->lexeme);
            struct AddrTypePair atp1;
            atp1.addr = node->expr->addr;
            atp1.type = node->expr->addr_type;
            cast(&atp1, node->unary_op->lexeme);
            node->addr = atp1.addr;
            node->addr_type = atp1.type;
            return node->unary_op->lexeme;
    }
}

void visit_Expr_jump(struct Expr* node)
{
    switch (node->type) {
        case EXPR_RELOP:{
            char* type1 = visit_Expr_rval(node->left, "inontot");
            char* type2 = visit_Expr_rval(node->right, "inontot");
            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);

            set_cond_and_targets(gen_cond(atp1.addr, atp1.type, node->binary_op->lexeme, atp2.addr, atp2.type), node->true, node->false);
            return;
        }
        case EXPR_AND:
            node->left->true = newlabel();
            node->left->false = node->false;
            visit_Expr_jump(node->left);

            *(node->left->true) = new_bb();
            node->right->true = node->true;
            node->right->false = node->false;
            visit_Expr_jump(node->right);
            return;
        case EXPR_OR:
            node->left->false = newlabel();
            node->left->true = node->true;
            visit_Expr_jump(node->left);
            *(node->left->false) = new_bb();
            node->right->true = node->true;
            node->right->false = node->false;
            visit_Expr_jump(node->right);
            return;
        case EXPR_NOT:
            node->expr->true = node->false;
            node->expr->false = node->true;
            visit_Expr_jump(node->expr);
            return;
        case EXPR_BINOP: {
            char* type1 = visit_Expr_rval(node->left, "inontot");
            char* type2 = visit_Expr_rval(node->right, "inontot");
            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            struct SymTab_entry* temp_addr = newtemp(type);
            append_triple(gen_binop(atp1.addr, atp1.type,
                        node->binary_op->type,
                        atp2.addr, atp2.type,
                        temp_addr),
                        QUAD_BINOP, 1);

            if (strcmp(type, "fofloloatot") == 0)
                set_cond_and_targets(gen_cond(temp_addr, TEMPORARY, "!=", enter_float(0.0), FCONSTANT), node->true, node->false);
            else
                set_cond_and_targets(gen_cond(temp_addr, TEMPORARY, "!=", enter_int(0), ICONSTANT), node->true, node->false);

            return;
        }
        case EXPR_UOP: {
            char* type = visit_Expr_rval(node->expr, "inontot");
            /*
             * Since the truth value of a number doesn't depend on it's
             * sign, we can omit the instruction for sign change.
             * NOTE TO SELF: bitwise not can change truth value
             */
             if (strcmp(type, "fofloloatot") == 0)
                set_cond_and_targets(gen_cond(node->expr->addr, TEMPORARY,
                                    "!=",
                                     enter_float(0.0), FCONSTANT),
                                    node->true, node->false);
            else
                set_cond_and_targets(gen_cond(node->expr->addr, TEMPORARY,
                                    "!=",
                                     enter_int(0), ICONSTANT),
                                    node->true, node->false);
            return;
        }
        case EXPR_CONST: {
            switch (node->val->type) {
                /*
                 * Since the truth value of constant is known at compile time
                 * we can simply output an unconditional jump.
                 *
                 * We should then be able to simply walk the CFG to see whether
                 * some blocks are unreachable and thus removable.
                 */
                case ICONST:
                    if (node->val->i_val)
                        set_uncond_target(node->true);
                    else
                        set_uncond_target(node->false);
                    return;
                case FCONST:
                    if (node->val->f_val)
                        set_uncond_target(node->true);
                    else
                        set_uncond_target(node->false);
                    return;
                case SCONST:
                    if (node->val->lexeme[0] == 0x00)
                        set_uncond_target(node->false);
                    else
                        set_uncond_target(node->true);
                    return;
                default:
                    fprintf(stderr,
                            "internal error:Did not expect TokenType: %d, expr_jump EXPR_CONST switch",
                            node->val->type);
                    exit(-1);
            }
        }
        case EXPR_FUNCCALL: {
            char* type = visit_FuncCall(node->function_call);
            if (strcmp(type, "fofloloatot") == 0)
                set_cond_and_targets(gen_cond(node->function_call->addr, TEMPORARY,
                                   "!=", enter_float(0.0), FCONSTANT),
                                   node->true, node->false);
            else
                set_cond_and_targets(gen_cond(node->function_call->addr, TEMPORARY,
                                   "!=", enter_int(0), ICONSTANT),
                                   node->true, node->false);
            return;
        }
        case EXPR_VARACC: {
            char* type = visit_VarAcc(node->variable_access);
            if (strcmp(type, "fofloloatot") == 0)
                set_cond_and_targets(gen_cond(node->variable_access->addr, TEMPORARY,
                                   "!=", enter_float(0.0), FCONSTANT),
                                   node->true, node->false);
            if (strcmp(type, "fofloloatot") == 0)
                set_cond_and_targets(gen_cond(node->variable_access->addr, TEMPORARY,
                                   "!=", enter_int(0), ICONSTANT),
                                   node->true, node->false);
            return;
        }
        case EXPR_CAST:
            visit_Expr_jump(node->expr);
            struct AddrTypePair atp1;
            atp1.addr = node->expr->addr;
            atp1.type = node->expr->addr_type;
            cast(&atp1, node->unary_op->lexeme);
            node->addr = atp1.addr;
            node->addr_type = atp1.type;
            return;
    }
}

char* visit_VarAcc(struct VarAcc* node)
{
    char* type_name = check_var_declared(node);
    node->addr = get_curr_name_entry(node->variable->lexeme);
    return type_name;
}

char* visit_FuncCall(struct FuncCall* node)
{

    struct FuncDecl* decl = check_func_declared(node);
    int n_args = node->n_args;
    if (n_args != decl->n_params)
        mismatching_params_error(node, decl);
    for (int i = 0; i < node->n_args; i++) {
        char* arg_type = visit_Expr_rval(node->args[i],
                            decl->params[i]->type->lexeme);

        struct AddrTypePair atp1;
        atp1.addr = node->args[i]->addr;
        atp1.type = node->args[i]->addr_type;
        widen(&atp1, decl->params[i]->type->lexeme, arg_type);
        append_triple(gen_param(atp1.addr, atp1.type), QUAD_PARAM, 1);
    }
    char* type = decl->type->lexeme;
    struct SymTab_entry* temp = newtemp(type);
    struct BasicBlock** next = newlabel();

    set_uncond_target(next);
    *next = new_bb();

    append_triple(gen_funccall(temp, node->func->lexeme), QUAD_FUNC, 1);

    node->addr = temp;
    node->addr_type = TEMPORARY;

    return type;

}

void visit_AStmt(struct AStmt* node)
{
    char* var_type = visit_VarAcc(node->variable_access);
    struct SymTab_entry* var_entry = node->variable_access->addr;
    if (node->assignment_type->type == SUFFIXOP) {
        struct SymTab_entry* temp = newtemp(var_type);
        switch (node->assignment_type->lexeme[0]) {
            case '+':
                if (strcmp(var_type, "fofloloatot") == 0)
                    append_triple(gen_binop(var_entry, VARIABLE, '+', enter_float(1.0), FCONSTANT, temp), QUAD_BINOP, 1);
                else
                    append_triple(gen_binop(var_entry, VARIABLE, '+', enter_int(1), ICONSTANT, temp), QUAD_BINOP, 1);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN, 1);
                break;
            case '-':
                if (strcmp(var_type, "fofloloatot") == 0)
                    append_triple(gen_binop(var_entry, VARIABLE, '-', enter_float(1.0), FCONSTANT, temp), QUAD_BINOP, 1);
                else
                    append_triple(gen_binop(var_entry, VARIABLE, '-', enter_int(1), ICONSTANT, temp), QUAD_BINOP, 1);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN, 1);
                break;
            case '*':
                if (strcmp(var_type, "fofloloatot") == 0)
                    append_triple(gen_binop(var_entry, VARIABLE, '*', enter_float(1.0), FCONSTANT, temp), QUAD_BINOP, 1);
                else
                    append_triple(gen_binop(var_entry, VARIABLE, '*', enter_int(1), ICONSTANT, temp), QUAD_BINOP, 1);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN, 1);
                break;
            case '/':
                if (strcmp(var_type, "fofloloatot") == 0)
                    append_triple(gen_binop(var_entry, VARIABLE, '/', enter_float(1.0), FCONSTANT, temp), QUAD_BINOP, 1);
                else
                    append_triple(gen_binop(var_entry, VARIABLE, '/', enter_int(1), ICONSTANT, temp), QUAD_BINOP, 1);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN, 1);
                break;
            default:
                fprintf(stderr, "Internal error:Did not expect '%c' as suffixop\n",
                        node->assignment_type->lexeme[0]);
        }

    } else {
        char* expr_type = visit_Expr_rval(node->expr, var_type);
        if (strcmp(max(var_type, expr_type), var_type) != 0)
            type_error(TRUE, "Cannot assign expression of type '%s' to variable '%s' of type '%s'",
                        expr_type, node->variable_access->variable->lexeme, var_type);
        struct AddrTypePair atp1;
        atp1.addr = node->expr->addr;
        atp1.type = node->expr->addr_type;
        widen(&atp1, expr_type, var_type);
        if (node->assignment_type->type != '=') {
            /*
             * This is a bit of a hack
             */
            struct SymTab_entry* temp = newtemp(expr_type);
            enum TokenType op_type;
            switch (node->assignment_type->lexeme[0]) {
                case '+':
                    op_type = '+';
                    break;
                case '-':
                    op_type = '-';
                    break;
                case '*':
                    op_type = '*';
                    break;
                case '/':
                    op_type = '/';
                    break;
                case '%':
                    op_type = '%';
                    break;
                case '^':
                    op_type = '^';
                    break;
                case '&':
                    op_type = '&';
                    break;
                case '|':
                    op_type = '|';
                    break;
                case '>':
                    op_type = SHR;
                    break;
                case '<':
                    op_type = SHL;
                    break;
            }
            append_triple(gen_binop(var_entry, VARIABLE, op_type, atp1.addr,atp1.type, temp), QUAD_BINOP, 1);
            append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN, 1);
        } else {
            append_triple(gen_assignment(var_entry, atp1.addr,atp1.type), QUAD_ASSIGN, 1);
        }

    }
}

void visit_IEEStmt(struct IEEStmt* node)
{
    if (node->_else != NULL) {
        struct BasicBlock** next = node->next;
        struct BasicBlock** prev_false = if_with_else(node->if_stmt, next, NULL); // ???
        for (int i = 0; i < node->n_elifs; i++)
            prev_false = if_with_else(node->elif_list[i], next, prev_false);
        push_Env(NULL);
        *(prev_false) = new_bb();
        node->_else->next = next;
        visit_CompStmt(node->_else);
        set_uncond_target(next);
        pop_Env();
    } else if (node->n_elifs == 0) {
        node->if_stmt->boolean->true = newlabel();
        node->if_stmt->boolean->false = node->if_stmt->body->next = node->next;
        visit_Expr_jump(node->if_stmt->boolean);
        push_Env(NULL);
        *(node->if_stmt->boolean->true) = new_bb();
        visit_CompStmt(node->if_stmt->body);
        set_uncond_target(node->next);
        pop_Env();
    } else {
        struct BasicBlock** next = node->next;
        struct BasicBlock** prev_false = if_with_else(node->if_stmt, next, NULL);
        int n_elifs_minus_1 = node->n_elifs-1;
        for (int i = 0; i < n_elifs_minus_1; i++)
            if_with_else(node->elif_list[i], next, prev_false);
        struct CondStmt* last_elif = node->elif_list[n_elifs_minus_1];

        last_elif->boolean->true = newlabel();
        last_elif->boolean->false = next;
        *(prev_false) = new_bb();
        visit_Expr_jump(last_elif->boolean);
        push_Env(NULL);
        *(last_elif->boolean->true) = new_bb();
        visit_CompStmt(last_elif->body);
        set_uncond_target(next);
        pop_Env();
    }
}

struct BasicBlock** if_with_else(struct CondStmt* node, struct BasicBlock** next, struct BasicBlock** prev_false)
{
    node->boolean->true = newlabel();
    node->boolean->false  = newlabel();
    node->body->next = next;
    if (prev_false != NULL)
        *prev_false = new_bb();

    visit_Expr_jump(node->boolean);
    *(node->boolean->true) = new_bb();
    push_Env(NULL);
    visit_CompStmt(node->body);
    pop_Env();
    set_uncond_target(next);
    return node->boolean->false;
}

void visit_WLoop(struct CondStmt* node)
{
    struct BasicBlock** begin = newlabel();
    set_uncond_target(begin);
    *begin = new_bb();
    node->boolean->true = newlabel();
    node->boolean->false = node->next;
    visit_Expr_jump(node->boolean);
    node->body->next = begin;
    push_Env(NULL);
    *(node->boolean->true) = new_bb();
    visit_CompStmt(node->body);
    set_uncond_target(begin);
    pop_Env();
}

void visit_FLoop(struct FLoop* node)
{
    push_Env(NULL);
    if (node->type == VARIABLE_DECLARATION)
        visit_VarDecl(node->init_stmt);
    else
        visit_AStmt(node->init_stmt);
    struct BasicBlock** begin = newlabel();
    set_uncond_target(begin);
    *begin = new_bb();
    node->boolean->true = newlabel();
    node->boolean->false = node->next;
    visit_Expr_jump(node->boolean);
    *(node->boolean->true) = new_bb();
    node->body->next = newlabel();

    visit_CompStmt(node->body);
    set_uncond_target(node->body->next);
    *(node->body->next) = new_bb();
    visit_AStmt(node->update_statement);
    set_uncond_target(begin);
    pop_Env();
}


void visit_ReturnStmt(struct Expr* node)
{

    if (!in_function)
        return_not_in_func_error();
    struct BasicBlock** ret = newlabel();
    set_uncond_target(ret);
    *ret = new_bb();
    char* type = visit_Expr_rval(node, function_type);
    struct AddrTypePair atp;
    atp.addr = node->addr;
    atp.type = node->addr_type;
    widen(&atp, type, function_type);
    append_triple(gen_return(atp.addr, atp.type), QUAD_RETURN, 0);
    set_uncond_target(epilogue);
    /*
     * Like with bools for constant, we generate basic blocks
     * even if it they are not reachable from any other basic block.
     * Unreachable code is then removed by walking the CFG.
     */
    new_bb();
}
