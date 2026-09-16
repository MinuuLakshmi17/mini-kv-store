#define _POSIX_C_SOURCE 200809L
#include "commands.h"
#include "persistence.h"
#include <ctype.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 16
#define MAX_LINE 4096
#define MAX_TOKENS 3

typedef struct ExpiryEntry { char *key; time_t expires_at; struct ExpiryEntry *next; } ExpiryEntry;
struct KVStore { HashTable *table; ExpiryEntry *expiries; pthread_mutex_t lock; char *aof_path; int replaying; };

static void setresp(char *out,size_t n,const char *fmt,...){va_list ap;va_start(ap,fmt);vsnprintf(out,n,fmt,ap);va_end(ap);}
static ExpiryEntry *find_expiry(KVStore*s,const char*k){for(ExpiryEntry*e=s->expiries;e;e=e->next)if(strcmp(e->key,k)==0)return e;return NULL;}
static void remove_expiry(KVStore*s,const char*k){ExpiryEntry*e=s->expiries,*p=NULL;while(e){if(strcmp(e->key,k)==0){if(p)p->next=e->next;else s->expiries=e->next;free(e->key);free(e);return;}p=e;e=e->next;}}
static int set_expiry(KVStore*s,const char*k,time_t when){ExpiryEntry*e=find_expiry(s,k);if(e){e->expires_at=when;return 0;}e=malloc(sizeof(*e));if(!e)return -1;e->key=strdup(k);if(!e->key){free(e);return -1;}e->expires_at=when;e->next=s->expiries;s->expiries=e;return 0;}
static void purge_expired(KVStore*s,const char*k){ExpiryEntry*e=find_expiry(s,k);if(e&&e->expires_at<=time(NULL)){hashtable_delete(s->table,k);remove_expiry(s,k);}}
static int parse(char *line, char **tok) {
    int n = 0;
    char *p = line;
    while (*p && n < MAX_TOKENS) {
        while (isspace((unsigned char)*p)) p++;
        if (!*p) break;
        tok[n++] = p;
        while (*p && !isspace((unsigned char)*p)) p++;
        if (*p) *p++ = '\0';
    }
    if (*p) return -1;
    return n;
}

static int parse_set(char *line, char **command, char **key, char **value) {
    char *p = line;
    while (isspace((unsigned char)*p)) p++;
    if (!*p) return 0;
    *command = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    if (*p) *p++ = '\0';
    while (isspace((unsigned char)*p)) p++;
    if (!*p) return 0;
    *key = p;
    while (*p && !isspace((unsigned char)*p)) p++;
    if (*p) *p++ = '\0';
    while (isspace((unsigned char)*p)) p++;
    if (!*p) return 0;
    *value = p;
    char *end = p + strlen(p);
    while (end > p && isspace((unsigned char)end[-1])) *--end = '\0';
    return *value[0] ? 3 : 0;
}

static int persist_if_needed(KVStore*s,const char*record){return s->replaying?0:persistence_append(s->aof_path,record);}

KVStore *store_create(size_t capacity,const char*aof_path){KVStore*s=calloc(1,sizeof(*s));if(!s)return NULL;s->table=hashtable_create(capacity?capacity:INITIAL_CAPACITY);if(!s->table){free(s);return NULL;}if(pthread_mutex_init(&s->lock,NULL)!=0){hashtable_destroy(s->table);free(s);return NULL;}if(aof_path){s->aof_path=strdup(aof_path);if(!s->aof_path){pthread_mutex_destroy(&s->lock);hashtable_destroy(s->table);free(s);return NULL;}}return s;}
void store_destroy(KVStore*s){if(!s)return;ExpiryEntry*e=s->expiries;while(e){ExpiryEntry*n=e->next;free(e->key);free(e);e=n;}free(s->aof_path);pthread_mutex_destroy(&s->lock);hashtable_destroy(s->table);free(s);}

int store_execute(KVStore*s,const char*input,char*out,size_t outsz){
    if(!s||!input||!out||!outsz)return -1;
    char line[MAX_LINE];snprintf(line,sizeof(line),"%s",input);line[strcspn(line,"\r\n")]='\0';
    char *tok[MAX_TOKENS] = {0};
    int n = 0;
    char command_name[32] = {0};
    size_t command_len = 0;
    char *scan = line;
    while (isspace((unsigned char)*scan)) scan++;
    while (scan[command_len] && !isspace((unsigned char)scan[command_len]) && command_len + 1 < sizeof(command_name)) {
        command_name[command_len] = (char)toupper((unsigned char)scan[command_len]);
        command_len++;
    }

    if (strcmp(command_name, "SET") == 0) {
        char *cmd = NULL, *key = NULL, *value = NULL;
        if (parse_set(line, &cmd, &key, &value) != 3) {
            snprintf(out, outsz, "ERR wrong number of arguments\n");
            return 0;
        }
        tok[0] = cmd;
        tok[1] = key;
        tok[2] = value;
        n = 3;
    } else {
        n = parse(line, tok);
        if (n < 0) {
            snprintf(out, outsz, "ERR too many arguments\n");
            return 0;
        }
        if (!n) {
            snprintf(out, outsz, "ERR empty command\n");
            return 0;
        }
    }
    for(char*p=tok[0];*p;p++)*p=(char)toupper((unsigned char)*p);
    pthread_mutex_lock(&s->lock);
    if(n>=2)purge_expired(s,tok[1]);
    int rc=0;
    if(strcmp(tok[0],"PING")==0){if(n!=1)snprintf(out,outsz,"ERR wrong number of arguments\n");else snprintf(out,outsz,"PONG\n");}
    else if(strcmp(tok[0],"GET")==0){if(n!=2)snprintf(out,outsz,"ERR wrong number of arguments\n");else{const char*v=hashtable_get(s->table,tok[1]);snprintf(out,outsz,v?"%s\n":"(nil)\n",v?v:"");}}
    else if(strcmp(tok[0],"SET")==0){if(n!=3)snprintf(out,outsz,"ERR wrong number of arguments\n");else if(hashtable_set(s->table,tok[1],tok[2])!=0)snprintf(out,outsz,"ERR out of memory\n");else{remove_expiry(s,tok[1]);char rec[MAX_LINE];setresp(rec,sizeof(rec),"SET %s %s",tok[1],tok[2]);rc=persist_if_needed(s,rec);snprintf(out,outsz,rc==0?"OK\n":"ERR persistence\n");}}
    else if(strcmp(tok[0],"DEL")==0){if(n!=2)snprintf(out,outsz,"ERR wrong number of arguments\n");else{int existed=hashtable_delete(s->table,tok[1]);remove_expiry(s,tok[1]);char rec[MAX_LINE];setresp(rec,sizeof(rec),"DEL %s",tok[1]);rc=persist_if_needed(s,rec);snprintf(out,outsz,rc==0?"%d\n":"ERR persistence\n",existed);}}
    else if(strcmp(tok[0],"EXISTS")==0){if(n!=2)snprintf(out,outsz,"ERR wrong number of arguments\n");else snprintf(out,outsz,"%d\n",hashtable_exists(s->table,tok[1]));}
    else if(strcmp(tok[0],"EXPIRE")==0){if(n!=3)snprintf(out,outsz,"ERR wrong number of arguments\n");else{char*end;errno=0;long sec=strtol(tok[2],&end,10);if(errno||*end||sec<0)snprintf(out,outsz,"ERR invalid seconds\n");else if(!hashtable_exists(s->table,tok[1]))snprintf(out,outsz,"0\n");else{time_t when=time(NULL)+(time_t)sec;if(set_expiry(s,tok[1],when)!=0)snprintf(out,outsz,"ERR out of memory\n");else{char rec[MAX_LINE];setresp(rec,sizeof(rec),"EXPIREAT %s %lld",tok[1],(long long)when);rc=persist_if_needed(s,rec);snprintf(out,outsz,rc==0?"1\n":"ERR persistence\n");}}}}
    else if(strcmp(tok[0],"EXPIREAT")==0){if(n!=3)snprintf(out,outsz,"ERR wrong number of arguments\n");else{char*end;errno=0;long long stamp=strtoll(tok[2],&end,10);if(errno||*end||stamp<0)snprintf(out,outsz,"ERR invalid timestamp\n");else if(!hashtable_exists(s->table,tok[1]))snprintf(out,outsz,"0\n");else{if(set_expiry(s,tok[1],(time_t)stamp)!=0)snprintf(out,outsz,"ERR out of memory\n");else snprintf(out,outsz,"1\n");}}}
    else if(strcmp(tok[0],"QUIT")==0){if(n!=1)snprintf(out,outsz,"ERR wrong number of arguments\n");else{snprintf(out,outsz,"BYE\n");rc=1;}}
    else snprintf(out,outsz,"ERR unknown command\n");
    pthread_mutex_unlock(&s->lock);return rc;
}

int store_replay(KVStore*s){if(!s||!s->aof_path)return 0;FILE*f=fopen(s->aof_path,"r");if(!f)return errno==ENOENT?0:-1;char line[MAX_LINE],out[256];s->replaying=1;while(fgets(line,sizeof(line),f)){int rc=store_execute(s,line,out,sizeof(out));if(rc<0){s->replaying=0;fclose(f);return -1;}}s->replaying=0;fclose(f);return 0;}
