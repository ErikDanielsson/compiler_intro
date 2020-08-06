#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "keyword_table.h"
#include "lexer.h"
#include "hashing.h"
#define MAX_ID_SIZE 256

struct KeywordTab* create_KeywordTab(int table_size)
{
    struct KeywordTab* keywords = malloc(sizeof(struct KeywordTab) * 1);
    keywords->table_size  = table_size;
    keywords->entries = malloc(sizeof(struct keyword_entry*) * table_size);

    for (int i = 0; i < table_size; i++) {
        keywords->entries[i] = NULL;
    }

    return keywords;
}

struct keyword_entry* KeywordTab_pair(char* key, enum TokenType type)
{
    struct keyword_entry* entry = malloc(sizeof(struct keyword_entry) * 1);
    entry->key = malloc(sizeof(char)*strlen(key)+1);
    strcpy(entry->key, key);
    entry->type = type;
    entry->next = NULL;
    return entry;
}

void KeywordTab_set(struct KeywordTab* keywords, char* key, enum TokenType type)
{
    unsigned int slot = hash(key, keywords->table_size);
    struct keyword_entry* entry = keywords->entries[slot];
    struct keyword_entry* prev;
    if (entry == NULL) {
        keywords->entries[slot] = KeywordTab_pair(key, type);
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
    prev->next = KeywordTab_pair(key, type);
}

enum TokenType KeywordTab_get(struct KeywordTab* keywords, char* key)
{
    unsigned int slot = hash(key, keywords->table_size);
    struct keyword_entry* entry = keywords->entries[slot];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->type;
        }
        entry = entry->next;
    }
    return -1;
}

char* KeywordTab_get_key_ptr(struct KeywordTab* keywords, char* key)
{
    unsigned int slot = hash(key, keywords->table_size);
    struct keyword_entry* entry = keywords->entries[slot];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->key;
        }
        entry = entry->next;
    }
    return NULL;
}

char* KeywordTab_first_key_by_type(struct KeywordTab* keywords, enum TokenType type)
{
    for (int i = 0; i < keywords->table_size; i++) {
        struct keyword_entry* entry = keywords->entries[i];
        while (entry != NULL) {
            if (entry->type == type) {
                return entry->key;
            }
            entry = entry->next;
        }
    }
    return NULL;
}

char* closest_keyword_with_action(struct KeywordTab* keywords, char* string,
                                    int* action_row, int n_states,
                                    enum TokenType* should_be)
{
    int n = strlen(string);
    int v0[n+1];
    int v1[n+1];

    char current_match[MAX_ID_SIZE];
    char tmp1[MAX_ID_SIZE];
    for (int i = 0; i < MAX_ID_SIZE-1; i++)
        tmp1[i] = 0x20;
    tmp1[MAX_ID_SIZE-1] = 0x00;
    int shortest_diff = 2147483647;
    char match = FALSE;
    for (int q = 0; q < keywords->table_size; q++) {
        struct keyword_entry* c_entry = keywords->entries[q];
        for (;c_entry != NULL; c_entry = c_entry->next) {

            if (action_row[c_entry->type] > n_states)
                continue;
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
                *should_be = c_entry->type;
                strcpy(current_match, tmp);
                match = TRUE;
            }
            for (int i = 0; i < m; i++)
                tmp1[i] = 0x20;
            c_entry = c_entry->next;
        }
    }
    if (!match)
        return NULL;
    char* res = malloc(sizeof(current_match));
    strcpy(res, current_match);
    return res;
}

void KeywordTab_destroy(struct KeywordTab* keywords)
{
    if (keywords == NULL)
        return;
    if (keywords->entries != NULL) {
        size_t i = 0;
        while (i < keywords->table_size) {
            struct keyword_entry* head = keywords->entries[i];
            while (head != NULL) {
                struct keyword_entry* tmp = head;
                if (tmp->key != NULL) {
                    free(tmp->key);
                    tmp->key = NULL;
                }

                head = head->next;
                free(tmp);
            }
            i++;
        }
        free(keywords->entries);
        keywords->entries = NULL;
    }
    free(keywords);
    keywords = NULL;
}

void KeywordTab_dump(struct KeywordTab* keywords)
{
    printf("\n");
    printf("Keywords:\n");
    for (int i = 0; i < keywords->table_size; i++) {
        struct keyword_entry* entry = keywords->entries[i];
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
