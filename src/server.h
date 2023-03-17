#ifndef server_h
#define server_h

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

struct requesthttp
{
    char get[1024];
    char file[1024];
    char version[1024];
};

struct responsehttp
{
    char *firstline;
    char *server_type;
    char *date;
    char *content_type;
    char *content_len;
    char *body;
};
struct requesthttp *fillRequest(char *buffer);
void init_server(void);

#endif /* SERVER_H */
