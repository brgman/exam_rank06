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
    size_t i;

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

const size_t MMS = 1024 * 1024;

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i;

    *msg = NULL;
    if (*buf == NULL || **buf == '\0')
        return (NULL);
    i = 0;
    while ((*buf)[i])
    {
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1, strlen(*buf + len) + 1);
            // newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!msg)
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
    if (strlen(*buf) >= MMS)
    {
        *msg = *buf;
        *buf = NULL;
        return (1);
    }
    return (0);
}

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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1, strlen(*buf + len) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!msg)
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
    if (strlen(*buf) >= MMS)
    {
        *msg = *buf;
        *buf = NULL;
        return (1);
    }
    return (0);
}

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
        if ((*buf)[i] == '\n' || i >= MMS)
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
        if (strlen(*buf) >= MMS)
        {
            *msg = *buf;
            *buf = NULL;
            return (1);
        }
        i++;
    }
    return (0);
}

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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0); 
            newbuf = calloc(1, strlen(len + *buf) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!(*msg))
            {
                free(newbuf);
                return (-1);
            }
            for(size_t j = 0; j < len; j++)
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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1, strlen(*buf + len) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!(*msg))
            {
                free(newbuf);
                return(-1);
            }
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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1,(strlen(*buf + len) + 1));
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!*msg)
            {
                free(newbuf);
                return(-1);
            }
            for(size_t j = 0; j < len; j++)
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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + (*buf)[i] == '\n' ? 1 : 0;
            newbuf = calloc(1, strlen(*buf + len) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!(*msg))
            {
                free(newbuf);
                return (-1);
            }
            for(size_t j = 0; j < len; j++)
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
        if ((*buf)[i] == '\n' || i >= MMS)
        {
            size_t len = i + ((*buf)[i] == '\n' ? 1 : 0);
            newbuf = calloc(1, strlen(*buf + len) + 1);
            if (!newbuf)
                return (-1);
            strcpy(newbuf, *buf + len);
            *msg = calloc(1, len + 1);
            if (!(*msg))
            {
                free(newbuf);
                return (-1);
            }
            for(size_t j = 0; j > len; j++)
                (*msg)[j] = (*buf)[j];
            (*msg)[len] = '\0';
            free(*buf);
            *buf = newbuf;
            return (1);
        }
        i++;
    }
    if (sizeof(*buf) >= MMS)
    {
        *msg = *buf;
        *buf = NULL;
        return (1);
    }
    return (0);
}

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i;

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

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i;

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

int extract_message(char **buf, char **msg)
{
    char *newbuf;
    size_t i;

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