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

/*
 *  Intermediate code generation
 */

//struct IC intermediate_code;

FILE* IC_file_desc;
char* ic_filename = "IR_file.tmp";

int temp_num = 0;
#define MAX_LABEL_LENGTH 10
struct SymTab_entry* newtemp()
{
    char* temp = malloc(MAX_LABEL_LENGTH+1);
    sprintf(temp, "$%d", temp_num);
    enter_temp_var(temp);
    temp_num++;
    return get_curr_name_entry(temp);
}

int label_num = 0;
struct BasicBlock** newlabel()
{
    return malloc(sizeof(struct BasicBlock*));
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

void widen(struct AddrTypePair* a,  char* type1, char* type2)
{
    if (strcmp(type1, type2) == 0)
        return;
    check_type_defined(type1);
    check_type_defined(type2);

    char* caster = max(type1, type2);
    struct SymTab_entry* temp_entry = newtemp();
    append_triple(gen_conv(caster, a->addr, a->type, temp_entry), QUAD_CONV);
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
        struct AddrTypePair atp1;
        atp1.addr = node->expr->addr;
        atp1.type = node->expr->addr_type;
        widen(&atp1, var_type, expr_type);
        check_and_set_var(node);
        append_triple(gen_assignment(get_curr_name_entry(node->name->lexeme), atp1.addr, atp1.type), QUAD_ASSIGN);
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
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            char* true = newlabel();
            struct BasicBlock** next = newlabel();
            struct SymTab_entry* t = newtemp();
            //emit("if %s %s %s goto %s",
                //a1, node->binary_op->lexeme,
                //a2, true);
            //emit("%s = 0", t);
            //emit("goto %s", next);
            //emitlabel(true);
            //emit("%s = 1", t);
            //emitlabel(next);
            node->addr = t;
            node->addr_type = TEMPORARY;

            return "inontot";
        }
        case EXPR_AND: {
            char* true = newlabel();
            char* false = newlabel();
            struct BasicBlock** next = newlabel();

            node->left->true = "fall";
            node->left->false = false;
            node->right->true = true;
            node->right->false = "fall";

            visit_Expr_jump(node->left);
            visit_Expr_jump(node->right);
            //emitlabel(false);
            struct SymTab_entry* t = newtemp();
            //emit("%s = 0", t);
            //emit("goto %s", next);
            //emitlabel(true);
            //emit("%s = 1", t);
            //emitlabel(next);
            node->addr = t;
            node->addr_type = TEMPORARY;
            return "inontot";
        }
        case EXPR_OR: {
            char* true = newlabel();
            char* false = newlabel();
            struct BasicBlock** next = newlabel();
            node->left->false = "fall";
            node->left->true = true;
            node->right->true = "fall";
            node->right->false = false;

            visit_Expr_jump(node->left);
            visit_Expr_jump(node->right);
            //emitlabel(true);
            struct SymTab_entry* t = newtemp();
            //emit("%s = 1", t);
            //emit("goto %s", next);
            //emitlabel(false);
            //emit("%s = 0", t);
            //emitlabel(next);
            node->addr = t;
            node->addr_type = TEMPORARY;

            return "inontot";
        }
        case EXPR_NOT: {
            char* true = newlabel();
            struct BasicBlock** next = newlabel();
            printf("%s", next);
            node->expr->true = "fall";
            node->expr->false = true;
            visit_Expr_jump(node->expr);
            struct SymTab_entry* t = newtemp();
            //emit("%s = 0", t);
            //emit("goto %s", next);
            //emitlabel(true);
            //emit("%s = 1", t);
            //emitlabel(next);
            node->addr = t;
            node->addr_type = TEMPORARY;
            return "inontot";
        }

        case EXPR_BINOP: {
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            node->addr = newtemp();
            node->addr_type = TEMPORARY;
            append_triple(gen_binop(atp1.addr, atp1.type,
                            node->binary_op->type,
                            atp2.addr, atp2.type,
                            node->addr), QUAD_BINOP);
            return type;

        }
        case EXPR_UOP: {
            char* type = visit_Expr_rval(node->expr);
            node->addr = newtemp();
            append_triple(gen_uop(node->expr->addr, node->expr->addr_type,
                                node->unary_op->type, node->addr), QUAD_UOP);
            return type;
        }
        case EXPR_CONST: {
            switch (node->val->type) {

                case ICONST:
                    node->addr = malloc(12);
                    sprintf(node->addr, "%ld", node->val->i_val);
                    node->addr_type = CONSTANT;
                    return "inontot";
                case FCONST:
                    node->addr = malloc(11);
                    sprintf(node->addr, "%lf", node->val->f_val);
                    node->addr_type = CONSTANT;
                    return "fofloloatot";
                case SCONST:

                    node->addr = node->val->lexeme;
                    node->addr_type = CONSTANT;

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
            node->addr_type = FUNCTION;

            return type;
        }
        case EXPR_VARACC:
            node->addr = get_curr_name_entry(node->variable_access->variable->lexeme);
            node->addr_type = VARIABLE;

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
            /*
            if (strcmp(node->true, "fall") == 0) {
                if (strcmp(node->false, "fall") != 0)
                    //emit("if %s %s %s goto %s", a1,
                            relop_inv(node->binary_op->lexeme), a2,
                            node->false);
            } else if (strcmp(node->false, "fall") == 0) {
                //emit("if %s %s %s goto %s", a1,
                        node->binary_op->lexeme, a2,
                        node->true);
            } else {
                //emit("if %s %s %s goto %s", a1,
                        node->binary_op->lexeme, a2,
                        node->true);
                //emit("goto %s", node->false);
            }
            */

        }
        case EXPR_AND:
            printf("hej");
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
            char* type1 = visit_Expr_rval(node->left);
            char* type2 = visit_Expr_rval(node->right);
            char* type = max(type1, type2);
            struct AddrTypePair atp1;
            atp1.addr = node->left->addr;
            atp1.type = node->left->addr_type;
            widen(&atp1, type1, type);
            struct AddrTypePair atp2;
            atp2.addr = node->right->addr;
            atp2.type = node->right->addr_type;
            widen(&atp2, type1, type);
            // Is this really needed?? Is it used by a parent function??????
            struct SymTab_entry* temp_addr = newtemp();
            append_triple(gen_binop(atp1.addr, atp1.type,
                        node->binary_op->type,
                        atp2.addr, atp2.type,
                        temp_addr),
                        QUAD_BINOP);
            set_cond_and_targets(gen_cond(temp_addr, TEMPORARY, "!=", "0", CONSTANT), node->true, node->false);
            return;
        }
        case EXPR_UOP: {
            visit_Expr_rval(node->expr);
            /*
             * Since the truth value of a number doesn't depend on it's
             * sign, we can omit the instruction for sign change.
             */
             set_cond_and_targets(gen_cond(node->expr->addr, TEMPORARY,
                                "!=",
                                "0", CONSTANT),
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
        case EXPR_FUNCCALL:
            visit_FuncCall(node->function_call);
            set_cond_and_targets(gen_cond(node->function_call->addr, TEMPORARY,
                               "!=", "0", CONSTANT), node->true, node->false);
            return;
        case EXPR_VARACC:
            visit_VarAcc(node->variable_access);
            set_cond_and_targets(gen_cond(node->variable_access->addr, TEMPORARY,
                               "!=", "0", CONSTANT), node->true, node->false);

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
        char* arg_type = visit_Expr_rval(node->args[i]);
        struct AddrTypePair atp1;
        atp1.addr = node->args[i]->addr;
        atp1.type = node->args[i]->addr_type;
        widen(&atp1, decl->params[i]->type->lexeme, arg_type);
    }
    for (int i = 0; i < node->n_args; i++) {
        //emit("param %s", node->args[i]->addr);
    }
    struct SymTab_entry* temp = newtemp();
    //emit("%s = call %s",temp,  node->func->lexeme);
    node->addr = temp;
    node->addr_type = TEMPORARY;

    return decl->type->lexeme;
}

void visit_AStmt(struct AStmt* node)
{
    char* var_type = visit_VarAcc(node->variable_access);
    struct SymTab_entry* var_entry = node->variable_access->addr;
    if (node->assignment_type->type == SUFFIXOP) {
        struct SymTab_entry* temp = newtemp();
        switch (node->assignment_type->lexeme[0]) {
            case '+':
                append_triple(gen_binop(var_entry, VARIABLE, '+', "1", CONSTANT, temp), QUAD_BINOP);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN);
                break;
            case '-':
                append_triple(gen_binop(var_entry, VARIABLE, '-', "1", CONSTANT, temp), QUAD_BINOP);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN);
                break;
            case '*':
                append_triple(gen_binop(var_entry, VARIABLE, SHL, "1", CONSTANT, temp), QUAD_BINOP);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN);
                break;
            case '/':
                append_triple(gen_binop(var_entry, VARIABLE, SHR, "1", CONSTANT, temp), QUAD_BINOP);
                append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN);
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
        struct AddrTypePair atp1;
        atp1.addr = node->expr->addr;
        atp1.type = node->expr->addr_type;
        widen(&atp1, expr_type, var_type);
        if (node->assignment_type->type != '=') {
            /*
             * This is a bit of a hack
             */
            struct SymTab_entry* temp = newtemp();
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
            append_triple(gen_binop(var_entry, VARIABLE, op_type, atp1.addr,atp1.type, temp), QUAD_BINOP);
            append_triple(gen_assignment(var_entry, temp, TEMPORARY), QUAD_ASSIGN);
        } else {
            append_triple(gen_assignment(var_entry, atp1.addr,atp1.type), QUAD_ASSIGN);
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
        push_Env();
        *(prev_false) = new_bb();
        node->_else->next = next;
        visit_CompStmt(node->_else);
        set_uncond_target(next);
        pop_Env();
    } else if (node->n_elifs == 0) {
        node->if_stmt->boolean->true = newlabel();
        node->if_stmt->boolean->false = node->if_stmt->body->next = node->next;
        visit_Expr_jump(node->if_stmt->boolean);
        push_Env();
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
        push_Env();
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
    push_Env();
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
    push_Env();
    *(node->boolean->true) = new_bb();
    visit_CompStmt(node->body);
    set_uncond_target(begin);
    pop_Env();
}

void visit_FLoop(struct FLoop* node)
{
    push_Env();
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
    char* type = visit_Expr_rval(node);
    struct AddrTypePair atp1;
    atp1.addr = node->addr;
    atp1.type = node->addr_type;
    widen(&atp1, type, function_type);

}
