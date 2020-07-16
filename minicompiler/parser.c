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
				a = recovery_token;
				recovery_mode = FALSE;
			} else {
				a = get_token();
			}

		} else if (action == -1) {
			#if VERBOSE
			printf("parse done\n");
			print_tree((struct CompStmt*)(record_ptr->value));
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
	struct Record* record = malloc(sizeof(struct Record));
	record->type = type;
	printf("record type: %d\n", type);
	switch (type) {
		case COMPOUND_STATEMENT:
		{	printf("COMPOUND_STATEMENT\n");
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
			memcpy(*top, record, sizeof(struct Record));
			break;
		}
		case STATEMENT:
		{
			printf("STATEMENT\n");
			struct Stmt* tmp_node = malloc(sizeof(struct Stmt));
			free(((struct Token*)((*top)->value))->lexeme);
			free((*top)->value);
			(*top)--;
			tmp_node->statement_type = (*top)->type;
			if ((*top)->type == VARIABLE_DECLARATION) {
				printf("yey\n");
				tmp_node->variable_declaration = (struct VarDecl*)((*top)->value);

			} else {
				printf("type %d should be %d\n", (*top)->type, VARIABLE_DECLARATION);
			}
			record->value = tmp_node;
			memcpy(*top, record, sizeof(struct Record));
			break;
		}
		case VARIABLE_DECLARATION:
		{
			printf("VARIABLE_DECLARATION\n");
			struct VarDecl* tmp_node = malloc(sizeof(struct VarDecl));
			tmp_node->name = (struct Token*)((*top)->value);
			(*top)--;
			tmp_node->type = ((struct Type*)((*top)->value))->type;
			record->value = tmp_node;
			free((*top)->value);
			memcpy(*top, record, sizeof(struct Record));
			break;
		}
		/*
		case INDEX_EXPR:
			printf("INDEX_EXPR\n");
			*record_ptr = NULL;
			break;
		case FUNCTION_DECLARATION:
			printf("FUNCTION_DECLARATION\n");
			*record_ptr = malloc(sizeof(struct FuncDecl));
			break;
		case PARAMS:
			printf("PARAMS\n");
			*record_ptr = NULL;
			break;
		case PARAM_DECL:
			printf("PARAM_DECL\n");
			*record_ptr = NULL;
			break;*/
		case TYPE:
		{
			printf("TYPE\n");
			struct Type* tmp_node = malloc(sizeof(struct Type));
			tmp_node->type = (struct Token*)((*top)->value);
			tmp_node->n_brackets = 0;
			tmp_node->prior_expr = NULL;
			record->value = tmp_node;
			memcpy(*top, record, sizeof(struct Record));
			break;
		}
					/*	break;
		case VARIABLE_ACCESS:
			printf("VARIABLE_ACCESS\n");
			*record_ptr = malloc(sizeof(struct VarAcc));
			break;
		case EXPR:
			printf("EXPR\n");
			*record_ptr = malloc(sizeof(struct Expr));
			break;
		case ASSIGNMENT_STATEMENT:
			printf("ASSIGNMENT_STATEMENT\n");
			*record_ptr = malloc(sizeof(struct AStmt));
			break;
		case FUNCTION_CALL:
			printf("FUNCTION_CALL\n");
			*record_ptr = malloc(sizeof(struct FuncCall));
			break;
		case ARGS:
			printf("ARGS\n");
			*record_ptr = NULL;
			break;
		case IF_ELIF_ELSE_STATEMENT:
			printf("IF_ELIF_ELSE_STATEMENT\n");
			*record_ptr = malloc(sizeof(struct IIEStmt));
			break;
		case ELIF_LIST:
			printf("ELIF_LIST\n");
			*record_ptr = NULL;
			break;
		case IF_STATEMENT:
			printf("IF_STATEMENT\n");
			*record_ptr = malloc(sizeof(struct IfStmt));
			break;
		case ELIF_STATEMENT:
			printf("ELIF_STATEMENT\n");
			*record_ptr = NULL;
			break;
		case ELSE_STATEMENT:
			printf("ELSE_STATEMENT\n");
			*record_ptr = NULL;
			break;
		case WHILE_LOOP:
			printf("WHILE_LOOP\n");
			*record_ptr = malloc(sizeof(struct WLoop));
			break;
		case FOR_LOOP:
			printf("FOR_LOOP\n");
			*record_ptr = malloc(sizeof(struct FLoop));
			break;
		case B_EXPR:
			printf("B_EXPR\n");
			*record_ptr = malloc(sizeof(struct BExpr));
			break;
		case R_EXPR:
			printf("R_EXPR\n");
			*record_ptr = malloc(sizeof(struct RExpr));
			break;*/
		default:
			printf("default\n");
			break;

	}
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
