
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

const size_t MMS = 1024 * 1024;

typedef struct s_client
{
    int fd;
    int id;
    char *buf;
    struct s_client *next;
} t_client;

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i; // ! size_t

    *msg = NULL;
    if (*buf == NULL || **buf == '\0')
        return (0);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + (*buf)[i] == '\n' ? 1 : 0;
            newbuf = calloc(1, (strlen(*buf + len) + 1));
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!(*msg))      // !(*msg)
            {                 // !
                free(newbuf); // ! free
                return (-1);  // ! -1
            }
            for (size_t j = 0; j < len; j++) // size_t
                (*msg)[j] = (*buf)[j];
            (*msg)[len] = '\0';
            free(*buf);
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    if (strlen(*buf) >= MMS)
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

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        write(2, "Wrong number of arguments\n", 26);
        exit(1);
    }
    int sockfd = socket(2, 1, 0);
    if (sockfd == -1)
    {
        write(2, "Fatal error\n", 12);
        exit(1);
    }
    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_addr.s_addr = htonl(0x7f000001);
    serv_addr.sin_family = 2;
    serv_addr.sin_port = htons(atoi(argv[1]));

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0) // != 0
    {
        write(2, "Fatal erroe\n", 12);
        close(sockfd); // close sockfd
        exit(1);
    }

    if (listen(sockfd, 10) != 0) // != 0
    {
        write(2, "Fatal error\n", 12);
        close(sockfd); // close sockfd
        exit(1);
    }

    t_client *clients = NULL;
    int max_fd = sockfd;
    int next_id = 0;
    fd_set read_fds;

    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(sockfd, &read_fds);
        for (t_client *tmp = clients; tmp; tmp = tmp->next) //
        {
            FD_SET(tmp->fd, &read_fds); //
            if (tmp->fd > max_fd)       //
                max_fd = tmp->fd;       //
        }

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) //
            continue;                                            // for if if while

        if (FD_ISSET(sockfd, &read_fds))
        {
            struct sockaddr_in cli_addr;
            socklen_t len = sizeof(cli_addr);
            int client_fd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);
            if (client_fd >= 0)
            {
                t_client *new = calloc(1, sizeof(t_client));
                if (!new)
                {
                    write(2, "Fatal error\n", 12);
                    close(client_fd); // close clientfd
                    exit(1);
                }
                new->buf = NULL;
                new->fd = client_fd;
                new->id = next_id++;
                new->next = clients;
                clients = new;
                if (client_fd > max_fd)
                    max_fd = client_fd;
                char msg[100];
                sprintf(msg, "server: client %d just arrived\n", new->id);
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
                char buf[65536];
                size_t ret = recv(curr->fd, buf, sizeof(buf) - 1, 0); // size_t
                if (ret <= 0)
                {
                    if (curr->buf) // if curr->bur;
                    {
                        char *line; // *
                        while (extract_message(&curr->buf, &line))
                        {
                            char msg[MMS + 100];
                            sprintf(msg, "client %d: %s", curr->id, line);
                            for (t_client *tmp = clients; tmp; tmp = tmp->next)
                                if (tmp->fd != curr->fd)
                                    send(tmp->fd, msg, strlen(msg), 0);
                            free(line);
                        }
                    }
                    char msg[100];
                    sprintf(msg, "server: client %d just left\n", curr->id);
                    for (t_client *tmp = clients; tmp; tmp = tmp->next)
                        if (tmp->fd != curr->fd)
                            send(tmp->fd, msg, strlen(msg), 0);
                    close(curr->fd);
                    if (prev)
                        prev->next = curr->next;
                    else
                        clients = curr->next;
                    if (curr->buf)
                        free(curr->buf);
                    t_client *tmp = curr;
                    curr = curr->next;
                    free(tmp);
                    continue;
                }
                buf[ret] = '\0';
                curr->buf = str_join(curr->buf, buf); // !
                if (!curr->buf)                       // !
                {
                    write(2, "Fatal error\n", 12);
                    close(sockfd);
                    exit(1);
                }
                char *line;
                while (curr->buf && extract_message(&curr->buf, &line))
                {
                    char msg[MMS + 100];
                    sprintf(msg, "client %d: %s", curr->id, line);
                    for (t_client *tmp = clients; tmp; tmp = tmp->next)
                        if (tmp->fd != curr->fd)
                            send(tmp->fd, msg, strlen(msg), 0);
                    free(line);
                }
            }
            prev = curr;       // prev = curr;
            curr = curr->next; // curr = curr->next;
        }
    }
    return (0);
}