#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "http_parser.h"
#include "server.h"

extern struct server *allserver;

int main(int argc, char *argv[])
{
    if (argc >= 5 && strcmp(argv[3], "stop") == 0 && strcmp(argv[2], "-a") == 0)
    {
        parse(argv[4]);
        FILE *fin = fopen(allserver->global->file_pid, "r");
        if (fin == NULL)
        {
            return 1;
        }
        char buffer[100];
        if (fgets(buffer, 100, fin))
        {
            int i;
            sscanf(buffer, "%d", &i);
            kill(i, SIGTERM);
        }
        return 0;
    }
    else if (argc >= 5 && strcmp(argv[3], "start") == 0
             && strcmp(argv[2], "-a") == 0)
    {
        parse(argv[4]);
        allserver->daemon = 1;
        init_server();
        return 0;
    }
    else if (argc >= 4 && strcmp(argv[2], "start") == 0
             && strcmp(argv[1], "-a") == 0)
    {
        parse(argv[3]);
        allserver->daemon = 1;
        init_server();
        return 0;
    }
    else if (argc >= 4 && strcmp(argv[2], "stop") == 0
             && strcmp(argv[1], "-a") == 0 && argc > 4)
    {
        parse(argv[3]);
        FILE *fin = fopen(allserver->global->file_pid, "r");
        if (fin == NULL)
        {
            return 1;
        }
        char buffer[100];
        if (fgets(buffer, 100, fin))
        {
            int i;
            sscanf(buffer, "%d", &i);
            kill(i, SIGTERM);
        }
        return 0;
    }
    else if (argc >= 3 && strcmp(argv[1], "stop") == 0)
    {
        parse(argv[2]);
        FILE *fin = fopen(allserver->global->file_pid, "r");
        if (fin == NULL)
        {
            return 1;
        }
        char buffer[100];
        if (fgets(buffer, 100, fin))
        {
            int i;
            sscanf(buffer, "%d", &i);
            kill(i, SIGTERM);
        }
        return 0;
    }
    else if (argc >= 3 && strcmp(argv[1], "start") == 0)
    {
        parse(argv[2]);
        allserver->daemon = 0;
        init_server();
        return 0;
    }
    else if (argc >= 4 && strcmp(argv[2], "stop") == 0)
    {
        init_server();

        parse(argv[3]);
        FILE *fin = fopen(allserver->global->file_pid, "r");
        if (fin == NULL)
        {
            return 1;
        }
        char buffer[100];
        if (fgets(buffer, 100, fin))
        {
            int i;
            sscanf(buffer, "%d", &i);
            kill(i, SIGTERM);
        }
        return 0;
    }
    else if (argc >= 4 && strcmp(argv[2], "start") == 0)
    {
        parse(argv[3]);
        allserver->daemon = 0;
        init_server();
        return 0;
    }
    else if (argc == 2)
    {
        int i = parse(argv[1]);
        printf("%d\n", i);
        if (i == 5)
        {
            allserver->daemon = 1;
            init_server();
            return 0;
        }
        else
        {
            return 1;
        }
    }
    else if (argc == 3 && strstr(argv[1], "--dry-run"))
    {
        int i = parse(argv[2]);
        if (i == 5)
        {
            allserver->daemon = 1;
            init_server();
            return 0;
        }
        else
        {
            return 1;
        }
    }
    return 0;
}
