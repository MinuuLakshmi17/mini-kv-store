#define _POSIX_C_SOURCE 200809L
#include "client_handler.h"
#include "commands.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define DEFAULT_PORT 6380
#define BACKLOG 64
#define AOF_PATH "mini-kv.aof"
static volatile sig_atomic_t running=1;
static void stop_server(int sig){(void)sig;running=0;}
static int make_server_socket(int port){int fd=socket(AF_INET,SOCK_STREAM,0);if(fd<0)return -1;int one=1;setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));struct sockaddr_in addr={0};addr.sin_family=AF_INET;addr.sin_addr.s_addr=htonl(INADDR_ANY);addr.sin_port=htons((uint16_t)port);if(bind(fd,(struct sockaddr*)&addr,sizeof(addr))<0){close(fd);return -1;}if(listen(fd,BACKLOG)<0){close(fd);return -1;}return fd;}
int main(int argc,char**argv){int port=DEFAULT_PORT;if(argc>2){fprintf(stderr,"Usage: %s [port]\n",argv[0]);return 1;}if(argc==2){char*end;long p=strtol(argv[1],&end,10);if(*end||p<1||p>65535){fprintf(stderr,"Invalid port\n");return 1;}port=(int)p;}signal(SIGINT,stop_server);signal(SIGTERM,stop_server);signal(SIGPIPE,SIG_IGN);KVStore*store=store_create(16,AOF_PATH);if(!store){perror("store_create");return 1;}if(store_replay(store)!=0){fprintf(stderr,"Failed to replay AOF '%s'\n",AOF_PATH);store_destroy(store);return 1;}int server_fd=make_server_socket(port);if(server_fd<0){perror("server socket");store_destroy(store);return 1;}printf("mini-kv-server listening on port %d\n",port);fflush(stdout);while(running){struct sockaddr_in client;socklen_t len=sizeof(client);int fd=accept(server_fd,(struct sockaddr*)&client,&len);if(fd<0){if(errno==EINTR)continue;perror("accept");break;}ClientContext*ctx=malloc(sizeof(*ctx));if(!ctx){close(fd);continue;}ctx->fd=fd;ctx->store=store;pthread_t tid;if(pthread_create(&tid,NULL,client_handler_run,ctx)!=0){close(fd);free(ctx);continue;}pthread_detach(tid);}close(server_fd);store_destroy(store);return 0;}
