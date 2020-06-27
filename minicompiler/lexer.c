#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "lexer.h"

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
		} else if (forward == buffer+2*BUFFERSIZE+1) {
			int r = read(file_desc, buffer, BUFFERSIZE);
			buffer[r] = 0x04;
			forward = buffer-1; // Since it is increased
		} else {
			read_done = TRUE;
			return;
		}
	}
	column_num++;
}
char peek() {
	char* tmp = forward;
	getchar();
	char tmp2 = *forward;
	forward = tmp;
	return tmp2;
}

void init_lexer() {
	// Initialize last char of both buffers to eof
	int r = read(file_desc, buffer, BUFFERSIZE);
	buffer[r] = 0x04;
	printf("%d\n", r);
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
			token->type = ID;
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
				token->type = FLOAT;
				while (isdigit(*forward))
					get_char();
			} else {
				token->type = INT;
			}
			char* lexeme = get_lexeme();
			set_lexeme_ptr();
			token->lexeme = lexeme;
			token->line = line_num;
			token->column = column_num-strlen(lexeme);
			return token;
		}
		if (*forward == '/') {
			get_char();
			switch(*forward) {
				case '/':
					while (*forward != '\n')
						get_char();
					break;
				case '*':
					while (*forward != '*' && peek() != '/')
						get_char();
					break;
				default:
					token->type = '/';
					token->lexeme = get_lexeme();
					token->line = line_num;
					token->column = column_num-strlen(token->lexeme);
					return token
			}
			continue;
		}

	}
	token->type = _EOF;
	return token;
}

int main(int argc, char** argv) {
	file_desc = open(argv[1], O_RDONLY);
	init_lexer();
	struct Token* token = get_token();
	printf("lexeme: \'%s\' at line: %d, column: %d\n",
		token->lexeme, token->line, token->column);
	free(token->lexeme);

	for (int i = 0; i < 60; i++) {
		token = get_token();
		switch (token->type) {
			case ID:
				printf("ID: lexeme: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case INT:
				printf("INT: lexeme: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case FLOAT:
				printf("FLOAT: lexeme: \'%s\' at line: %d, column: %d\n",
					token->lexeme, token->line, token->column);
				break;
			case _EOF:
				printf("EOF\n");
		}
		free(token->lexeme);
	}
	return 0;
}
