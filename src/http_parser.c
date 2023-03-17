#include "http_parser.h"

#include <stdio.h>
#include <string.h>

struct server *allserver = NULL;

void removeSpaces(char *str1)
{
    char *str2;
    str2 = str1;
    while (*str2 == ' ')
        str2++;
    if (str2 != str1)
        memmove(str1, str2, strlen(str2) + 1);
}

int parse(const char *file)
{
    int i = 0;
    FILE *fin = fopen(file, "r");
    char name[128];
    char val[128];
    char buffer[100];
    fgets(buffer, 100, fin);
    struct server_glob *globnew = malloc(sizeof(struct server_glob));
    while (fscanf(fin, "%127[^=[]=%127[^\n]%*c", name, val) == 2)
    {
        if (strcmp(name, "log_file ") == 0)
        {
            removeSpaces(val);
            strcpy(globnew->file_log, val);
            // printf("%s\n", globnew->file_log);
        }
        if (strcmp(name, "pid_file ") == 0)
        {
            removeSpaces(val);
            strcpy(globnew->file_pid, val);
            i++;
            // printf("%s\n", globnew->file_pid);
        }
        if (strcmp(name, "log ") == 0)
        {
            removeSpaces(val);
            if (strcmp(val, "true") == 0)
            {
                globnew->log = true;
            }
            else
            {
                globnew->log = false;
            }
            // printf("%d\n", globnew->log);
        }
        if (strchr(name, '\n'))
        {
            break;
        }
    }
    struct server_host *head = malloc(sizeof(struct server_host));
    struct server_host *host = head;
    int a = 0;
    while (fscanf(fin, "%127[^=]=%127[^\n]%*c", name, val) == 2)
    {
        // printf("%s\n\n", name);
        if (strstr(name, "[[vhosts]]\nserver_name "))
        {
            // printf("%s", val);
            removeSpaces(val);
            struct server_host *new_host = malloc(sizeof(struct server_host));
            new_host->next = NULL;
            host->next = new_host;
            host = host->next;
            strcpy(host->server_name, val);
            if ((a == 0 || a == 1) && i <= 5)
            {
                a++;
                if (a == 1)
                {
                    i++;
                }
            }
            continue;
        }
        else if (strcmp(name, "ip ") == 0)
        {
            removeSpaces(val);
            strcpy(host->ip, val);
            if (a == 1)
            {
                i++;
            }
            // printf("%s\n", host->ip);
        }
        else if (strcmp(name, "root_dir ") == 0)
        {
            removeSpaces(val);
            strcpy(host->dir_root, val);
            if (a == 1)
            {
                i++;
            }
        }
        else if (strcmp(name, "port ") == 0)
        {
            removeSpaces(val);
            strcpy(host->port, val);
            if (a == 1)
            {
                i++;
            }
        }
        if (strchr(name, '\n'))
        {
            struct server_host *new_host = malloc(sizeof(struct server_host));
            new_host->next = NULL;
            host->next = new_host;
            host = host->next;
        }
    }
    // printf("%s\n",host->server_name);
    host = head->next;
    free(head);
    allserver = malloc(sizeof(struct server));
    allserver->global = globnew;
    allserver->hosts = host;
    fclose(fin);
    // printall(allserver);
    return i;
}

/*void free_server(struct server *all)
{
    free(all->global);
    struct server_host *tmp;
    while (all->hosts)
    {
        tmp = all->hosts;
        all->hosts = all->hosts->next;
        free(tmp);
    }
    free(all);
}

void printall(struct server *all)
{
    printf("GLOBAL\n");
    printf("file_pid : %s\n", all->global->file_pid);
    printf("file_log : %s\n", all->global->file_log);
    printf("%d\n", all->global->log);
    printf("\n");
    printf("VHOSTS\n");
    while (all->hosts != NULL)
    {
        printf("server name : %s\n", all->hosts->server_name);
        printf("IP : %s\n", all->hosts->ip);
        printf("directory : %s\n", all->hosts->dir_root);
        printf("PORT : %s\n\n", all->hosts->port);
        if (all->hosts->next != NULL)
        {
            printf("VHOSTS\n");
        }
        all->hosts = all->hosts->next;
    }
    free_server(all);
}*/
