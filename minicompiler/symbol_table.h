#pragma once
#include "lexer.h"
#include "consts.h"

unsigned int hash(const char* key, int table_size);

struct entry {
    char* key;
    enum TokenType type;
    struct entry* next;
};

struct SymTab {
    int table_size;
    struct entry** entries;
};

struct SymTab* create_SymTab();

struct entry* SymTab_pair(const char* key, enum TokenType type);

void SymTab_set(struct SymTab* symboltable, const char* key, enum TokenType type);

enum TokenType SymTab_get(struct SymTab* symboltable, const char* key);

const char* SymTab_get_key_ptr(struct SymTab* symboltable, const char* key);

const char* SymTab_first_key_by_value(struct SymTab* symboltable, enum TokenType value);

char* closest_key(struct SymTab* symboltable, const char* string);

void SymTab_destroy(struct SymTab* symboltable);

void SymTab_dump(struct SymTab* symboltable);
