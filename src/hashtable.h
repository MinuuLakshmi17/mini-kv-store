#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <stddef.h>
typedef struct HashEntry { char *key; char *value; struct HashEntry *next; } HashEntry;
typedef struct { HashEntry **buckets; size_t capacity; size_t size; } HashTable;
HashTable *hashtable_create(size_t initial_capacity);
void hashtable_destroy(HashTable *table);
int hashtable_set(HashTable *table, const char *key, const char *value);
const char *hashtable_get(const HashTable *table, const char *key);
int hashtable_delete(HashTable *table, const char *key);
int hashtable_exists(const HashTable *table, const char *key);
#endif
