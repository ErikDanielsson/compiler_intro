#include <string.h>
unsigned int hash(const char* key, int table_size)
{
    // FVN (Fowler / Noll / Vo) hash function:
    unsigned int h = 0x811c9dc5;
    for (int i = 0; i < strlen(key); i++ ) {
        h = ( h ^ key[i] ) * 0x01000193;
    }
    return h % table_size;
}

unsigned int max_hash(const char* key)
{
    // FVN (Fowler / Noll / Vo) hash function:
    unsigned int h = 0x811c9dc5;
    for (int i = 0; i < strlen(key); i++ ) {
        h = ( h ^ key[i] ) * 0x01000193;
    }
    return h;
}