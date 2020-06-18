#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "symbol_table.h"
#include "lexer.h"
#define TABLE_SIZE 1000

unsigned int hash( const char* key) {
	// FVN (Fowler / Noll / Vo) hash function:
	const char *p = key;
	unsigned int key_len = strlen(key);
	unsigned int h = 0x811c9dc5;
	int i;

	for ( i = 0; i < key_len; i++ ) {
		h = ( h ^ p[i] ) * 0x01000193;
	}
	return h % TABLE_SIZE;
}

SymTab* create_SymTab(void) {
	SymTab* symboltable = malloc(sizeof(SymTab) * 1);

	symboltable->entries = malloc(sizeof(entry*)*TABLE_SIZE);

	for (int i = 0; i < TABLE_SIZE; i++) {
		symboltable->entries[i] = NULL;
	}
	return symboltable;
}
entry* SymTab_pair(const char* key, token* value) {
	entry* entry = malloc(sizeof(entry) * 1);
	entry->key = malloc(sizeof(strlen(key))+1);
	entry->value = malloc(sizeof(token*));
	strcpy(entry->key, key);
	entry->value = value;

	entry->next = NULL;
	return entry;
}
void SymTab_set(SymTab* symboltable, const char* key, token* value) {
	unsigned int slot = hash(key);
	entry* entry = symboltable->entries[slot];
	struct entry* prev;
	if (entry == NULL) {
		symboltable->entries[slot] = SymTab_pair(key, value);
		return;
	}


	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			free(entry->value);
			entry->value = malloc(sizeof(token*));
			entry->value = value;
			return;
		}
		prev = entry;
		entry = prev->next;
	}
	prev->next = SymTab_pair(key, value);
}

token* SymTab_get(SymTab* symboltable, const char* key) {
	unsigned int slot = hash(key);
	entry* entry = symboltable->entries[slot];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->value;
		}
		entry = entry->next;
	}
	return NULL;
}

void SymTab_dump(SymTab* symboltable) {
	printf("\n");
	printf("Symbol table:\n");
	for (int i = 0; i < TABLE_SIZE; i++) {
		entry* entry = symboltable->entries[i];
		if (entry == NULL) {
			continue;
		}
		printf("%4d: ", i);
		while (1) {
			printf("%s : %s, ", entry->key, entry->value->string);
			if (entry->next == NULL) {
				break;
			}
			entry = entry->next;
		}
		printf("\n");
	}
	printf("\n");

}
