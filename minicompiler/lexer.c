#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"
#include "symbol_table.h"

#define BUFFERSIZE 4096
#define LINELENGTH 256
#define FALSE 0
#define TRUE 1

const char* filename;
int file_desc;
char read_done = FALSE;

// Two buffers but fit into one, two extra chars for eof i.e 0x04
char buffer[2*BUFFERSIZE+2];
char* forward = buffer;// + 2*BUFFERSIZE+1;
char* lexeme_begin = buffer;// + 2*BUFFERSIZE+1;
char last_line[LINELENGTH];
char curr_line[LINELENGTH];
int line_num = 1;
int column_num = 0;

struct SymTab* symbol_table;
int error_flag = 0;

void token_error(int length, char* expected) {
	fprintf(stderr, "\n%s:\033[1;31merror\033[0m: unidentified token at %d:%d\nexpected%s\n", filename, line_num, column_num-length, expected);
	error_flag = -1;
	fprintf(stderr, " ... |\n");
	if (line_num > 1)
		fprintf(stderr, "%4d |%s\n", line_num-1, last_line);
	char* curr_i = curr_line;
	fprintf(stderr, "%4d |", line_num);
	while (*curr_i != 0x00) {
		if (curr_i-curr_line+1 == column_num-length)
			fprintf(stderr, "\033[1;31m");
		fprintf(stderr, "%c", *curr_i);
		curr_i++;
	}
	fprintf(stderr, "\033[0m\n");
	fprintf(stderr, " ... |");
	curr_i = curr_line;
	while (*curr_i != 0x00) {
		if (curr_i-curr_line+1 == column_num-length) {
			fprintf(stderr, "\033[1;31m");
			fprintf(stderr, "^");
		} else if (curr_i-curr_line+1 > column_num-length) {
			fprintf(stderr, "~");
		} else {
		fprintf(stderr, " ");
		}
		curr_i++;
	}
	fprintf(stderr, "\033[0m\n\n");
}

void get_char() {
	forward++;
	if (*forward == '\n') {
		line_num++;
		strcpy(last_line, curr_line);
		for (int i = 0; i < column_num; i++)
			curr_line[i] = 0x00;
		column_num = 1;
		return;
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
	curr_line[column_num-1] = *forward;
	column_num++;
}

void init_lexer() {
	// Initialize last char of both buffers to eof
	int r = read(file_desc, buffer, BUFFERSIZE);
	printf("hejsan\n");
	buffer[r] = 0x04;
	for (int i = 0; i < LINELENGTH; i++)
		curr_line[i] = 0x00;

	symbol_table = create_SymTab();
	SymTab_set(symbol_table, "utotilolizinongog", UTILIZING);
	SymTab_set(symbol_table, "poprorogogroramom", PROGRAM);
	SymTab_set(symbol_table, "fofloloatot", FTYPE);
	SymTab_set(symbol_table, "inontot", ITYPE);
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
						if (*forward == '#') {
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
						}
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
					int len = strlen(lexeme);
					if (len > 1)
						token_error(len, " single char token.");
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
			case '%':
			case '^':
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
					int len = strlen(lexeme);
					if (len > 1)
						token_error(len, " single char token.");
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
			case '=':
			case '<':
			case '>':
			case '!':
				get_char();
				if (*forward == '=') {
					get_char();
					token->type = RELOP;
					token->lexeme = get_lexeme();
					token->line = line_num;
					token->column = column_num-2;
					set_lexeme_ptr();
					return token;
				} else {
					lexeme = get_lexeme();
					int len = strlen(lexeme);
					if (len > 1)
						token_error(len, " single char token.");
					token->lexeme = lexeme;
					token->type = RELOP;
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
				int len = strlen(lexeme);
				if (len > 1)
					token_error(len, " single char token.");
				token->lexeme = lexeme;
				token->type = *lexeme;
				token->line = line_num;
				token->column = column_num-strlen(token->lexeme);
				set_lexeme_ptr();
				return token;
		}

	}
	token->type = 0x04;
	return token;
}
