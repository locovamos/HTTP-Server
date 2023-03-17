#ifndef http_parser_h
#define http_parser_h

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse(const char *file);
// void printall(struct server *all);
// void free_server(struct server *all);

struct server
{
    struct server_glob *global;
    struct server_host *hosts;
    long int daemon;
};

struct server_glob
{
    char file_pid[1024];
    char file_log[1024];
    bool log;
    int pid;
};

struct server_host
{
    char server_name[1024];
    char port[1024];
    char ip[1024];
    char dir_root[2048];
    struct server_host *next;
};

#endif /* HTTP_PARSER_H */
