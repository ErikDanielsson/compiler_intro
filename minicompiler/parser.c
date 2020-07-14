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

/*
 * Stack machine for LR parsing. Uses a LALR parsing table and error mapping
 * generated by "table_generator.c", and produces a parse of the token stream
 * from the lexer.
 */

void lr_parser(char verbose)
/*
 * Uses variables rules, reduction_rules, n_pop_states, action table,
 * n_states, and goto_table defined in "table_generator.h"
 */
{
	if (verbose)
		printf("parsing...\n");
	int stack[STACK_SIZE];
	int* s_ptr = stack;
	struct Token* a = get_token();
	*s_ptr = 0;
	int action;
	while (1) {
		action = action_table[*s_ptr][a->type];
		if (verbose)
			printf("Stack depth %ld\n", s_ptr-stack);
		printf("action: %d, lexeme: %s\n", action, a->lexeme);
		if (action >= n_states) {
			printf("error in state %d on input %s\n", *s_ptr, a->lexeme);
			printf("Stack: ");
			for (int i = 0; i <=  s_ptr-stack; i++)
				printf("%d, ", *(stack+i));

			printf("\n");
			parser_error(strlen(a->lexeme), "", 0, a->line, a->column);

			free(a);
			return;
		} else if (action >= 0) {
			s_ptr++;
			*s_ptr = action;
			if (a->type != 0x04)
				free(a->lexeme);
			free(a);
			a = get_token();

		} else if (action == -1) {
			printf("parse done\n");
			free(a);
			return;
		} else {
			int r = reduction_rules[-(action+1)];
			s_ptr -= n_pop_states[-(action+1)];
			int tmp = *s_ptr;
			s_ptr++;
			*s_ptr = goto_table[r][tmp];
			if (verbose)
				printf("reduce by %s\n", rules[-(action+1)]);
		}
	}
}

void parser_error(int length, const char* expected, int fatal, int line, int column) {
	error("syntax error", length, expected, fatal, line, column);
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
