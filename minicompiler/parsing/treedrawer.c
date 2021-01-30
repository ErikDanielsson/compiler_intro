#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "treedrawer.h"

/*
 * Converts AST into dot format which is the compile to png.
 */

char* file_name;
FILE* tree_file_desc;
char* get_identifier();
void write_new_node(char* name, char* label);
void write_new_edge(char* start, char* end);

void draw_CompStmt(struct CompStmt* node, char* name);
void draw_Stmt(struct Stmt* node, char* name);
void draw_VarDecl(struct VarDecl* node, char* name);
void draw_FuncDecl(struct FuncDecl* node, char* name);
void draw_VarAcc(struct VarAcc* node, char* name);
void draw_Expr(struct Expr* node, char* name);
void draw_AStmt(struct AStmt* node, char* name);
void draw_FuncCall(struct FuncCall* node, char* name);
void draw_IEEStmt(struct IEEStmt* node, char* name);
void draw_CondStmt(struct CondStmt* node, char* name);
void draw_WLoop(struct CondStmt* node, char* name);
void draw_FLoop(struct FLoop* node, char* name);

void treedrawer_init(char* basename, struct CompStmt* root)
{
    int len = strlen(basename);
    char file_name[len+4];
    sprintf(file_name, "%s.gv", basename);
    
    tree_file_desc = fopen(file_name, "w");
    fprintf(tree_file_desc, "graph {\n");
    char* root_name = get_identifier();
    write_new_node(root_name, "root");
    draw_CompStmt(root, root_name);
    fprintf(tree_file_desc, "}\n");
    fclose(tree_file_desc);
    len *= 2;
    len += strlen("/bin/dot -Tpng .png -o .gv") + 1;
    char cmd_name[len];
    sprintf(cmd_name, "/bin/dot -Tpng %s.gv -o %s.png", basename, basename);
    system(cmd_name);
    sprintf(cmd_name, "/bin/rm %s.gv", basename);
    system(cmd_name);
}

long id_counter = 0;
char* get_identifier()
{
    long local_counter = id_counter;
    int diff = (int) ('Z' - 'A');
    char* str_buff = malloc(sizeof(char)*256);
    int i = 0;
    do {
        str_buff[i] = local_counter % diff + 'A';
        local_counter /= diff;
        i++;
    } while (local_counter > 0);
    id_counter++;
    str_buff[i] = 0;
    return str_buff;
}

void write_new_node(char* name, char* label)
{
    fprintf(tree_file_desc, "%s [label = \"%s\"]\n", name, label);
}

void write_new_edge(char* start, char* end)
{
    fprintf(tree_file_desc, "%s -- %s\n", start, end);
}


void draw_CompStmt(struct CompStmt* node, char* name) 
{
    char* node_name = get_identifier();
    write_new_node(node_name, "Compound statement");
    write_new_edge(name, node_name);
    for (int i = 0; i < node->n_statements; i++) {
        draw_Stmt(node->statement_list[i], node_name);
    }
}

void draw_Stmt(struct Stmt* node, char* name)
{
    
    switch (node->statement_type) {
            case VARIABLE_DECLARATION:
                draw_VarDecl(node->stmt, name);
                return;
            case FUNCTION_DECLARATION:
                draw_FuncDecl(node->stmt, name);
                return;
            case STRUCT_DECLARATION:
                //draw_StructDecl(node->stmt, name);
                return;
            case ASSIGNMENT_STATEMENT:
                draw_AStmt(node->stmt, name);
                return;
            case FUNCTION_CALL:
                draw_FuncCall(node->stmt, name);
                return;
            case IF_ELIF_ELSE_STATEMENT:
                draw_IEEStmt(node->stmt, name);
                return;
            case WHILE_LOOP:
                draw_WLoop(node->stmt, name);
                return;
            case FOR_LOOP:
                draw_FLoop(node->stmt, name);
                return;
            case SCOPE:
                draw_CompStmt(node->stmt, name);
                return;
            case RETURN_STATEMENT:
                //draw_ReturnStmt(node->stmt, name);
            default:
                break;
    }
}
void draw_VarDecl(struct VarDecl* node, char* name)
{
    char* node_name = get_identifier();
    char* str = get_token_str(node->name);
    char buff[strlen(str) + 1 + strlen("Decl: ") + 2];
    sprintf(buff, "Decl: '%s'", str);
    write_new_node(node_name, buff);
    write_new_edge(name, node_name);
    if (node->expr != NULL)
        draw_Expr(node->expr, node_name);

}
void draw_FuncDecl(struct FuncDecl* node, char* name)
{
    char* node_name = get_identifier();
    write_new_node(node_name, "FuncDecl");
    write_new_edge(name, node_name);
    char* param_name = get_identifier();
    write_new_node(param_name, "Params");
    write_new_edge(node_name, param_name);
    for (int i = 0 ; i < node->n_params; i++)
        draw_VarDecl(node->params[i], param_name);
    draw_CompStmt(node->body, node_name);
}
void draw_VarAcc(struct VarAcc* node, char* name)
{
    char* node_name = get_identifier();
    char* str = get_token_str(node->variable);
    char buff[strlen(str) + 3];
    sprintf(buff, "'%s'", str);
    write_new_node(node_name, buff);
    write_new_edge(name, node_name);
    if (node->next != NULL)
        draw_VarAcc(node->next, node_name);
}
void draw_Expr(struct Expr* node, char* name)
{
    char* node_name = get_identifier();
    switch (node->type) {
        case EXPR_BINOP:
        case EXPR_RELOP:
        case EXPR_AND:
        case EXPR_OR:
            write_new_node(node_name, get_token_str(node->binary_op));
            draw_Expr(node->left, node_name);
            draw_Expr(node->right, node_name);
            break;
        case EXPR_NOT:
        case EXPR_UOP:
            write_new_node(node_name, get_token_str(node->unary_op));
            draw_Expr(node->expr, node_name);
            break;
        case EXPR_CONST:
            write_new_node(node_name, get_token_str(node->val));
            break;
        case EXPR_FUNCCALL:
            draw_FuncCall(node->function_call, name);
            return;
        case EXPR_VARACC:
            draw_VarAcc(node->variable_access, name);
            return;
        case EXPR_CAST: {
            char* str = get_token_str(node->unary_op);
            char buff[strlen(str) + 1 + strlen("Cast: ")];
            sprintf(buff, "Cast: %s", str);
            write_new_node(node_name, buff);
            draw_Expr(node->expr, node_name);
            break;
        }
    }
    write_new_edge(name, node_name);
}
void draw_AStmt(struct AStmt* node, char* name)
{
    char* node_name = get_identifier();
    write_new_node(node_name, get_token_str(node->assignment_type));
    write_new_edge(name, node_name);
    draw_VarAcc(node->variable_access, node_name);
    if (node->assignment_type->type != SUFFIXOP) 
        draw_Expr(node->expr, node_name);
}
void draw_FuncCall(struct FuncCall* node, char* name)
{
    char* node_name = get_identifier();
    char* args_name = get_identifier();
    char* func_name = get_token_str(node->func);
    char buff[strlen(func_name) + 1 + strlen("Call:  ")];
    sprintf(buff, "Call: %s", func_name);
    write_new_node(node_name, buff);
    write_new_node(args_name, "args");
    write_new_edge(name, node_name);
    write_new_edge(node_name, args_name);
    for (int i = 0; i < node->n_args; i++) 
        draw_Expr(node->args[i], args_name);
}
void draw_IEEStmt(struct IEEStmt* node, char* name)
{
    char* if_name = get_identifier();
    char* then_name = get_identifier();
    char* else_name;
    write_new_node(if_name, "If");
    write_new_edge(name, if_name);
    draw_Expr(node->if_stmt->boolean, if_name);
    write_new_node(then_name, "then");
    write_new_edge(if_name, then_name);
    draw_CompStmt(node->if_stmt->body, then_name);
    int i = 0; 
    for (int i = 0; i < node->n_elifs; i++) {
        else_name = get_identifier();
        write_new_node(else_name, "else");
        write_new_edge(if_name, else_name);
        if_name = get_identifier();
        then_name = get_identifier();
        write_new_node(if_name, "If");
        write_new_edge(else_name, if_name);
        draw_Expr(node->elif_list[i]->boolean, if_name);
        write_new_node(then_name, "then");
        write_new_edge(if_name, then_name);
        draw_CompStmt(node->if_stmt->body, then_name);
    } 
    if (node->_else != NULL) {
        else_name = get_identifier();
        write_new_node(else_name, "else");
        write_new_edge(if_name, else_name);
        draw_CompStmt(node->_else, else_name);
    }
}
void draw_WLoop(struct CondStmt* node, char* name)
{
    char* node_name = get_identifier();
    write_new_node(node_name, "While");
    write_new_edge(name, node_name);
    draw_Expr(node->boolean, node_name);
    draw_CompStmt(node->body, node_name);
}
void draw_FLoop(struct FLoop* node, char* name)
{
    char* node_name = get_identifier();
    char* init_name = get_identifier();
    char* bool_name = get_identifier();
    char* update_name = get_identifier();
    write_new_node(node_name, "For");
    write_new_edge(name, node_name);
    write_new_node(init_name, "init");
    write_new_edge(init_name, node_name);
    if (node->type == VARIABLE_DECLARATION)
        draw_VarDecl(node->init_stmt, init_name);
    else 
        draw_AStmt(node->init_stmt, init_name);
    write_new_node(bool_name, "while");
    write_new_edge(node_name, bool_name);
    draw_Expr(node->boolean, bool_name);
    write_new_node(update_name, "update");
    write_new_edge(node_name, update_name);
    draw_AStmt(node->update_statement, update_name);

    draw_CompStmt(node->body, node_name);
}
