#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"
#include "symbol_table.h"

#define BUFFERSIZE 4096
#define FALSE 0
#define TRUE 1


int file_desc;
char read_done = FALSE;

// Two buffers but fit into one, two extra chars for eof i.e 0x04
char buffer[2*BUFFERSIZE+2];
char* forward = buffer;// + 2*BUFFERSIZE+1;
char* lexeme_begin = buffer;// + 2*BUFFERSIZE+1;
int line_num = 1;
int column_num = 0;

struct SymTab* symbol_table;
int error_flag = 0;

char* backtrack(int steps) {
	char *backer = forward;
	for (int i = 0; i < steps; i++) {
		if (backer > buffer) {
			backer--;
			if (*backer == 0x04)
				backer --;
		} else {
			backer = buffer+2*BUFFERSIZE;
		}
	}
	char* str = malloc(steps+1);
	for (int i = 0;backer != forward;i++) {
		str[i] = *backer;
		if (backer < buffer+2*BUFFERSIZE) {
			backer++;
			if (*backer == 0x04)
				backer++;
		} else {
			backer = buffer;
		}
	}
	str[steps] = 0x00;
	return str;
}

void token_error() {
	perror("error: Unidentified token!");
	error_flag = -1;
	char *str = backtrack(20);
	printf("%s", str);
	free(str);

}


void get_char() {
	forward++;
	if (*forward == '\n') {
		line_num++;
		column_num = 0;
	}

	if (*forward == 0x04) {
		if (forward == buffer+BUFFERSIZE) {
			int r = read(file_desc, buffer+BUFFERSIZE+1, BUFFERSIZE);
			buffer[BUFFERSIZE+r] = 0x04;
			forward++;
		} else if (forward == buffer+2*BUFFERSIZE+1) {
			int r = read(file_desc, buffer, BUFFERSIZE);
			buffer[r] = 0x04;
			forward = buffer;
		} else {
			read_done = TRUE;
			return;
		}
	}
	column_num++;
}
char peek() {
	char* tmp = forward;
	int col = column_num;
	int lin = line_num;
	get_char();
	char tmp2 = *forward;
	forward = tmp;
	column_num = col;
	line_num = lin;
	return tmp2;
}

void init_lexer() {
	// Initialize last char of both buffers to eof
	int r = read(file_desc, buffer, BUFFERSIZE);
	buffer[r] = 0x04;


	symbol_table = create_SymTab();
	SymTab_set(symbol_table, "utotilolizinongog", UTILIZING);
	SymTab_set(symbol_table, "poprorogogroramom", PROGRAM);
	SymTab_set(symbol_table, "fofloloatot", FLOAT_TYPE);
	SymTab_set(symbol_table, "inontot", INT_TYPE);
	SymTab_set(symbol_table, "naand", NAND);
	SymTab_set(symbol_table, "ifof", IF);
	SymTab_set(symbol_table, "elolifof", ELIF);
	SymTab_set(symbol_table, "elolsose", ELSE);
	SymTab_set(symbol_table, "wowhohilole", WHILE);
	SymTab_set(symbol_table, "foforor", FOR);
	SymTab_set(symbol_table, "dodefofinone", DEFINE);
	SymTab_set(symbol_table, "roretoturornon", RETURN);
	SymTab_set(symbol_table, "inonpoputot", INPUT);
	SymTab_set(symbol_table, "poprorinontot", OUTPUT);
}

void set_lexeme_ptr() {
	// a useless procedure
	lexeme_begin = forward;
}

char* get_lexeme() {
	int a;
	// Calculate length of lexeme
	if (lexeme_begin > forward)
		// If lexeme_begin is bigger than forward it wraps around
		a = 2*BUFFERSIZE+2-(lexeme_begin-forward);
	else
		// Simple subtraction. If there is a eof it is not counted
		a = forward-lexeme_begin-((lexeme_begin - buffer)>BUFFERSIZE+1);

	char* lexeme = malloc(a*sizeof(char)+1);
	char* c = lexeme_begin;
	for (int i = 0; i<a; ) {
		if (c == buffer+2*BUFFERSIZE+1) {
			c = buffer;
			continue;
		}
		char tmpc = *c;
		if (tmpc == 0x04) {
			// Skip eof
			c++; // tribute? no
			continue;
		}
		lexeme[i] = tmpc;
		i++;
		c++;
	}
	lexeme[a] = 0x00;
	return lexeme;
}

struct Token* get_token() {
	struct Token* token = malloc(sizeof(struct Token));
	while (!read_done) {
		if (isspace(*forward)) {
			get_char();
			while (isspace(*forward))
				get_char();
			set_lexeme_ptr();
			continue;
		}
		if (isalpha(*forward) || *forward == '_') {
			get_char();
			while (isalnum(*forward) || *forward == '_') {
				get_char();
			}
			char* lexeme = get_lexeme();
			set_lexeme_ptr();
			token->lexeme = lexeme;
			int type = SymTab_get(symbol_table, lexeme);
			if (type != -1) {
				printf("\nGot it! %s\n\n", lexeme);
				token->type = type;
			} else {
				SymTab_set(symbol_table, lexeme, ID);
				token->type = ID;
			}
			token->line = line_num;
			token->column = column_num-strlen(lexeme);
			return token;
		}
		if (isdigit(*forward)) {
			get_char();

			while (isdigit(*forward)) {
				get_char();
			}

			if (*forward == '.') {
				get_char();
				token->type = FCONST;
				while (isdigit(*forward))
					get_char();
			} else {
				token->type = ICONST;
			}
			char* lexeme = get_lexeme();
			set_lexeme_ptr();
			token->lexeme = lexeme;
			token->line = line_num;
			token->column = column_num-strlen(lexeme);
			return token;
		}
		switch(*forward) {
			char tmp0;
			char tmp1;
			char *lexeme;
			case '#':
				get_char();
				switch(*forward) {
					case 'o':
						get_char();
						if (*forward != '#') {
							token_error();
							break;
						}
						get_char();
						char last_last = *forward;
						get_char();
						char last = *forward;
						get_char();
						while (!(last_last == '#' && last == 'o' && *forward == '#')) {
							last_last = last;
							last = *forward;
							get_char();
						}
						get_char();
						break;
					default:
						while (*forward != '\n')
							get_char();
						break;

				}
				set_lexeme_ptr();
				break;
			case '+':
			case '-':
			case '/':
			case '*':
				tmp0 = *forward;
				get_char();
				tmp1 = *forward;
				if (tmp1 == '=') {
					get_char();
					token->type = ASSIGN;
					token->lexeme = get_lexeme();
					token->line = line_num;
					token->column = column_num-strlen(token->lexeme);
					set_lexeme_ptr();
					return token;
				} else if (tmp1 == tmp0) {
					get_char();
					token->type = SUFFIXOP;
					token->lexeme = get_lexeme();
					token->line = line_num;
					token->column = column_num-strlen(token->lexeme);
					set_lexeme_ptr();
					return token;
				} else {
					lexeme = get_lexeme();
					if (strlen(lexeme) > 1)
						token_error();
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
			case '%':
			case '^':
			case '=':
			case '<':
			case '>':
			case '!':
				get_char();
				if (*forward == '=') {
					get_char();
					token->type = ASSIGN;
					token->lexeme = get_lexeme();
					token->line = line_num;
					token->column = column_num-2;
					set_lexeme_ptr();
					return token;
				} else {
					lexeme = get_lexeme();
					if (strlen(lexeme) > 1)
						token_error();
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
			case '"':
			case '\'':
				tmp0 = *forward;
				get_char();
				set_lexeme_ptr();
				while (*forward != tmp0) {
					get_char();
				}
				lexeme = get_lexeme();
				get_char();
				set_lexeme_ptr();
				token->lexeme = lexeme;
				token->type = SCONST;
				token->line = line_num;
				token->column = column_num-strlen(lexeme);
				return token;
			default:
				get_char();
				lexeme = get_lexeme();
				if (strlen(lexeme) > 1)
					token_error();
				token->lexeme = lexeme;
				token->type = *lexeme;
				token->line = line_num;
				token->column = column_num-strlen(token->lexeme);
				set_lexeme_ptr();
				return token;
		}

	}
	token->type = _EOF;
	return token;
}

int main(int argc, char** argv) {
	file_desc = open(argv[1], O_RDONLY);
	init_lexer();
	struct Token* token;
	while (!read_done) {
		token = get_token();
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
			case FLOAT_TYPE:
				printf("FLOAT_TYPE: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case INT_TYPE:
				printf("INT_TYPE: \'%s\' at line: %d, column: %d\n",
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
		if (token->type != _EOF)
			free(token->lexeme);
		free(token);
		token = NULL;

	}
	SymTab_dump(symbol_table);
	SymTab_destroy(symbol_table);
	return error_flag;
}
