#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symbol_table.h"
#include "hashing.h"
#include "io.h"
#include "consts.h"
#include "parser.h"

struct SymTab* create_SymTab(int table_size, struct SymTab* parent, char* name)
{
    struct SymTab* symbol_table = malloc(sizeof(struct SymTab));
    symbol_table->table_size = table_size;
    symbol_table->name = name;
    symbol_table->entries = malloc(sizeof(struct SymTab_entry*) * table_size);

    for (int i = 0; i < table_size; i++) {
        symbol_table->entries[i] = NULL;
    }
    symbol_table->parent = parent;
    symbol_table->n_childs = 0;
    return symbol_table;
}

void SymTab_append_child(struct SymTab* parent, struct SymTab* child)
{
    if (parent->n_childs) {
        int n_childs = parent->n_childs;
        struct SymTab* tmp[n_childs];
        memcpy(tmp, parent->childs, sizeof(struct SymTab*)*n_childs);
        parent->n_childs++;

        free(parent->childs);
        parent->childs = malloc(sizeof(struct SymTab*)*(n_childs+1));
        memcpy(parent->childs, tmp, sizeof(struct SymTab*)*n_childs);
        parent->childs[n_childs] = child;
    } else {
        parent->n_childs = 1;
        parent->childs = malloc(sizeof(struct SymTab*)*1);
        parent->childs[0] = child;

    }
}
/*
 * Since the lang supports nested scopes with redeclarations of local variables.
 * This introduces a problem, since we need a way to uniquely distinguish
 * between different declarations of the same variable during IC generation.
 * This is most simply done by assigning to each variable it's entry number
 * which is guaranteed to be unique. Behold the symbol_counter.
 */
unsigned int symbol_counter = 0;
struct SymTab_entry* SymTab_pair(char* key, enum SymbolType type,
                                void* symbol, long offset, int width)
{

    struct SymTab_entry* entry = malloc(sizeof(struct SymTab_entry) * 1);
    entry->key = malloc(sizeof(char)*(strlen(key)+1));
    strcpy(entry->key, key);
    entry->type = type;
    entry->next = NULL;
    entry->symbol = symbol;
    entry->offset = offset;
    entry->width = width;
    entry->counter_value = symbol_counter;
    symbol_counter++;
    return entry;
}

int SymTab_check_and_set(struct SymTab* symbol_table, char* key,
                    enum SymbolType type, void* symbol, long offset, int width)
{
    unsigned int slot = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry = symbol_table->entries[slot];
    struct SymTab_entry* prev;
    if (entry == NULL) {
        symbol_table->entries[slot] = SymTab_pair(key, type, symbol, offset, width);
        return 0;
    }
    do {
        if (entry->type == type && strcmp(entry->key, key) == 0)
            return 1;

        prev = entry;
        entry = prev->next;
    } while (entry != NULL);
    prev->next = SymTab_pair(key, type, symbol, offset, width);
    return 0;
}

struct SymTab_entry* find_in_table(struct SymTab* symbol_table, unsigned int hashv, char* key,
                                    enum SymbolType type)
{
    struct SymTab_entry* entry = symbol_table->entries[hashv];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0 && entry->type == type)
            return entry;
        entry = entry->next;
    }
    return NULL;
}

/*
 * Search symbol table and parents recursivly for entry
 */
struct SymTab_entry* SymTab_getr(struct SymTab* symbol_table, char* key,
                            enum SymbolType type)
{
    unsigned int hashv = hash(key, symbol_table->table_size);
    struct SymTab_entry* entry;
    for(; symbol_table != NULL; symbol_table = symbol_table->parent) {
        if ((entry = find_in_table(symbol_table,  hashv, key, type)) != NULL)
            return entry;
    }
    return NULL;
}

void SymTab_destroy(struct SymTab* symbol_table)
{
    if (symbol_table == NULL)
        return;
    if (symbol_table->entries != NULL) {
        size_t i = 0;
        while (i < symbol_table->table_size) {
            struct SymTab_entry* head = symbol_table->entries[i];
            while (head != NULL) {
                struct SymTab_entry* tmp = head;
                if (tmp->key != NULL) {
                    free(tmp->key);
                    tmp->key = NULL;
                }

                head = head->next;
                free(tmp);
            }
            i++;
        }
        free(symbol_table->entries);
        symbol_table->entries = NULL;
    }
    free(symbol_table);
    symbol_table = NULL;
}

void SymTab_destroyr(struct SymTab* symbol_table)
{
    for (int i = 0; i < symbol_table->n_childs; i++)
        SymTab_destroyr(symbol_table->childs[i]);
    SymTab_destroy(symbol_table);
}

void SymTab_dump(struct SymTab* symbol_table,  int indent)
{
    printf("\n");
    if (symbol_table->name != NULL)
        print_w_indent(indent, "%s:\n", symbol_table->name);
    else
        print_w_indent(indent, "Local env:\n", symbol_table->name);

    for (int i = 0; i < symbol_table->table_size; i++) {
        struct SymTab_entry* entry = symbol_table->entries[i];
        if (entry == NULL) {
            continue;
        }
        print_w_indent(indent, "%4d:\t", i);
        if (entry->type == VARIABLE) {
            struct VarDecl* symbol = entry->symbol;
            printf("var %-15s : %-5s At offset %lx\n", symbol->type->lexeme, entry->key, entry->offset);
        } else if (entry->type == TEMPORARY) {
            printf("temp t%s\n", entry->key);
        } else {
            struct FuncDecl* symbol = entry->symbol;
            printf("func %s : %s\n", symbol->type->lexeme, entry->key);
        }

        while (TRUE) {
            if (entry->next == NULL) {
                break;
            }
            entry = entry->next;
            if (entry->type == VARIABLE) {
                struct VarDecl* symbol = entry->symbol;
                printf("\tvar %-15s : %-5s At offset %lx\n", symbol->type->lexeme, entry->key, entry->offset);
            } else if (entry->type == TEMPORARY) {
                printf("\ttemp t%s\n", entry->key);
            } else {
                struct FuncDecl* symbol = entry->symbol;
                printf("\tfunc %s : %s\n", symbol->type->lexeme, entry->key);
            }        }
        printf("\n");
    }
    printf("\n");
}
