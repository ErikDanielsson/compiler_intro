#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "int_set.h"
#include "hashing.h"

int addb(struct int_set* set, int item)
{
    unsigned int hashv = int_hash(item, set->size);
    struct int_set_entry* entry = set->entries[hashv];
    struct int_set_entry* prev;
    if (entry == NULL) {
        set->entries[hashv] = malloc(sizeof(struct int_set_entry));
        set->entries[hashv]->val = item;
        set->entries[hashv]->next = NULL;
        return 1;
    }
    do {
        if (entry->val == item)
            return 0;
        prev = entry;
        entry = entry->next;
    } while (entry != NULL)
    prev->next = malloc(sizeof(struct int_set_entry));
    prev->next->val = item;
    prev->next->next = NULL;
    return 1;
}

void destroy_int_set(struct int_set* set)
{
    for (int i = 0; i < set->size; i++) {
        struct int_set_entry* entry = set->entries[i];
        if (entry == NULL)
            continue;
        struct int_set_entry* prev;
        while (entry != NULL) {
            prev = entry;
            entry = entry->next;
            free(prev);
        }
    }
    free(set->entries);
}
