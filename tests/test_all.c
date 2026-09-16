#define _POSIX_C_SOURCE 200809L
#include "commands.h"
#include "hashtable.h"
#include "persistence.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int pass=0,fail=0;
#define CHECK(x,msg) do{if(x){printf("  PASS: %s\n",msg);pass++;}else{printf("  FAIL: %s\n",msg);fail++;return 0;}}while(0)
static int test_hash(void){printf("Test hash table...\n");HashTable*t=hashtable_create(4);CHECK(t,"create");CHECK(hashtable_set(t,"name","Minuu Shanmugam")==0,"set");CHECK(strcmp(hashtable_get(t,"name"),"Minuu Shanmugam")==0,"get");CHECK(hashtable_set(t,"name","Computer Science")==0,"update");CHECK(strcmp(hashtable_get(t,"name"),"Computer Science")==0,"updated value");CHECK(hashtable_exists(t,"name"),"exists");CHECK(hashtable_delete(t,"name")==1,"delete");CHECK(!hashtable_exists(t,"name"),"missing after delete");for(int i=0;i<20;i++){char k[32],v[32];snprintf(k,sizeof(k),"key%d",i);snprintf(v,sizeof(v),"value%d",i);CHECK(hashtable_set(t,k,v)==0,"insert/resize");}CHECK(t->capacity>=32,"resize occurred");for(int i=0;i<20;i++){char k[32],v[32];snprintf(k,sizeof(k),"key%d",i);snprintf(v,sizeof(v),"value%d",i);CHECK(strcmp(hashtable_get(t,k),v)==0,"rehash preserved value");}hashtable_destroy(t);return 1;}
static int run(KVStore*s,const char*cmd,const char*expected){char out[512];int rc=store_execute(s,cmd,out,sizeof(out));CHECK(strcmp(out,expected)==0,cmd);return rc>=0;}
static int test_commands(void){printf("Test commands...\n");const char*aof="test-mini-kv.aof";unlink(aof);KVStore*s=store_create(4,aof);CHECK(s,"store create");run(s,"PING","PONG\n");run(s,"SET name Minuu","OK\n");run(s,"GET name","Minuu\n");run(s,"EXISTS name","1\n");run(s,"DEL name","1\n");run(s,"GET name","(nil)\n");run(s,"DEL name","0\n");run(s,"SET university Binghamton","OK\n");run(s,"EXPIRE university 1","1\n");run(s,"EXPIREAT university 9999999999","1\n");run(s,"EXPIRE missing 10","0\n");run(s,"NOPE","ERR unknown command\n");run(s,"GET","ERR wrong number of arguments\n");store_destroy(s);unlink(aof);return 1;}
static int test_persistence(void){printf("Test persistence/replay...\n");const char*aof="test-replay.aof";unlink(aof);KVStore*s=store_create(8,aof);CHECK(s,"store create");run(s,"SET name Minuu","OK\n");run(s,"SET program ComputerScience","OK\n");run(s,"EXPIRE program 60","1\n");store_destroy(s);s=store_create(8,aof);CHECK(store_replay(s)==0,"replay succeeds");run(s,"GET name","Minuu\n");run(s,"GET program","ComputerScience\n");store_destroy(s);unlink(aof);return 1;}
int main(void){int tests=3;int ok=0;ok+=test_hash();ok+=test_commands();ok+=test_persistence();printf("\nSuite: %d/%d test groups passed; %d checks passed; %d checks failed\n",ok,tests,pass,fail);return fail?1:0;}
