
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

const size_t MAX_MESSAGE_SIZE = 1024 * 1024;

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i;

    *msg = NULL;
    if (*buf == NULL || **buf == '\0')
        return (0);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n' || i >= MAX_MESSAGE_SIZE)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1, strlen(*buf + len) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!*msg)
            {
                free(newbuf);
                return (-1);
            }
            for (size_t j = 0; j < len; j++)
                (*msg)[j] = (*buf)[j];
            (*msg)[len] = '\0';
            free(*buf);
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    if (strlen(*buf) >= MAX_MESSAGE_SIZE)
    {
        *msg = *buf;
        *buf = NULL;
        return (1);
    }
    return (0);
}

char *str_join(char *buf, char *add)
{
    char *newbuf;
    size_t len = buf ? strlen(buf) : 0;
    newbuf = malloc(len + strlen(add) + 1);
    if (!newbuf)
        return (NULL);
    newbuf[0] = '\0';
    if (buf)
        strcat(newbuf, buf);
    free(buf);
    strcat(newbuf, add);
    return (newbuf);
}

typedef struct s_client
{
    int id;
    int fd;
    char *buf;
    struct s_client *next;
} t_client;

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

    t_client *clients = NULL;
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
                    write(STDERR_FILENO, "Fatal error\n", 12);
                    close(client_fd);
                    exit(1);
                }
                new_client->fd = client_fd;
                new_client->id = next_id++;
                new_client->buf = NULL;
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
                char buf[65536]; // Use a reasonable size for recv
                ssize_t ret = recv(curr->fd, buf, sizeof(buf) - 1, 0);
                if (ret <= 0)
                {
                    if (curr->buf)
                    {
                        char *line;
                        while (extract_message(&curr->buf, &line))
                        {
                            char msg[MAX_MESSAGE_SIZE + 100];
                            sprintf(msg, "client %d: %s", curr->id, line);
                            for (t_client *other = clients; other; other = other->next)
                                if (other->fd != curr->fd)
                                    send(other->fd, msg, strlen(msg), 0);
                            free(line);
                        }
                    }
                    char msg[100];
                    sprintf(msg, "server: client %d just left\n", curr->id);
                    for (t_client *other = clients; other; other = other->next)
                    {
                        if (other->fd != curr->fd)
                            send(other->fd, msg, strlen(msg), 0);
                    }
                    close(curr->fd);
                    if (curr->buf)
                        free(curr->buf);
                    if (prev)
                        prev->next = curr->next;
                    else
                        clients = curr->next;
                    t_client *tmp = curr;
                    curr = curr->next;
                    free(tmp);
                    continue;
                }
                buf[ret] = '\0';
                curr->buf = str_join(curr->buf, buf);
                if (!curr->buf)
                {
                    write(STDERR_FILENO, "Fatal error\n", 12);
                    close(sockfd);
                    exit(1);
                }
                char *line;
                while (curr->buf && extract_message(&curr->buf, &line))
                {
                    char msg[MAX_MESSAGE_SIZE + 100];
                    sprintf(msg, "client %d: %s", curr->id, line);
                    for (t_client *other = clients; other; other = other->next)
                        if (other->fd != curr->fd)
                            send(other->fd, msg, strlen(msg), 0);
                    free(line);
                }
            }
            prev = curr;
            curr = curr->next;
        }
    }
    return 0;
}