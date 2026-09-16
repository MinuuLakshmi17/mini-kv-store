#define _POSIX_C_SOURCE 200809L
#include "hashtable.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#define LOAD_NUM 3
#define LOAD_DEN 4
#define FNV_OFFSET 14695981039346656037ULL
#define FNV_PRIME 1099511628211ULL
static uint64_t hash_string(const char *s){uint64_t h=FNV_OFFSET; for(;*s;s++){h^=(unsigned char)*s; h*=FNV_PRIME;} return h;}
static size_t bucket_index(const HashTable *t,const char *k){return (size_t)(hash_string(k)%t->capacity);}
static int resize_table(HashTable *t,size_t cap){HashEntry **b=calloc(cap,sizeof(*b)); if(!b)return -1; for(size_t i=0;i<t->capacity;i++){HashEntry *e=t->buckets[i]; while(e){HashEntry *n=e->next; size_t j=(size_t)(hash_string(e->key)%cap); e->next=b[j]; b[j]=e; e=n;}} free(t->buckets); t->buckets=b; t->capacity=cap; return 0;}
HashTable *hashtable_create(size_t initial_capacity){if(!initial_capacity)return NULL; HashTable *t=malloc(sizeof(*t)); if(!t)return NULL; t->buckets=calloc(initial_capacity,sizeof(*t->buckets)); if(!t->buckets){free(t);return NULL;} t->capacity=initial_capacity;t->size=0;return t;}
void hashtable_destroy(HashTable *t){if(!t)return; for(size_t i=0;i<t->capacity;i++){HashEntry *e=t->buckets[i];while(e){HashEntry*n=e->next;free(e->key);free(e->value);free(e);e=n;}}free(t->buckets);free(t);}
int hashtable_set(HashTable *t,const char*k,const char*v){if(!t||!k||!v)return -1; size_t i=bucket_index(t,k); for(HashEntry*e=t->buckets[i];e;e=e->next)if(strcmp(e->key,k)==0){char*nv=strdup(v);if(!nv)return -1;free(e->value);e->value=nv;return 0;} if(t->size==SIZE_MAX)return -1; size_t projected=t->size+1; if(projected>SIZE_MAX/LOAD_DEN || t->capacity>SIZE_MAX/LOAD_NUM || projected*LOAD_DEN>t->capacity*LOAD_NUM){if(t->capacity>SIZE_MAX/2)return -1; if(resize_table(t,t->capacity*2)!=0)return -1; i=bucket_index(t,k);} HashEntry*e=malloc(sizeof(*e));if(!e)return -1;e->key=strdup(k);e->value=strdup(v);if(!e->key||!e->value){free(e->key);free(e->value);free(e);return -1;}e->next=t->buckets[i];t->buckets[i]=e;t->size++;return 0;}
const char *hashtable_get(const HashTable*t,const char*k){if(!t||!k)return NULL;size_t i=bucket_index(t,k);for(HashEntry*e=t->buckets[i];e;e=e->next)if(strcmp(e->key,k)==0)return e->value;return NULL;}
int hashtable_delete(HashTable*t,const char*k){if(!t||!k)return 0;size_t i=bucket_index(t,k);HashEntry*e=t->buckets[i],*p=NULL;while(e){if(strcmp(e->key,k)==0){if(p)p->next=e->next;else t->buckets[i]=e->next;free(e->key);free(e->value);free(e);t->size--;return 1;}p=e;e=e->next;}return 0;}
int hashtable_exists(const HashTable*t,const char*k){return hashtable_get(t,k)!=NULL;}
