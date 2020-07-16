#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "symbol_table.h"
#include "lexer.h"
#define TABLE_SIZE 100
#define MAX_ID_SIZE 256

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
	entry->key = malloc(sizeof(char)*strlen(key)+1);
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

char* closest_key(struct SymTab* symboltable, const char* string) {
	int n = strlen(string);
	int v0[n+1];

	int v1[n+1];
	char current_match[MAX_ID_SIZE];
	char tmp1[MAX_ID_SIZE];
	for (int i = 0; i < MAX_ID_SIZE-1; i++)
		tmp1[i] = 0x20;
	tmp1[MAX_ID_SIZE-1] = 0x00;
	int shortest_diff = 2147483647;
	for (int q = 0; q< TABLE_SIZE; q++) {
		struct entry* c_entry = symboltable->entries[q];
		while (1) {
			if (c_entry == NULL) {
				break;
			}
			for (int i = 0; i < n+1; i++)
				v0[i] = i;
			char* tmp = c_entry->key;
			int m = strlen(tmp);
			strcpy(tmp1, tmp);
			tmp1[m] = 0x20;
			tmp = tmp1;
			for (int i = 1; i < MAX_ID_SIZE+1; i++) {
				v1[0] = i;
				for (int j = 1; j < n+1; j++) {
					int del = v0[j]+1;
					int ins = v1[j-1]+1;
					int sub = v0[j-1]+(string[j-1] != tmp[i-1]);
					if (del > ins) {
						if (ins > sub)
							v1[j] = sub;
						else
							v1[j] = ins;
					} else {
						if (del > sub)
							v1[j] = sub;
						else
							v1[j] = del;
					}
				}
				memcpy(v0, v1, sizeof(v1));

			}
			int tmp2 = v0[n];
			printf("%s: %d\n", tmp, tmp2);
			if (shortest_diff > tmp2) {
				shortest_diff = tmp2;
				strcpy(current_match, tmp);
			}
			for (int i = 0; i < m; i++)
				tmp1[i] = 0x20;
			c_entry = c_entry->next;
		}
	}
	char* res = malloc(sizeof(current_match));
	strcpy(res, current_match);
	return res;
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
