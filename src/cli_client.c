#define _POSIX_C_SOURCE 200809L
#include "protocol.h"
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define DEFAULT_PORT 6380
static int connect_server(const char*host,int port){int fd=socket(AF_INET,SOCK_STREAM,0);if(fd<0)return -1;struct sockaddr_in a={0};a.sin_family=AF_INET;a.sin_port=htons((uint16_t)port);if(inet_pton(AF_INET,host,&a.sin_addr)!=1){close(fd);return -1;}if(connect(fd,(struct sockaddr*)&a,sizeof(a))<0){close(fd);return -1;}return fd;}
int main(int argc,char**argv){const char*host="127.0.0.1";int port=DEFAULT_PORT;if(argc>1)host=argv[1];if(argc>2){char*e;long p=strtol(argv[2],&e,10);if(*e||p<1||p>65535){fprintf(stderr,"Invalid port\n");return 1;}port=(int)p;}if(argc>3){fprintf(stderr,"Usage: %s [host] [port]\n",argv[0]);return 1;}int fd=connect_server(host,port);if(fd<0){perror("connect");return 1;}printf("Connected to %s:%d. Type commands; QUIT exits.\n",host,port);char line[PROTOCOL_MAX_LINE],response[PROTOCOL_MAX_LINE+128];while(fgets(line,sizeof(line),stdin)){size_t n=strlen(line);if(n==sizeof(line)-1&&line[n-1]!='\n'){int c;while((c=getchar())!='\n'&&c!=EOF){} }if(protocol_send(fd,line)!=0){fprintf(stderr,"send failed\n");break;}int r=protocol_read_line(fd,response,sizeof(response));if(r<=0){fprintf(stderr,"server disconnected\n");break;}printf("%s\n",response);if(strncmp(response,"BYE",3)==0)break;}close(fd);return 0;}
