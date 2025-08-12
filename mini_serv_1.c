#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>

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

void handle_signal(int sig)
{
    (void)sig;
    free_clients(clients);
    if (sockfd >= 0)
        close(sockfd);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Wrong number of arguments\n", 26);
        exit(1);
    }

    signal(SIGINT, handle_signal);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        write(STDERR_FILENO, "Fatal error: socket\n", 20);
        exit(1);
    }

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(atoi(argv[1]));
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
    {
        write(STDERR_FILENO, "Fatal error: bind\n", 18);
        close(sockfd);
        exit(1);
    }
    if (listen(sockfd, 10) != 0)
    {
        write(STDERR_FILENO, "Fatal error: listen\n", 20);
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
        {
            if (errno == EINTR)
                continue;
            perror("select error");
            free_clients(clients);
            close(sockfd);
            exit(1);
        }

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
                snprintf(msg, sizeof(msg), "server: client %d just arrived\n", new_client->id);
                for (t_client *tmp = clients; tmp; tmp = tmp->next)
                {
                    if (tmp->fd != client_fd)
                        send(tmp->fd, msg, strlen(msg), 0);
                }
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
                    snprintf(msg, sizeof(msg), "server: client %d just left\n", curr->id);
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
                snprintf(msg, sizeof(msg), "client %d: %s", curr->id, buf);
                for (t_client *other = clients; other; other = other->next)
                {
                    if (other->fd != curr->fd)
                        send(other->fd, msg, strlen(msg), 0);
                }
            }
            prev = curr;
            curr = curr->next;
        }
    }
    return 0;
}