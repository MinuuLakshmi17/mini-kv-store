#define _POSIX_C_SOURCE 200809L
#include "persistence.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int persistence_append(const char *path,const char *record){
    if(!path||!record)return 0;
    FILE*f=fopen(path,"a");if(!f)return -1;
    if(fprintf(f,"%s\n",record)<0){fclose(f);return -1;}
    if(fflush(f)!=0){fclose(f);return -1;}
    if(fsync(fileno(f))!=0){fclose(f);return -1;}
    return fclose(f)==0?0:-1;
}
