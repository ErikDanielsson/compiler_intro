#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"
#include "symbol_table.h"
#include "parser.h"
#include "table_generator.h"

#define STACK_SIZE 8192
#define TRUE 1
#define FALSE 0
#define VERBOSE 1
char grammar_error = FALSE;
char recovery_mode = FALSE;
/*
 * Stack machine for LR parsing. Uses a LALR parsing table and generated by
 * "table_generator.c", and produces a parse of the token stream from the lexer.
 */

void lr_parser(char verbose)
/*
 * Uses variables rules, reduction_rules, n_pop_states, action table,
 * n_states, and goto_table defined in "table_generator.h"
 */
{

    #if VERBOSE
    printf("parsing...\n");
    #endif
    struct Record tree_stack[STACK_SIZE];
    struct Record* record_ptr = tree_stack;
    int stack[STACK_SIZE];
    int* s_ptr = stack;
    struct Token* a = get_token();
    struct Token* recovery_token = NULL;
    *s_ptr = 0;
    record_ptr->value = NULL;
    record_ptr->type = -1;

    int action;
    parsing_loop:
    while (1) {
        action = action_table[*s_ptr][a->type];
        #if VERBOSE
        printf("Stack depth %ld, top: %d\n", s_ptr-stack, *s_ptr);
        printf("action: %d, lexeme: '%s', %x\n", action, a->lexeme, a->type);
        #endif

        if (action >= n_states) {
            if (a->type == 0x04)
                printf("error in state %d on input 'EOF'\n", *s_ptr);
            else
                printf("error in state %d on input '%s'\n", *s_ptr, a->lexeme);
            #if VERBOSE
            printf("Stack: ");
            for (int i = 0; i <=  s_ptr-stack; i++)
                printf("%d, ", *(stack+i));

            printf("\n");
            #endif
            int* row = action_table[*s_ptr];
            int len;
            if (a->lexeme == NULL)
                len = 0;
            else
                len = strlen(a->lexeme);
            if (row['}'] < n_states) {
                char* msg = malloc(sizeof(char)*(strlen("Expected '}' before ")+strlen(a->lexeme)+3));
                strcpy(msg, "Expected '}' before '");
                strcat(msg, a->lexeme);
                strcat(msg, "'");

                struct Token* tmp = inject_token('}');
                parser_error(len, msg, 0, a->line, a->column, a->column+1, '}');
                free(msg);
                grammar_error = TRUE;
                recovery_token = a;
                a = tmp;
                recovery_mode = TRUE;
                goto parsing_loop;
            } else if (row[';'] < n_states) {
                char* msg = malloc(sizeof(char)*(strlen("Expected ',' before ")+strlen(a->lexeme)+3));
                strcpy(msg, "Expected ';' before '");
                strcat(msg, a->lexeme);
                strcat(msg, "'");
                struct Token* tmp = inject_token(';');
                parser_error(len, msg, 0, a->line, a->column, -1, ';');
                free(msg);
                grammar_error = TRUE;
                recovery_token = a;
                a = tmp;
                recovery_mode = TRUE;

                goto parsing_loop;
            }
            free(a);
            parser_error(len, "", 0, a->line, a->column, 0, 0);
            return;
        } else if (action >= 0) {
            s_ptr++;
            #if VERBOSE
            printf("Push %d\n", action);
            #endif
            *s_ptr = action;
            record_ptr++;
            create_token_record(record_ptr, a);

            if (recovery_mode) {
                printf("IN RECORVERY\n");
                a = recovery_token;
                recovery_mode = FALSE;
            } else {
                a = get_token();
            }

        } else if (action == -1) {
            #if VERBOSE
            printf("parse done\n");
            //print_tree((struct CompStmt*)(record_ptr->value));
            #endif
            free(a);
            return;
        } else {
            enum NodeType r = reduction_rules[-(action+1)];
            int n_pop = n_pop_states[-(action+1)];
            s_ptr -= n_pop;
            int tmp = *s_ptr;
            s_ptr++;
            *s_ptr = goto_table[r][tmp];
            printf("before %d\n", record_ptr->type);

            create_node_record(&record_ptr, r, n_pop);
            printf("after %d\n", record_ptr->type);
            #if VERBOSE
            printf("reduce by %s\n", rules[-(action+1)]);
            #endif
        }
    }
}

void parser_error(int length, const char* expected,
          int fatal, int line, int column,
          int inject_symbol, char symbol) {
    /*
     * Calls error func implemented in "lexer.c"
     */
    error("syntax error", length, expected, fatal, line, column,
        inject_symbol, symbol);
}

struct Token* inject_token(enum TokenType type) {
    /*
     * Creates a token which is injected into the token stream,
     * to alleviate the damage caused by the user.
     */
    struct Token* imaginary_token = malloc(sizeof(struct Token));
    imaginary_token->type = type;
    imaginary_token->lexeme = NULL;
    return imaginary_token;
}

void create_node_record(struct Record** top, enum NodeType type, int n_pop) {
    /*
     * Creation of a node of AST (abstract syntax tree). When the parsers
     * determines a reduction, this function is called. By looking at the types
     * of symbols on the stack it determines a parse according to the grammar
     * rules (defined in "grammar"). Since the algorithm is stack based, it
     * works from right to left in the production, shifting symbols from the
     * stack into structs of the correct type. Some productions, such as
     * 'indices', generate lists, and while they would naturally produce a
     * linked list if left on their own, they are converted to arrays.
     */
    struct Record* record = malloc(sizeof(struct Record));
    record->type = type;
    printf("record type: %d\n", type);
    switch (type) {
        case COMPOUND_STATEMENT: {
            break;
            printf("COMPOUND_STATEMENT\n");
            struct CompStmt* tmp_node = malloc(sizeof(struct CompStmt));

            if (n_pop == 1) {
                tmp_node->n_statements = 1;
                tmp_node->statement_list = malloc(sizeof(struct Stmt*)*1);
                tmp_node->statement_list[0] = (struct Stmt*)((*top)->value);
            } else {
                printf("second level\n");
                struct Stmt* tmp_stmt = (struct Stmt*)(*top)->value;
                printf("lex name: %s\n", tmp_stmt->variable_declaration->name->lexeme);
                (*top)--;
                struct CompStmt* prior_compound = (struct CompStmt*)((*top)->value);
                int n_new_stmts = prior_compound->n_statements+1;
                tmp_node->n_statements = n_new_stmts;
                tmp_node->statement_list = malloc(sizeof(struct Stmt*)*n_new_stmts);
                tmp_node->statement_list[n_new_stmts-1] = tmp_stmt;
                memcpy(tmp_node->statement_list, prior_compound->statement_list, sizeof(struct Stmt*)*(n_new_stmts-1));
                free(prior_compound->statement_list);
                free(prior_compound);

            }
            record->value = tmp_node;
            break;
        }

        case STATEMENT: {
            break;
            printf("STATEMENT\n");
            struct Stmt* tmp_node = malloc(sizeof(struct Stmt));
            enum NodeType type = (*top)->type;
            if (type == TOKEN) {
                free(((struct Token*)((*top)->value))->lexeme);
                free((*top)->value);
                (*top)--;

                type = (*top)->type;
                tmp_node->statement_type = type;
                switch (type) {
                    case VARIABLE_DECLARATION:
                        tmp_node->variable_declaration = (struct VarDecl*)((*top)->value);
                    case ASSIGNMENT_STATEMENT:
                        tmp_node->assignment_statement = (struct AStmt*)((*top)->value);
                    case FUNCTION_CALL:
                        tmp_node->function_call = (struct FuncCall*)((*top)->value);
                    default:
                        printf("Something fishy in statement 1\n");
                }

            } else {
                tmp_node->statement_type = type;
                switch (type) {
                    case FUNCTION_DECLARATION:
                        tmp_node->function_declaration = (struct FuncDecl*)((*top)->value);
                        break;
                    case IF_ELIF_ELSE_STATEMENT:
                        tmp_node->if_elif_else_statement = (struct IEEStmt*)((*top)->value);
                        break;
                    case WHILE_LOOP:
                        tmp_node->while_loop = (struct WLoop*)((*top)->value);
                        break;
                    case FOR_LOOP:
                        tmp_node->for_loop = (struct FLoop*)((*top)->value);
                        break;
                    case SCOPE:
                        tmp_node->scope = (struct CompStmt*)((*top)->value);
                        break;
                    default:
                        printf("Something fishy is statement 2\n");
                }
            }
            record->value = tmp_node;
            break;
        }

        case VARIABLE_DECLARATION: {
            printf("VARIABLE_DECLARATION\n");
            struct VarDecl* tmp_node = malloc(sizeof(struct VarDecl));

            if ((*top)->type == EXPR) {
                tmp_node->expr = (struct Expr*)((*top)->value);

                (*top)--;
                struct Token* top_token = (struct Token*)((*top)->value);
                free(top_token->lexeme);
                free(top_token);
                (*top)--;
            } else {
                tmp_node->expr = NULL;
            }

            tmp_node->name = (struct Token*)((*top)->value);
            (*top)--;

            if ((*top)->type == INDICES) {
                struct Inds* ind= (struct Inds*)((*top)->value);
                int n_indices = ind->n_indices;
                tmp_node->n_indices = n_indices;
                tmp_node->indices = malloc(sizeof(struct Expr*)*n_indices);
                memcpy(tmp_node->indices, ind->indices, sizeof(struct Expr*)*n_indices);
                free(ind->indices);
                free(ind);
                (*top)--;
                printf("n indices: %d", n_indices);
                for (int i = 0; i < n_indices; i++) {
                        if (tmp_node->indices[i] == NULL) {
                                printf("Empty, ");
                        } else {
                                printf("Non empty, ");
                        }
                }
                printf("\n");
            } else {
                tmp_node->n_indices = 0;
                tmp_node->indices = NULL;
            }

            tmp_node->type = (struct Token*)((*top)->value);

            record->value = tmp_node;
            break;
        }

        case FUNCTION_DECLARATION: {
            printf("FUNCTION_DECLARATION\n");
            struct FuncDecl* tmp_node = malloc(sizeof(struct FuncDecl));

            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->body = (struct CompStmt*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            if (n_pop == 10) {
                //UNPACK PARAMS
                tmp_node->params;
                (*top)--;
            }

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->name = (struct Token*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);

            record->value = tmp_node;
            break;
        }

        case PARAMS: {
            printf("PARAMS\n");
            struct Params* tmp_node = malloc(sizeof(struct Params));

            struct VarDecl* next_decl = (struct VarDecl*)((*top)->value);
            (*top)--;

            if (n_pop == 3) {
                struct Token* top_token = (struct Token*)((*top)->value);
                free(top_token->lexeme);
                free(top_token);
                (*top)--;

                struct Params* prior_params = (struct Params*)((*top)->value);
                int n_params = prior_params->n_params;
                tmp_node->n_params = n_params+1;
                tmp_node->params = malloc(sizeof(struct VarDecl*)*(n_params+1));
                tmp_node->params[n_params] = next_decl;
                memcpy(tmp_node->params, prior_params->params, sizeof(struct Vardecl*)*n_params);
                free(prior_params->params);
                free(prior_params);
            } else {
                tmp_node->n_params = 1;
                tmp_node->params = malloc(sizeof(struct VarDecl*));
                tmp_node->params[0] = next_decl;
            }

            record->value = tmp_node;
            break;
        }

        case INDICES: {
            printf("INDICES\n");
            struct Inds* tmp_node = malloc(sizeof(struct Inds));
            struct Token* top_token = NULL;
            struct Expr* expr = NULL;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            if ((*top)->type == EXPR) {
                printf("HEJ\n");
                expr = (struct Expr*)((*top)->value);
                printf("HEJ\n");
                (*top)--;
                printf("HEJ\n");
                // To make sure the next 'if' evaluated correctly:
                n_pop--;
                printf("HEJ\n");
            }
            printf("%d =? %d\n", (*top)->type, TOKEN);
            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            printf("HEJ\n");
            if (n_pop == 3 ) {
                printf("n pop 3\n");
                (*top)--;
                struct Inds* prior_node = (struct Inds*)((*top)->value);
                int n_expr = prior_node->n_indices;

                printf("n pop 3\n");
                tmp_node->n_indices = n_expr+1;
                tmp_node->indices = malloc(sizeof(struct Expr*)*(n_expr+1));
                tmp_node->indices[n_expr] = expr;
                memcpy(tmp_node->indices, prior_node->indices, sizeof(struct Expr*)*n_expr);

                free(prior_node->indices);
                free(prior_node);

            } else {
                tmp_node->n_indices = 1;
                tmp_node->indices = malloc(sizeof(struct Expr*));
                tmp_node->indices[0] = expr;
            }
            record->value = tmp_node;
            break;
        }

        case VARIABLE_ACCESS: {
            printf("VARIABLE_ACCESS\n");
            struct VarAcc* tmp_node = malloc(sizeof(struct VarAcc));
            if ((*top)->type == INDICES) {
                struct Inds* ind= (struct Inds*)((*top)->value);
                int n_indices = ind->n_indices;
                tmp_node->n_indices = n_indices;
                tmp_node->indices = malloc(sizeof(struct Expr*)*n_indices);
                memcpy(tmp_node->indices, ind->indices, sizeof(struct Expr*)*n_indices);
                free(ind->indices);
                free(ind);
                (*top)--;
            }
            tmp_node->variable = (struct Token*)((*top)->value);
            record->value = tmp_node;
            break;
        }

        case EXPR: {
            printf("EXPR\n");

            struct Token* top_token = NULL;
            switch (n_pop) {
                case 1: {
                    struct Expr* tmp_node = malloc(sizeof(struct Expr));
                    switch ((*top)->type) {
                        case TOKEN:
                            tmp_node->type = CONST;
                            tmp_node->val = (struct Token*)((*top)->value);
                            break;
                        case FUNCTION_CALL:
                            tmp_node->type = FUNCCALL;
                            tmp_node->function_call = (struct FuncCall*)((*top)->value);
                            break;
                        case VARIABLE_ACCESS:
                            tmp_node->type = VARACC;
                            tmp_node->variable_access = (struct VarAcc*)((*top)->value);
                            break;
                        default:
                            printf("Something fishy in expr\n");
                    }
                    printf("broke correcly\n");
                    record->value = tmp_node;
                    break;
                }
                case 2: {
                    struct Expr* tmp_node = malloc(sizeof(struct Expr));
                    tmp_node->type = UOP;

                    tmp_node->unary_op = (struct Token*)((*top)->value);
                    (*top)--;

                    tmp_node->expr = (struct Expr*)((*top)->value);

                    record->value = tmp_node;
                    break;
                }
                case 3:
                if ((*top)->type == TOKEN) {
                    top_token = (struct Token*)((*top)->value);
                    free(top_token->lexeme);
                    free(top_token);
                    (*top)--;
                    /*
                     * Since paranthesis are only for precedence,
                     * we can put the expression back on the stack
                     */
                    record->value = (*top)->value;
                    (*top)--;
                    top_token = (struct Token*)((*top)->value);
                    free(top_token->lexeme);
                    free(top_token);

                } else {
                    struct Expr* tmp_node = malloc(sizeof(struct Expr));
                    tmp_node->type = BINOP;

                    tmp_node->right = (struct Expr*)((*top)->value);
                    (*top)--;

                    tmp_node->binary_op = (struct Token*)((*top)->value);
                    (*top)--;

                    tmp_node->left = (struct Expr*)((*top)->value);
                    record->value = tmp_node;
                }
                break;
            }
            break;
        }

        case ASSIGNMENT_STATEMENT: {
            printf("ASSIGNMENT_STATEMENT\n");
            struct AStmt* tmp_node = malloc(sizeof(struct AStmt));
            if ((*top)->type == TOKEN) {
                tmp_node->assignment_type = (struct Token*)((*top)->value);
                tmp_node->expr = NULL;

            } else {
                tmp_node->expr = (struct Expr*)((*top)->value);
                (*top)--;
                tmp_node->assignment_type = (struct Token*)((*top)->value);
            }

            (*top)--;
            tmp_node->variable_access = (struct VarAcc*)((*top)->value);
            record->value = tmp_node;
            break;
        }

        case FUNCTION_CALL: {
            printf("FUNCTION_CALL\n");
            struct FuncCall* tmp_node = malloc(sizeof(struct FuncCall));
            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;
            if ((*top)->type == ARGS) {


            } else {
                tmp_node->n_args = 0;
                tmp_node->args = NULL;
            }
            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->func = (struct Token*)((*top)->value);
            record->value = tmp_node;
            break;
        }

        case ARGS: {
            printf("ARGS\n");
            struct Args* tmp_node = malloc(sizeof(struct Args));
            struct Expr* next_arg = (struct Expr*)((*top)->value);
            if (n_pop == 3) {
                (*top)--;

                struct Token* top_token = (struct Token*)((*top)->value);
                free(top_token->lexeme);
                free(top_token);
                (*top)--;

                struct Args* prior_args = (struct Args*)((*top)->value);
                int n_args = prior_args->n_args;
                tmp_node->n_args = n_args+1;
                tmp_node->args = malloc(sizeof(struct Expr*)*(n_args+1));
                tmp_node->args[n_args] = next_arg;
                memcpy(tmp_node->args, prior_args->args, sizeof(struct Expr*)*n_args);
                free(prior_args->args);
                free(prior_args);
            } else {
                tmp_node->n_args = 1;
                tmp_node->args = malloc(sizeof(struct Expr*));
                tmp_node->args[0] = next_arg;
            }
            record->value = tmp_node;
            break;
        }

        case IF_ELIF_ELSE_STATEMENT: {
            printf("IF_ELIF_ELSE_STATEMENT\n");
            struct IEEStmt* tmp_node = malloc(sizeof(struct IEEStmt));
            if (n_pop == 1) {
                tmp_node->n_elifs = 0;
                tmp_node->elif_list = NULL;
                tmp_node->_else = NULL;
            } else {
                struct EList* elif_list = (struct EList*)((*top)->value);
                int n_elifs = elif_list->n_elifs;
                tmp_node->n_elifs = n_elifs;
                tmp_node->elif_list = malloc(sizeof(struct CondStmt*)*n_elifs);
                tmp_node->_else = elif_list->_else;
                memcpy(tmp_node->elif_list, elif_list->elif_list, sizeof(sizeof(struct CondStmt*)*n_elifs));
                free(elif_list->elif_list);
                free(elif_list);
                (*top)--;
            }
            tmp_node->if_stmt = (struct CondStmt*)((*top)->value);
            record->value = tmp_node;
            break;
        }

        case ELIF_LIST:{
            printf("ELIF_LIST\n");
            struct EList* tmp_node = malloc(sizeof(struct EList));
            if (n_pop == 2) {
                struct EList* prior_list = (struct EList*)((*top)->value);
                int n_elifs = prior_list->n_elifs;
                if (n_elifs != 0) {
                    tmp_node->n_elifs = n_elifs+1;
                    tmp_node->elif_list = malloc(sizeof(struct CondStmt*)*(n_elifs+1));
                    memcpy(tmp_node->elif_list+1, prior_list->elif_list, sizeof(struct CondStmt*)*n_elifs);
                    free(prior_list->elif_list);
                } else {
                    tmp_node->n_elifs = 1;
                    tmp_node->elif_list = malloc(sizeof(struct CondStmt*));
                }
                tmp_node->_else = prior_list->_else;
                free(prior_list);
                (*top)--;

                tmp_node->elif_list[0] = (struct CondStmt*)((*top)->value);

            } else {
                if ((*top)->type == ELIF_STATEMENT) {
                    tmp_node->n_elifs = 1;
                    tmp_node->elif_list = malloc(sizeof(struct CondStmt*));
                    tmp_node->elif_list[0] = (struct CondStmt*)((*top)->value);
                    tmp_node->_else = NULL;
                } else {
                    tmp_node->n_elifs = 0;
                    tmp_node->elif_list = NULL;
                    tmp_node->_else = (struct CompStmt*)((*top)->value);
                }
            }
            record->value = tmp_node;
            break;
        }

        /*
         * 'if', 'elif' and 'while' have the same structure
         */
        case IF_STATEMENT:
        case ELIF_STATEMENT:
        case WHILE_LOOP: {
            printf("IF_STATEMENT\n");
            struct CondStmt* tmp_node = malloc(sizeof(struct CondStmt));
            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->body = (struct CompStmt*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->boolean = (struct BExpr*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);

            record->value = tmp_node;
            break;
        }

        case ELSE_STATEMENT: {
            printf("ELSE_STATEMENT\n");
            struct CompStmt* tmp_node = malloc(sizeof(struct CompStmt));
            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node = (struct CompStmt*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);

            record->value = tmp_node;
            break;
        }

        case FOR_LOOP: {
            printf("FOR_LOOP\n");
            struct FLoop* tmp_node = malloc(sizeof(struct FLoop));
            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->body = (struct CompStmt*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->update_statement = (struct AStmt*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            tmp_node->boolean = (struct BExpr*)((*top)->value);
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            if ((*top)->type == VARIABLE_DECLARATION) {
                tmp_node->type = VARIABLE_DECLARATION;
                tmp_node->variable_declaration = (struct VarDecl*)((*top)->value);
            } else {
                tmp_node->type = ASSIGNMENT_STATEMENT;
                tmp_node->assignment_statement = (struct AStmt*)((*top)->value);
            }
            (*top)--;

            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);

            record->value = tmp_node;
            break;
        }

        case B_EXPR: {
            printf("B_EXPR\n");
            struct Token* top_token = NULL;
            switch (n_pop) {
                case 1:{
                    struct BExpr* tmp_node = malloc(sizeof(struct BExpr));
                    tmp_node->type = CONST;
                    tmp_node->r_expr = (struct RExpr*)((*top)->value);

                    record->value = tmp_node;
                    break;
                }
                case 3:
                    if ((*top)->type == TOKEN) {
                        top_token = (struct Token*)((*top)->value);
                        free(top_token->lexeme);
                        free(top_token);
                        (*top)--;

                        record->value = (*top)->value;
                        (*top)--;

                        top_token = (struct Token*)((*top)->value);
                        free(top_token->lexeme);
                        free(top_token);
                    } else {
                        struct BExpr* tmp_node = malloc(sizeof(struct BExpr));
                        tmp_node->right = (struct BExpr*)((*top)->value);
                        (*top)--;

                        top_token = (struct Token*)((*top)->value);
                        free(top_token->lexeme);
                        free(top_token);
                        (*top)--;

                        tmp_node->left = (struct BExpr*)((*top)->value);

                        record->value = tmp_node;
                    }
                    break;
                case 5: {
                    struct BExpr* tmp_node = malloc(sizeof(struct BExpr));
                    top_token = (struct Token*)((*top)->value);
                    free(top_token->lexeme);
                    free(top_token);
                    (*top)--;

                    tmp_node->right = (struct BExpr*)((*top)->value);
                    (*top)--;

                    top_token = (struct Token*)((*top)->value);
                    free(top_token->lexeme);
                    free(top_token);
                    (*top)--;

                    tmp_node->left = (struct BExpr*)((*top)->value);
                    (*top)--;

                    top_token = (struct Token*)((*top)->value);
                    free(top_token->lexeme);
                    free(top_token);

                    record->value = tmp_node;
                    break;
                }

            }
            break;
        }

        case R_EXPR:{
            printf("R_EXPR\n");
            struct RExpr* tmp_node = malloc(sizeof(struct RExpr));
            if (n_pop == 3) {
                tmp_node->type = BINOP;
                tmp_node->right = (struct Expr*)((*top)->value);
                (*top)--;
                tmp_node->operator = (struct Token*)((*top)->value);
                (*top)--;
                tmp_node->left = (struct Expr*)((*top)->value);

            } else {
                tmp_node->type = BINOP;
                tmp_node->expr = (struct Expr*)((*top)->value);
            }
            record->value = tmp_node;
            break;
        }

        case SCOPE:{
            printf("SCOPE\n");
            struct Token* top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            (*top)--;

            record->value = (*top)->value;
            (*top)--;
            top_token = (struct Token*)((*top)->value);
            free(top_token->lexeme);
            free(top_token);
            break;
        }

        default:
            printf("someone fucked up\n");
            break;

    }
    memcpy(*top, record, sizeof(struct Record));
    free(record);
}

void create_token_record(struct Record* record_ptr, struct Token* token) {
    record_ptr->type = TOKEN;
    record_ptr->value = token;
}

void print_tree(struct CompStmt* tree) {
    printf("compound statement\n");
    printf("n: %d\n", tree->n_statements);
    for (int i = 0; i < tree->n_statements; i++) {
        printf("STATEMENT\n");
        struct Stmt* stmt = tree->statement_list[i];


        if (stmt->statement_type == VARIABLE_DECLARATION) {
            printf("variable_declaration\n");
            struct VarDecl* var_decl = stmt->variable_declaration;
            free(stmt);
            printf("type: %s name: %s\n", var_decl->type->lexeme, var_decl->name->lexeme);
            free(var_decl->type->lexeme);
            free(var_decl->type);
            free(var_decl->name->lexeme);
            free(var_decl->name);
            free(var_decl);
        }
        else {
            printf("type: %d", stmt->statement_type);
        }
    }
    free(tree);
    free(tree->statement_list);
}

int main(int argc, const char** argv) {
    const char* table_file = "parsing_table.txt";
    filename = argv[1];
    file_desc = open(filename, O_RDONLY);
    init_lexer();
    generate_parse_table(table_file);
    lr_parser(1);
    destroy_parse_table();
    SymTab_destroy(symbol_table);
    close(file_desc);

}
