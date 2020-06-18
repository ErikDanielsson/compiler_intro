#pragma once
#include "lexer.h"

unsigned int hash(const char* key);

typedef struct entry {
	char* key;
	token* value;
	struct entry* next;
} entry;

typedef struct {
	entry** entries;
} SymTab;

SymTab* create_SymTab();

entry* SymTab_pair(const char* key, token* value;);

void SymTab_set(SymTab* symboltable, const char* key, token* value);

token* SymTab_get(SymTab* symboltable, const char* key);

void SymTab_dump(SymTab* symboltable);
