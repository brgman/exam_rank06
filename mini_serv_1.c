#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

typedef struct s_client
{
    int id;
    int fd;
    char *buf; // Добавляем буфер для хранения данных клиента
    struct s_client *next;
} t_client;

static t_client *clients = NULL;
static int sockfd = -1;

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

void free_clients(t_client *clients)
{
    t_client *curr = clients;
    while (curr)
    {
        t_client *tmp = curr;
        curr = curr->next;
        if (tmp->buf)
            free(tmp->buf); // Освобождаем буфер клиента
        close(tmp->fd);
        free(tmp);
    }
}

void cleanup_and_exit(int exit_code)
{
    free_clients(clients);
    if (sockfd != -1)
        close(sockfd);
    exit(exit_code);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        write(STDERR_FILENO, "Wrong number of arguments\n", 26);
        exit(1);
    }

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

    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds); // Следим за вводом с клавиатуры
    FD_SET(sockfd, &read_fds);

    while (1)
    {
        fd_set tmp_fds = read_fds;
        max_fd = sockfd;
        for (t_client *tmp = clients; tmp; tmp = tmp->next)
        {
            FD_SET(tmp->fd, &tmp_fds);
            if (tmp->fd > max_fd)
                max_fd = tmp->fd;
        }
        if (STDIN_FILENO > max_fd)
            max_fd = STDIN_FILENO;

        if (select(max_fd + 1, &tmp_fds, NULL, NULL, NULL) < 0)
        {
            if (errno == EINTR)
                continue;
            perror("select error");
            cleanup_and_exit(1);
        }

        // Проверяем ввод с клавиатуры
        if (FD_ISSET(STDIN_FILENO, &tmp_fds))
        {
            char input;
            if (read(STDIN_FILENO, &input, 1) > 0)
            {
                if (input == 'q' || input == '\n')
                {
                    cleanup_and_exit(0);
                }
            }
        }

        // Обработка нового подключения
        if (FD_ISSET(sockfd, &tmp_fds))
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
                new_client->buf = NULL; // Инициализируем буфер клиента
                new_client->next = clients;
                clients = new_client;
                FD_SET(client_fd, &read_fds);
                char msg[100];
                snprintf(msg, sizeof(msg), "server: client %d just arrived\n", new_client->id);
                for (t_client *tmp = clients; tmp; tmp = tmp->next)
                {
                    if (tmp->fd != client_fd)
                        send(tmp->fd, msg, strlen(msg), 0);
                }
            }
        }

        // Обработка клиентских сокетов
        t_client *curr = clients;
        t_client *prev = NULL;
        while (curr)
        {
            if (FD_ISSET(curr->fd, &tmp_fds))
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
                    FD_CLR(tmp->fd, &read_fds);
                    if (tmp->buf)
                        free(tmp->buf); // Освобождаем буфер клиента
                    close(tmp->fd);
                    free(tmp);
                    max_fd = sockfd;
                    for (t_client *tmp = clients; tmp; tmp = tmp->next)
                        if (tmp->fd > max_fd)
                            max_fd = tmp->fd;
                    if (STDIN_FILENO > max_fd)
                        max_fd = STDIN_FILENO;
                    continue;
                }
                buf[ret] = '\0';
                // Добавляем полученные данные в буфер клиента
                curr->buf = str_join(curr->buf, buf);
                if (!curr->buf)
                {
                    write(STDERR_FILENO, "Fatal error: str_join\n", 22);
                    cleanup_and_exit(1);
                }
                // Извлекаем и отправляем полные строки
                char *line;
                while (extract_message(&curr->buf, &line))
                {
                    char msg[1100];
                    snprintf(msg, sizeof(msg), "client %d: %s", curr->id, line);
                    for (t_client *other = clients; other; other = other->next)
                    {
                        if (other->fd != curr->fd)
                            send(other->fd, msg, strlen(msg), 0);
                    }
                    free(line); // Освобождаем извлечённую строку
                }
            }
            prev = curr;
            curr = curr->next;
        }
    }
    cleanup_and_exit(0);
}