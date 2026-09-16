#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stddef.h>
#define PROTOCOL_MAX_LINE 4096
int protocol_read_line(int fd,char *buffer,size_t capacity);
int protocol_send(int fd,const char *response);
#endif
