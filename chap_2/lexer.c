#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "lexer.h"
#include "symbol_table.h"
#include "error.h"
#define MAX_ID_LEN 64




char peek;
int line = 1;
SymTab* symboltable;

void init() {
	peek = getchar();
	symboltable = create_SymTab();
}



token* Lexer() {

	// Skip whitespace, and increment line num
	for ( ; ; peek = getchar()) {
		if (peek == ' ' || peek == '\t')
			continue;
		else if (peek == '\n') line++;
		else break;
	}
	// Skip comments
	if (peek == '/') {
		peek = getchar();
		if (peek == '/')
			while (peek != '\n')
				peek = getchar();
		else if (peek == '*') {
				while (1) {
					peek = getchar();
					if (peek == '*') {
						peek = getchar();
						if (peek == '/') {
							peek = getchar();
							break;
						}
					}
				}
			}
		else {
			token* ptr = malloc(sizeof(token));
			find_ptr_error(ptr);
			ptr->type = '/';
			return ptr;
		}

	}
	if (isdigit(peek)) {
		int num = 0;
		do {
			num = 10*num + peek-0x30;
			peek = getchar();
		}
		while (isdigit(peek));
		token* ptr = malloc(sizeof(token));
		find_ptr_error(ptr);
		ptr->type = NUM;
		ptr->value = num;
		return ptr;
	}
	if (isalpha(peek) || (peek == 0x5F)) {
		char string[MAX_ID_LEN] = {0};
		string[0] = peek;
		peek = getchar();
		int i = 1;
		for (; isalnum(peek) && i < MAX_ID_LEN-1; i++) {
		   string[i] = peek;
		   peek = getchar();
		}
		if (isalnum(peek)) {
			fprintf(stderr, "ERROR: identifier too long");
			return NULL;
		}
		i++;
		string[i] = 0x00;
		token* t = SymTab_get(symboltable, string);
		if (t != NULL) {
			return t;
		}
		token* ptr = malloc(sizeof(token));
		find_ptr_error(ptr);
		ptr->type = ID;
		strcpy(ptr->string, string);
		SymTab_set(symboltable, string, ptr);
		return ptr;
	}
	token* ptr = malloc(sizeof(token));
	find_ptr_error(ptr);
	ptr->type = peek;
	peek=' ';
	return ptr;
}


int main() {
	init();
	while (1) {
		token* temp = Lexer();
		find_ptr_error(temp);
		if (temp->type == ID) {
			printf("String: %s\n", temp->string);
		}
		else if (temp->type == NUM) {
			printf("Num: %d\n", temp->value);
		}
		else {
			printf("Char token: %c\n", temp->type);
		}
		SymTab_dump(symboltable);

	}
}
