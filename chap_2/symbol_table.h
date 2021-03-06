#pragma once
#include "lexer.h"

unsigned int hash(const char* key);

struct entry {
	char* key;
	struct Token* value;
	struct entry* next;
};

struct SymTab {
	struct entry** entries;
};

struct SymTab* create_SymTab();

struct entry* SymTab_pair(const char* key, struct Token* value);

void SymTab_set(struct SymTab* symboltable, const char* key, struct Token* value);

struct Token* SymTab_get(struct SymTab* symboltable, const char* key);

void SymTab_destroy(struct SymTab* symboltable);


void SymTab_dump(struct SymTab* symboltable);
