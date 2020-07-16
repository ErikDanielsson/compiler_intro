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
#define TABLENGTH 8
#define FALSE 0
#define TRUE 1

#define INJECE

const char* filename;
int file_desc;
char last_buffert = FALSE;
char on_last_buffert = FALSE;
char read_done = FALSE;
char* eof = "eof";

// Two buffers but fit into one, two extra chars for eof i.e 0x04
char buffer[2*BUFFERSIZE+2];
char* buffer_ptr = buffer;
char last_line[LINELENGTH];
char curr_line[LINELENGTH];
char next_line[LINELENGTH];
char* forward = curr_line;
char* lexeme_begin = curr_line;

int line_num = 1;
int column_num = 1;

struct SymTab* symbol_table;
int error_flag = 0;

void error(const char* type_msg, int length,
	   const char* expected, int fatal,
	   int line, int column,
   	   int inject_symbol, char symbol) {
	fprintf(stderr, "\n%s:\033[1;31merror\033[0m: %s at %d:%d\n%s\n", filename, type_msg, line, column, expected);
	error_flag = -1;
	fprintf(stderr, " ... |\n");
	if (line > 1) {
		if (inject_symbol == -1) {
			fprintf(stderr, "%4d |", line-1);
			char* tmp = last_line;
			int count = 0;
			char c;
			for (;(c = *tmp) != '\n'; tmp++) {
				if (c == '\t') {
					fprintf(stderr, "        ");
					count += TABLENGTH-1;
				} else {
					fprintf(stderr, "%c", c);
				}
				count++;
			}
			fprintf(stderr, "\033[1;34m%c\033[0m", symbol);
			fprintf(stderr, "\n");
			fprintf(stderr,"     |");
			for (; count > 0; count--)
				fprintf(stderr, " ");
			fprintf(stderr, "\033[1;31m^\033[0m\n");

		} else {
			fprintf(stderr, "%4d |", line-1);
			char* tmp = last_line;
			char c;
			for (;(c = *tmp) != 0x00; tmp++) {
				if (c == '\t')
					fprintf(stderr, "        ");
				else
					fprintf(stderr, "%c", c);
			}
			fprintf(stderr, "     |\n");

		}
	}
	char* curr_i = curr_line;
	char c;

	fprintf(stderr, "%4d |", line);
	if (*curr_i == 0x00) {
		if (inject_symbol)
			fprintf(stderr,"\033[1;34m%c\033[0m", symbol);
		fprintf(stderr,"\n");
		fprintf(stderr, "     |");
		if (inject_symbol)
			fprintf(stderr,"\033[1;31m^\033[0m");
	} else {
		while ((c = *curr_i) != 0x00) {
			int index = curr_i-curr_line+1;
			if (inject_symbol && index == inject_symbol)
				fprintf(stderr,"\033[1;34m%c\033[0m", symbol);
			if (index == column)
				fprintf(stderr, "\033[1;31m");
			if (index == column+length)
				fprintf(stderr, "\033[0m");
			if (c == '\t')
				fprintf(stderr, "        ");
			else
				fprintf(stderr, "%c", c);
			curr_i++;
		}
		fprintf(stderr, "\033[0m");
		fprintf(stderr, "     |");
		curr_i = curr_line;
		while ((c = *curr_i) != 0x00) {
			int i = curr_i-curr_line+1;
			if (i == column || i == inject_symbol) {
				fprintf(stderr, "\033[1;31m");
				fprintf(stderr, "^");
				break;
			} else if (c == '\t'){
			fprintf(stderr, "        ");
			} else {
				curr_i++;
			}
		}
		if (!inject_symbol) {
			while (*curr_i != 0x00 && curr_i-curr_line+2 < column+length) {
				fprintf(stderr, "~");
				curr_i++;
			}
		}
	}
	fprintf(stderr, "\033[0m\n");
	if (!read_done) {
		fprintf(stderr, "%4d |", line+1);
		for (curr_i = next_line; (c = *curr_i) != 0x00; curr_i++) {
			if (c == '\t')
				fprintf(stderr, "        ");
			else
				fprintf(stderr, "%c", c);
		}
		fprintf(stderr, " ... |\n");
	} else {
		fprintf(stderr, " eof |\n");
	}

	if (fatal)
		exit(-1);
}


void token_error(int length, char* expected, int fatal) {
	error("unidentified token", length, expected, fatal, line_num, column_num, 0, 0);
}
int get_line() {
	int i;
	if (*buffer_ptr == 0x04) {
		if (buffer_ptr == buffer+BUFFERSIZE) {
			int r = read(file_desc, buffer+BUFFERSIZE+1, BUFFERSIZE);
			buffer[BUFFERSIZE+r] = 0x04;
			buffer_ptr++;
		} else if (buffer_ptr == buffer+2*BUFFERSIZE+1) {
			int r = read(file_desc, buffer, BUFFERSIZE);
			buffer[r] = 0x04;
			buffer_ptr = buffer;
		} else {
			last_buffert = TRUE;
			next_line[0] = 0x00;
			return 0;
		}
	}
	next_line[0] = *buffer_ptr;
	for (i = 1; i < LINELENGTH-1 && *buffer_ptr != '\n';) {
		buffer_ptr++;
		if (*buffer_ptr == 0x04) {
			if (buffer_ptr == buffer+BUFFERSIZE) {
				int r = read(file_desc, buffer+BUFFERSIZE+1, BUFFERSIZE);
				buffer[BUFFERSIZE+r] = 0x04;
				buffer_ptr++;
			} else if (buffer_ptr == buffer+2*BUFFERSIZE+1) {
				int r = read(file_desc, buffer, BUFFERSIZE);
				buffer[r] = 0x04;
				buffer_ptr = buffer;
			} else {
				last_buffert = TRUE;
				next_line[i] = 0x00;
				return i;
			}
		}
		next_line[i] = *buffer_ptr;
		i++;
	}
	buffer_ptr++;
	next_line[i] = 0x00;
	return i;
}
int n_read = LINELENGTH-1;

void read_from_buffert() {
	strcpy(last_line, curr_line);
	strcpy(curr_line, next_line);
	if (last_buffert) {
		if (n_read == 0) {
			read_done = TRUE;
			return;
		}
		on_last_buffert = TRUE;
	} else if (on_last_buffert) {
		read_done = TRUE;
		*forward = 0x04;
		return;
	} else {
	n_read = get_line();
	}
	forward = curr_line;
}

void get_char() {
	forward++;
	if (*forward == 0x00) {
		read_from_buffert();
		line_num++;
		column_num = 0;
	}
	if (*forward == '\t')
		column_num += TABLENGTH;
	else
		column_num++;
}
char* get_lexeme() {
	int length = forward-lexeme_begin;
	char* lexeme = malloc(sizeof(char)*(length+1));
	for (int i = 0; i < length; i++)
		lexeme[i] = *(lexeme_begin+i);
	lexeme[length] = 0x00;
	return lexeme;
}

void init_lexer() {
	// Initialize last char of both buffers to eof
	int r = read(file_desc, buffer, BUFFERSIZE);
	buffer[r] = 0x04;
	n_read = get_line();
	read_from_buffert();

	symbol_table = create_SymTab();
	SymTab_set(symbol_table, "fofloloatot", ID);
	SymTab_set(symbol_table, "inontot", ID);
	SymTab_set(symbol_table, "sostotrorinongog", ID);
	SymTab_set(symbol_table, "naand", NAND);
	SymTab_set(symbol_table, "ifof", IF);
	SymTab_set(symbol_table, "elolifof", ELIF);
	SymTab_set(symbol_table, "elolsose", ELSE);
	SymTab_set(symbol_table, "wowhohilole", WHILE);
	SymTab_set(symbol_table, "foforor", FOR);
	SymTab_set(symbol_table, "dodefofinone", DEFINE);
	SymTab_set(symbol_table, "roretoturornon", RETURN);
	SymTab_set(symbol_table, "inonpoputot", ID);
	SymTab_set(symbol_table, "poprorinontot", ID);
}

void set_lexeme_ptr() {
	// a useless procedure
	lexeme_begin = forward;
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
								if (read_done)
									token_error(0, "End of file when scanning multi line comment", 1);
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
						token_error(len, "expected single char token.", 0);
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
						token_error(len, "expected single char token.", 0);
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
			case '=':
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
						token_error(len, "expected single char token.", 0);
					token->lexeme = lexeme;
					token->type = *lexeme;
					token->line = line_num;
					token->column = column_num-1;
					set_lexeme_ptr();
					return token;
				}
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
						token_error(len, "expected single char token.", 0);
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
				tmp1 = *forward;
				set_lexeme_ptr();
				while (tmp1 != tmp0) {
					get_char();
					if ((tmp1 = *forward) == '\n') {
						int len = strlen(get_lexeme());
						token_error(len, "End of line while scanning string literal", 0);
						break;
					}

				}
				if (tmp1 == '\n')
					break;
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
					token_error(len, "expected single char token.", 0);
				token->lexeme = lexeme;
				token->type = *lexeme;
				token->line = line_num;
				token->column = column_num-strlen(token->lexeme);
				set_lexeme_ptr();
				return token;
		}
	}
	token->type = 0x04;
	token->lexeme = eof;
	token->line = line_num;
	token->column = 0;
	return token;
}
