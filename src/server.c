#include "server.h"

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "http_parser.h"

volatile sig_atomic_t done = 1;
extern struct server *allserver;

void strip(char *s)
{
    char *p2 = s;
    while (*s != '\0')
    {
        if (*s != '\n' && *s != '\r')
            *p2++ = *s++;
        else
            ++s;
    }
    *p2 = '\0';
}

struct requesthttp *fillRequest(char *buffer)
{
    // int i = 0;
    struct requesthttp *new = malloc(sizeof(struct requesthttp));
    char get[100] = { 0 };
    char file[100] = { 0 };
    char version[100] = { 0 };
    sscanf(buffer, "%s %s %[^\r\n]", get, file, version);
    strcpy(new->get, get);
    strcpy(new->file, file);
    strcpy(new->version, version);
    // printf("%s,%s,%s\n", get, file, version);
    return new;
}

char *check_ferror(char *filename)
{
    struct stat sb;
    int a = stat(filename, &sb);
    if (!a)
    {
        return "200";
    }
    else if (a == -1 && errno == ENOENT)
    {
        return "404";
    }
    else
    {
        return "403";
    }
}

char *mess_error(char *error)
{
    if (strcmp("400", error) == 0)
    {
        return "Bad Request\n";
    }
    else if (strcmp("403", error) == 0)
    {
        return "Forbidden\n";
    }
    else if (strcmp("404", error) == 0)
    {
        return "Not Found\n";
    }
    else if (strcmp("405", error) == 0)
    {
        return "Method Not Allowed\n";
    }
    else if (strcmp("406", error) == 0)
    {
        return "Not Acceptable\n";
    }
    else if (strcmp("501", error) == 0)
    {
        return "Internal Server Error\n";
    }
    else if (strcmp("200", error) == 0)
    {
        return "OK\n";
    }
    else
    {
        return "Internal Error";
    }
}

void term(int signum)
{
    if (signum == SIGTERM)
    {
        done = 0;
    }
    else if (signum == SIGSTOP)
    {
        done = 0;
    }
    else if (signum == SIGINT)
    {
        done = 0;
    }
}

void init_server(void)
{
    char *ip = allserver->hosts->ip;
    char *port = allserver->hosts->port;
    struct addrinfo hints;
    struct addrinfo *addr_list, *addr;
    int socket_id = 0, client_socket_id;
    int res;
    pid_t child;
    // Get addresses list
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    res = getaddrinfo(ip, port, &hints, &addr_list);

    // If error, exit the program
    if (res != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(res));
        exit(0);
    }

    // Try to connect to each adress returned by getaddrinfo()
    for (addr = addr_list; addr != NULL; addr = addr->ai_next)
    {
        // Socket creation
        socket_id =
            socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);

        // If error, try next adress
        if (socket_id == -1)
            continue;

        // Set options on socket
        int enable = 1;
        if (setsockopt(socket_id, SOL_SOCKET, SO_REUSEADDR, &enable,
                       sizeof(int))
            == -1)
            perror("setsockopt(SO_REUSEADDR) failed");

        // Bind a name to a socket, exit if no error
        if (bind(socket_id, addr->ai_addr, addr->ai_addrlen) == 0)
            break;

        // Close current not connected socket
        close(socket_id);
    }

    // addr_list freed
    freeaddrinfo(addr_list);

    // If no address works, exit the program
    if (addr == NULL)
    {
        fprintf(stderr, "Could not bind\n");
        exit(0);
    }

    // Specify that the socket can be used to accept incoming connections
    if (listen(socket_id, 5) == -1)
    {
        fprintf(stderr, "Cannot wait\n");
        exit(0);
    }

    printf("Waiting for connections...\n");

    // On SIGCHLD signal, call signal to properly exit the forked process

    // Allow multiple connections
    if (allserver->daemon == 0)
    {
        child = fork();
        FILE *somefile = fopen(allserver->global->file_pid, "w");
        fprintf(somefile, "%d", getpid());
        fclose(somefile);
        if (child == -1)
        {
            fprintf(stderr, "Fork error\n");
            exit(0);
        }
        if (child > 0)
        {
            close(STDIN_FILENO);
            close(STDOUT_FILENO);
            close(STDERR_FILENO);
            exit(0);
        }
        struct sigaction action;
        memset(&action, 0, sizeof(struct sigaction));
        action.sa_handler = term;
        sigaction(SIGTERM, &action, NULL);
        sigaction(SIGINT, &action, NULL);
        sigaction(SIGSTOP, &action, NULL);
    }

    while (done == 1)
    {
        // Accept connection from a client and exit the program in case of error
        client_socket_id = accept(socket_id, NULL, NULL);
        if (client_socket_id == -1)
        {
            fprintf(stderr, "Cannot connect\n");
            exit(0);
        }
        // Close server sockets
        // close(socket_id);

        char buffer[1024];
        recv(client_socket_id, buffer, 1024, 0);
        size_t i = 0;
        char buffer1[1024];
        // printf("%s\n", buffer);
        if (buffer[i] == '\0')
        {
            exit(0);
        }
        while (buffer[i] != '\n' && buffer[i] != '\0')
        {
            buffer1[i] = buffer[i];
            i++;
        }
        buffer1[i] = '\0';
        time_t timestamp = time(NULL);
        struct tm *pTime = localtime(&timestamp);
        char buffer2[31];
        strftime(buffer2, 31, "%a, %d %b %Y %X GMT\n", pTime);
        struct requesthttp *new = fillRequest(buffer1);
        if (new->get[0] == 0 || new->file[0] == 0 || new->version[0] == 0)
        {
            char prb[1024];
            sprintf(prb, "HTTP/1.1 400 Bad Request\n");
            send(client_socket_id, prb, strlen(prb), 0);
            char body[4096];
            long int size = 0;
            sprintf(body,
                    "Server : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n",
                    allserver->hosts->server_name, buffer2, size);
            send(client_socket_id, body, strlen(body), 0);
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
            free(new);
        }
        else if (strcmp(new->get, "HEAD") != 0 && strcmp(new->get, "GET") != 0)
        {
            char prb[1024];
            sprintf(prb, "HTTP/1.1 405 Method Not Allowed\n");
            send(client_socket_id, prb, strlen(prb), 0);
            char body[4096];
            long int size = 0;
            sprintf(body,
                    "Server : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n",
                    allserver->hosts->server_name, buffer2, size);
            send(client_socket_id, body, strlen(body), 0);
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
            free(new);
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
        }
        else if (strcmp(new->version, "HTTP/1.1") != 0)
        {
            char prb[1024];
            sprintf(prb, "HTTP/1.1 505 HTTP Version Not Supported\n");
            send(client_socket_id, prb, strlen(prb), 0);
            char body[4096];
            long int size = 0;
            sprintf(body,
                    "Server : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n",
                    allserver->hosts->server_name, buffer2, size);
            send(client_socket_id, body, strlen(body), 0);
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
            free(new);
        }
        /*else if (strcmp(new->get, "HEAD") == 0)
        {
            char body[4096];
            long int size = 0;
            sprintf(body,
                    "%s 200 OK\nServer : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n",
                    new->version, allserver->hosts->server_name, buffer2, size);
            send(client_socket_id, body, strlen(body), 0);
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
            free(new);
        }*/

        else
        {
            // printf("ok\n");
            // rip(new->file);
            char firstline[4096];
            // w->version[strlen(new->version) - 1] = '\0';
            char path[4096];
            if (allserver->hosts
                    ->dir_root[strlen(allserver->hosts->dir_root) - 1]
                == '.')
            {
                allserver->hosts
                    ->dir_root[strlen(allserver->hosts->dir_root) - 1] = '\0';
            }

            sprintf(path, "./%s%s", allserver->hosts->dir_root, new->file);
            char number[200];
            strcpy(number, check_ferror(path));
            char type[200];
            strcpy(type, mess_error(number));

            FILE *fin = fopen(path, "r");
            char body[1024];
            long int size = 0;
            fclose(fin);
            if (fin != NULL)
            {
                struct stat st;
                stat(path, &st);
                size = st.st_size;
                if (strcmp(new->get, "HEAD") == 0)
                {
                    sprintf(firstline,
                    "%s %s %sServer : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n",
                    new->version, number, type,
                    allserver->hosts->server_name, buffer2, size);
                }
                else
                {
                    sprintf(firstline,
                    "%s %s %sServer : %s\nDate: %sContent-Length: "
                    "%lu\nConnection: Close\n\n",
                    new->version, number, type,
                    allserver->hosts->server_name, buffer2, size);
                }
                send(client_socket_id, firstline, strlen(firstline), 0);
            }
            else
            {
                sprintf(firstline,
                        "%s %s %sServer : %s\nDate: %sContent-Length: "
                        "%lu\nConnection: Close\n",
                        new->version, number, type,
                        allserver->hosts->server_name, buffer2, size);
                send(client_socket_id, firstline, strlen(firstline), 0);
            }
            if (fin != NULL && strstr(new->get, "GET"))
            {
                while (fgets(body, 1024, fin))
                {
                    send(client_socket_id, body, strlen(body), 0);
                }
            }
            if (fin != NULL)
            {
                fclose(fin);
            }
            close(client_socket_id);
            memset(&buffer, 0, sizeof(buffer));
            if (new != NULL)
            {
                free(new);
            }

            // Close client sockets
            // close(client_socket_id);

            // printf("Close connection (pid = %i)\n", getpid());

            // Exit the child process
        }

        // Parent process
        // Close client socket
    }

    // Close server sockets
    // close(socket_id);
}
