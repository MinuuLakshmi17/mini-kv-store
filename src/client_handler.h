#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#include "commands.h"
void *client_handler_run(void *arg);
typedef struct { int fd; KVStore *store; } ClientContext;
#endif
