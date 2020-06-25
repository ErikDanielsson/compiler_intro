#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "symbol_table.h"
#include "error.h"
#define MAX_ID_LEN 64


char eof = 0;
FILE *input;
char peek;
const char** keyword;

int line = 1;
int column = 1;
struct SymTab* symboltable;

static inline void get_next_char();
void init_lexer()
{
	get_next_char();

	symboltable = create_SymTab();

	struct Token* True = malloc(sizeof(struct Token));
	True->type = TRUE;
	True->value = 1;
	struct Token* False = malloc(sizeof(struct Token));
	False->type = FALSE;
	False->value = 0;

	SymTab_set(symboltable, "true", True);
	SymTab_set(symboltable, "false", False);
}

static inline void get_next_char()
{
	int c;
	c = fgetc(input);
	if (!feof(input)) {

		peek = (char)c;
		if (peek == '\n') {
			line++;
			column = 0;
		}
		column++;
	}
	else {
		eof = 1;
		peek = 0x00;
	}
}

struct Token* Lexer()
{
	// Skip whitespace, and increment line num
	for(;;get_next_char()) {
		if (peek == ' ' || peek == '\t')
			continue;
		else if (peek == '\n') line++;
		else break;

	}
	// Skip comments
	if (peek == '/') {
		get_next_char();
		if (peek == '/')
			while (peek != '\n')
				get_next_char();
		else if (peek == '*') {
				while (1) {
					get_next_char();
					if (peek == '*') {
						get_next_char();
						if (peek == '/') {
							get_next_char();
							break;
						}
					}
				}
			}
		else {
			struct Token* ptr = malloc(sizeof(struct Token));
			find_ptr_error(ptr);
			ptr->type = '/';
			return ptr;
		}

	}
	if (isdigit(peek)) {
		int num = 0;
		do {
			num = 10*num + peek-0x30;
			get_next_char();
		}
		while (isdigit(peek));
		struct Token* ptr = malloc(sizeof(struct Token));
		find_ptr_error(ptr);
		ptr->type = NUM;
		ptr->value = num;
		return ptr;
	}
	if (isalpha(peek) || (peek == 0x5F)) {
		char string[MAX_ID_LEN] = {0};
		string[0] = peek;
		get_next_char();
		int i = 1;
		for (; isalnum(peek) && i < MAX_ID_LEN-1; i++) {
		   string[i] = peek;
		   get_next_char();
		}
		if (isalnum(peek)) {
			fprintf(stderr, "ERROR: identifier too long");
			return NULL;
		}
		i++;
		string[i] = 0x00;
		struct Token* t = SymTab_get(symboltable, string);
		if (t != NULL) {
			return t;
		}

		struct Token* ptr = malloc(sizeof(struct Token));

		find_ptr_error(ptr);

		ptr->type = ID;
		strcpy(ptr->string, string);

		SymTab_set(symboltable, string, ptr);

		return ptr;
	}
	struct Token* ptr = malloc(sizeof(struct Token));
	find_ptr_error(ptr);
	ptr->line = line;
	ptr->column = column;
	ptr->type = peek;
	peek=' ';
	return ptr;
}


int main(int argc, const char** argv) {
	if (argc == 1) {
		fprintf(stderr, "error: To few arguments\n");
		return 0;
	}
	input = fopen(argv[1], "r");
	init_lexer();
	if (input == NULL)
		return 0;
	while (!eof) {
		struct Token* temp = Lexer();
		find_ptr_error(temp);
		if (temp->type == ID) {
			printf("String: %s\n", temp->string);
		}
		else if (temp->type == NUM) {
			printf("Num: %d\n", temp->value);
			//free(temp);
		}
		else {
			printf("Char struct Token: %c\n", temp->type);
			//free(temp);
		}

	}
	SymTab_dump(symboltable);
	getchar();
	SymTab_destroy(symboltable);
	fclose(input);

}
