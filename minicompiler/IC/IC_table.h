struct IC_table {
    int size;
    struct IC_entry** entries;
};

struct IC_entry {
    char* key;
    struct QuadList* instruction_list;
    struct IC_entry* next;
    long** t;
};
struct IC_table* create_IC_table(int table_size);
void IC_table_create_entry(struct IC_table* code_table, char* key);
struct IC_entry* IC_table_get_entry(struct IC_table* code_table, char* key);
