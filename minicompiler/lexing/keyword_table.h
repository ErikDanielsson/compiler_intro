#pragma once
#include "lexer.h"
#include "consts.h"

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

struct keyword_entry* KeywordTab_pair(char* key, enum TokenType type);

void KeywordTab_set(struct KeywordTab* symboltable, char* key, enum TokenType type);

enum TokenType KeywordTab_get(struct KeywordTab* symboltable, char* key);

char* KeywordTab_get_key_ptr(struct KeywordTab* symboltable, char* key);

char* KeywordTab_first_key_by_value(struct KeywordTab* symboltable, enum TokenType value);

char* closest_key(struct KeywordTab* symboltable, char* string);
char* closest_keyword_with_action(struct KeywordTab* symboltable, char* string,
                                    int* action_row, int n_states,
                                    enum TokenType* should_be);

void KeywordTab_destroy(struct KeywordTab* symboltable);

void KeywordTab_dump(struct KeywordTab* symboltable);
