#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFERSIZE 4096
#define FALSE 0
#define TRUE 1


int file_desc;
char read_done = FALSE;
// Two buffers but fit into one, two extra chars for eof i.e 0x04
char buffer[2*BUFFERSIZE+2];
char* forward = buffer + 2*BUFFERSIZE+1;
char* lexeme_begin = buffer + 2*BUFFERSIZE+1;


int init_lexer() {
	// Initialize last char of both buffers to eof
	buffer[BUFFERSIZE] = 0x04;
	buffer[2*BUFFERSIZE+1] = 0x04;
}

char get_char() {

	if (*forward == 0x04) {
		if (forward == buffer+BUFFERSIZE) {
			int r = read(file_desc, buffer+BUFFERSIZE+1, BUFFERSIZE);
			buffer[r] = 0x04;
		} else if (forward == buffer+2*BUFFERSIZE+1) {
			int r = read(file_desc, buffer, BUFFERSIZE);
			buffer[r] = 0x04;
			forward = buffer-1; // Since it is increased
		} else {
			read_done = TRUE;
			forward--;
			return read_done;
		}
	}
	forward++;
	return *forward;
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

/*struct Token* get_token() {
	char current_char = get_char();
	struct Token* token = malloc(sizeof(struct Token));
	for (;!read_done; current_char=get_char()) {
		if (isblank(current_char)) {
			current_char = get_char();
			while (isblank(current_char))
				current_char = get_char();
			set_lexeme_ptr();
			continue;
		}
		if (isalpha(current_char) || current_char == '_') {
			current_char = get_char();
			while (isalnum(current_char) || current_char == '_')
				current_char = get_char();
			char* lexeme = malloc(sizeof())

		}
	}
	token->type = EOF;
	return token;
}*/

int main(int argc, char** argv) {
	file_desc = open(argv[1], O_RDONLY);
	init_lexer();
	

	//printf("%ld\n", *lexeme_begin);
	return 0;
}
