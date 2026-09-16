#define _POSIX_C_SOURCE 200809L
#include "protocol.h"
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

int protocol_read_line(int fd,char *buffer,size_t capacity){
    if(!buffer||capacity<2)return -1;
    size_t used=0;
    while(used+1<capacity){
        char c;ssize_t n=recv(fd,&c,1,0);
        if(n==0)return used? (buffer[used]='\0',1):0;
        if(n<0){if(errno==EINTR)continue;return -1;}
        if(c=='\n'){buffer[used]='\0';return 1;}
        if(c=='\r')continue;
        buffer[used++]=c;
    }
    buffer[used]='\0';
    return -2;
}

int protocol_send(int fd,const char *response){
    if(!response) return -1;
    size_t left=0;
    while(response[left]) left++;
    size_t sent=0;while(sent<left){ssize_t n=send(fd,response+sent,left-sent,MSG_NOSIGNAL);if(n<0){if(errno==EINTR)continue;return -1;}if(n==0)return -1;sent+=(size_t)n;}return 0;
}
