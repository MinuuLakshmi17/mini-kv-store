#define _POSIX_C_SOURCE 200809L
#include "client_handler.h"
#include "protocol.h"
#include <stdlib.h>
#include <unistd.h>

void *client_handler_run(void *arg){
    ClientContext*ctx=arg;if(!ctx)return NULL;int fd=ctx->fd;KVStore*s=ctx->store;free(ctx);
    char line[PROTOCOL_MAX_LINE],response[PROTOCOL_MAX_LINE+128];
    for(;;){int r=protocol_read_line(fd,line,sizeof(line));if(r<=0)break;if(r==-2){protocol_send(fd,"ERR command too long\n");continue;}int rc=store_execute(s,line,response,sizeof(response));if(protocol_send(fd,response)!=0)break;if(rc==1)break;}
    close(fd);return NULL;
}
