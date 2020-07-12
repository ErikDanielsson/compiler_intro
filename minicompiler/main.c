#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"
#include "symbol_table.h"

int main(int argc, char** argv) {
	file_desc = open(argv[1], O_RDONLY);
	init_lexer();
	close(file_desc);
	filename = argv[1];
	struct Token* token;
	token = get_token();
	while (token->type != 0x04) {
		switch (token->type) {
			case ID:
				printf("ID: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case ICONST:
				printf("ICONST: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case FCONST:
				printf("FCONST: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case SCONST:
				printf("SCONST: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case UTILIZING:
				printf("UTILIZING: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case PROGRAM:
				printf("PROGRAM: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case FTYPE:
				printf("FTYPE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case ITYPE:
				printf("ITYPE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case NAND:
				printf("NAND: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case IF:
				printf("IF: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case ELIF:
				printf("ELIF: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case ELSE:
				printf("ELSE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case WHILE:
				printf("WHILE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case FOR:
				printf("FOR: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case DEFINE:
				printf("DEFINE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case RETURN:
				printf("RETURN: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case INPUT:
				printf("INPUT: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case OUTPUT:
				printf("OUTPUT: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;

			case ASSIGN:
				printf("ASSIGN: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case RELOP:
				printf("RELOP: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case SUFFIXOP:
				printf("SUFFIXOP: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case _EOF:
				printf("EOF\n");
				break;
			default:
				printf("%c: \'%s\' at line: %d, column: %d\n",
					token->type, token->lexeme, token->line, token->column);
				break;

		}

		if (token->type != 0x04)
			free(token->lexeme);
		free(token);
		token = NULL;
		token = get_token();

	}
	if (argc > 2)
		printf("entered: %s, closest: %s\n", argv[2], closest_key(symbol_table, argv[2]));
	SymTab_dump(symbol_table);

	SymTab_destroy(symbol_table);
	return error_flag;
}
