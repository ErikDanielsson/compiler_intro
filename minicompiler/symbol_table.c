#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "symbol_table.h"
#include "lexer.h"
#define TABLE_SIZE 100

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

struct SymTab* create_SymTab() {
	struct SymTab* symboltable = malloc(sizeof(struct SymTab) * 1);

	symboltable->entries = malloc(sizeof(struct entry*) * TABLE_SIZE);

	for (int i = 0; i < TABLE_SIZE; i++) {
		symboltable->entries[i] = NULL;
	}

	return symboltable;
}
struct entry* SymTab_pair(const char* key, enum TokenType type) {
	struct entry* entry = malloc(sizeof(struct entry) * 1);
	entry->key = malloc(sizeof(strlen(key))+1);
	strcpy(entry->key, key);
	entry->type = type;
	entry->next = NULL;
	return entry;
}
void SymTab_set(struct SymTab* symboltable, const char* key, enum TokenType type) {
	unsigned int slot = hash(key);
	struct entry* entry = symboltable->entries[slot];
	struct entry* prev;
	if (entry == NULL) {
		symboltable->entries[slot] = SymTab_pair(key, type);
		return;
	}
	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			entry->type = type;
			return;
		}
		prev = entry;
		entry = prev->next;
	}
	prev->next = SymTab_pair(key, type);
}

enum TokenType SymTab_get(struct SymTab* symboltable, const char* key) {
	unsigned int slot = hash(key);
	struct entry* entry = symboltable->entries[slot];

	while (entry != NULL) {
		if (strcmp(entry->key, key) == 0) {
			return entry->type;
		}
		entry = entry->next;
	}
	return -1;
}

void SymTab_destroy(struct SymTab* symboltable) {
	if (symboltable == NULL)
		return;
	if (symboltable->entries != NULL) {
		size_t i = 0;
		while (i < TABLE_SIZE) {
			struct entry* head = symboltable->entries[i];
			while (head != NULL) {
				struct entry* tmp = head;
				if (tmp->key != NULL) {
					free(tmp->key);
					tmp->key = NULL;
				}

				head = head->next;
				free(tmp);
			}
			i++;
		}
		free(symboltable->entries);
		symboltable->entries = NULL;
	}
	free(symboltable);
	symboltable = NULL;
}

void SymTab_dump(struct SymTab* symboltable) {
	printf("\n");
	printf("Symbol table:\n");
	for (int i = 0; i < TABLE_SIZE; i++) {
		struct entry* entry = symboltable->entries[i];
		if (entry == NULL) {
			continue;
		}
		printf("%4d:\t", i);
		printf("%s : %x", entry->key, entry->type);
		while (1) {
			if (entry->next == NULL) {
				break;
			}
			entry = entry->next;
			printf(",\n\t%s : %x", entry->key, entry->type);
		}
		printf("\n");
	}
	printf("\n");
}
