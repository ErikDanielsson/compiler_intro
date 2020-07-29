#pragma once
#include "lexer.h"
#include "consts.h"

unsigned int hash(const char* key, int table_size);

struct keyword_entry {
    char* key;
    enum TokenType type;
    struct keyword_entry* next;
};

struct KeywordTab {
    int table_size;
    struct keyword_entry** entries;
};

struct KeywordTab* create_KeywordTab();

struct keyword_entry* KeywordTab_pair(const char* key, enum TokenType type);

void KeywordTab_set(struct KeywordTab* symboltable, const char* key, enum TokenType type);

enum TokenType KeywordTab_get(struct KeywordTab* symboltable, const char* key);

const char* KeywordTab_get_key_ptr(struct KeywordTab* symboltable, const char* key);

const char* KeywordTab_first_key_by_value(struct KeywordTab* symboltable, enum TokenType value);

char* closest_key(struct KeywordTab* symboltable, const char* string);
char* closest_keyword_with_action(struct KeywordTab* symboltable, const char* string,
                                    int* action_row, int n_states,
                                    enum TokenType* should_be)

void KeywordTab_destroy(struct KeywordTab* symboltable);

void KeywordTab_dump(struct KeywordTab* symboltable);
