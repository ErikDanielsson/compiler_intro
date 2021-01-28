#pragma once
struct int_set {
    int size;
    struct int_set_entry** entries;
};

struct int_set_entry {
    int item;
    struct int_set_entry* next;
};

struct int_set* create_int_set(int size);
int addb(struct int_set* set, int item);
void destroy_int_set(struct int_set* set);
