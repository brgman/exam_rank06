
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/select.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    int i;

    *msg = 0;
    if (*buf == 0)
        return (0);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n')
        {
            newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
            if (newbuf == 0)
                return (-1);
            strcpy(newbuf, *buf + i + 1);
            *msg = *buf;
            (*msg)[i + 1] = 0;
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    return (0);
}

char *str_join(char *buf, char *add)
{
    char *newbuf;
    int len;

    if (buf == 0)
        len = 0;
    else
        len = strlen(buf);
    newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
    if (newbuf == 0)
        return (0);
    newbuf[0] = 0;
    if (buf != 0)
        strcat(newbuf, buf);
    free(buf);
    strcat(newbuf, add);
    return (newbuf);
}

typedef struct s_client
{
    int id;
    int fd;
    struct s_client *next;
} t_client;

static t_client *clients = NULL;
static int sockfd = -1;

void free_clients(t_client *clients)
{
    t_client *curr = clients;
    while (curr)
    {
        t_client *tmp = curr;
        curr = curr->next;
        close(tmp->fd);
        free(tmp);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Wrong number of arguments\n", 26);
        exit(1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        write(STDERR_FILENO, "Fatal error\n", 12);
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        write(STDERR_FILENO, "Fatal error\n", 12);
        close(sockfd);
        exit(1);
    }
    if (listen(sockfd, 10) != 0)
    {
        write(STDERR_FILENO, "Fatal error\n", 12);
        close(sockfd);
        exit(1);
    }

    int next_id = 0;
    int max_fd = sockfd;
    fd_set read_fds;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        for (t_client *tmp = clients; tmp; tmp = tmp->next)
        {
            FD_SET(tmp->fd, &read_fds);
            if (tmp->fd > max_fd)
                max_fd = tmp->fd;
        }
        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0)
            continue;
        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in cli_addr;
            socklen_t len = sizeof(cli_addr);
            int client_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);
            if (client_fd >= 0)
            {
                t_client *new_client = calloc(1, sizeof(t_client));
                if (!new_client)
                {
                    close(client_fd);
                    continue;
                }
                new_client->fd = client_fd;
                new_client->id = next_id++;
                new_client->next = clients;
                clients = new_client;
                if (client_fd > max_fd)
                    max_fd = client_fd;
                char msg[100];
                sprintf(msg, "server: client %d just arrived\n", new_client->id);
                for (t_client *tmp = clients; tmp; tmp = tmp->next)
                    if (tmp->fd != client_fd)
                        send(tmp->fd, msg, strlen(msg), 0);
            }
        }

        t_client *curr = clients;
        t_client *prev = NULL;
        while (curr)
        {
            if (FD_ISSET(curr->fd, &read_fds))
            {
                char buf[1024];
                ssize_t ret = recv(curr->fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0)
                {
                    char msg[100];
                    sprintf(msg, "server: client %d just left\n", curr->id);
                    for (t_client *other = clients; other; other = other->next)
                    {
                        if (other->fd != curr->fd)
                            send(other->fd, msg, strlen(msg), 0);
                    }
                    if (prev)
                        prev->next = curr->next;
                    else
                        clients = curr->next;
                    t_client *tmp = curr;
                    curr = curr->next;
                    close(tmp->fd);
                    free(tmp);
                    max_fd = sockfd;
                    for (t_client *tmp = clients; tmp; tmp = tmp->next)
                        if (tmp->fd > max_fd)
                            max_fd = tmp->fd;
                    continue;
                }
                buf[ret] = '\0';
                char msg[1100];
                sprintf(msg, sizeof(msg), "client %d: %s", curr->id, buf);
                for (t_client *other = clients; other; other = other->next)
                    if (other->fd != curr->fd)
                        send(other->fd, msg, strlen(msg), 0);
            }
            prev = curr;
            curr = curr->next;
        }
    }
    return 0;
}