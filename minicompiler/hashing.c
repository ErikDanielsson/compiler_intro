#include <string.h>
unsigned int hash(const char* key, int table_size)
{
    // FVN (Fowler / Noll / Vo) hash function:
    const char *p = key;
    unsigned int key_len = strlen(key);
    unsigned int h = 0x811c9dc5;
    int i;

    for ( i = 0; i < key_len; i++ ) {
        h = ( h ^ p[i] ) * 0x01000193;
    }
    return h % table_size;
}

unsigned int max_hash(const char* key)
{
    // FVN (Fowler / Noll / Vo) hash function:
    const char *p = key;
    unsigned int key_len = strlen(key);
    unsigned int h = 0x811c9dc5;
    int i;

    for ( i = 0; i < key_len; i++ ) {
        h = ( h ^ p[i] ) * 0x01000193;
    }
    return h;
}
