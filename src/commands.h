#ifndef COMMANDS_H
#define COMMANDS_H
#include <stddef.h>
#include "hashtable.h"
typedef struct KVStore KVStore;
KVStore *store_create(size_t capacity,const char *aof_path);
void store_destroy(KVStore *store);
int store_execute(KVStore *store,const char *line,char *response,size_t response_size);
int store_replay(KVStore *store);
#endif
