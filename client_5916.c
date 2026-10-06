#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define PORT 11916

int client_fd;

void *receive_messages(void *arg)
{
    char response[1024];
    int bytes_received;

    while (1)
    {
        memset(response, 0, sizeof(response));

        bytes_received = recv(client_fd,
                              response,
                              sizeof(response) - 1,
                              0);

        if (bytes_received <= 0)
        {
            printf("\nDisconnected from server.\n");
            break;
        }

        response[bytes_received] = '\0';

        /* FILE transfer */
        if (strncmp(response, "SENDFILE ", 9) == 0)
        {
            char filename[100];
            long filesize;

            if (sscanf(response, "SENDFILE %*s  %99s %ld",
                       filename, &filesize) == 2)
            {
                char *newline = strchr(response, '\n');

                if (newline != NULL)
                {
                    long header_length =
                        (newline - response) + 1;

                    long already_received =
                        bytes_received - header_length;

                    char save_name[150];

                    snprintf(save_name,
                             sizeof(save_name),
                             "received_%s",
                             filename);

                    FILE *fp = fopen(save_name, "wb");

                    if (fp == NULL)
                    {
                        printf("\nCannot create received file.\n");
                        continue;
                    }

                    /* Save any file bytes already received
                       together with the FILE header */
                    if (already_received > 0)
                    {
                        long bytes_to_write = already_received;

                        if (bytes_to_write > filesize)
                            bytes_to_write = filesize;

                        fwrite(response + header_length,
                               1,
                               bytes_to_write,
                               fp);

                        long total_received = bytes_to_write;

                        while (total_received < filesize)
                        {
                            char file_buffer[1024];

                            long remaining =
                                filesize - total_received;

                            int to_receive =
                                remaining < sizeof(file_buffer)
                                ? remaining
                                : sizeof(file_buffer);

                            int n = recv(client_fd,
                                         file_buffer,
                                         to_receive,
                                         0);

                            if (n <= 0)
                                break;

                            fwrite(file_buffer,
                                   1,
                                   n,
                                   fp);

                            total_received += n;
                        }

                        fclose(fp);

                        if (total_received == filesize)
                        {
                            printf("\nFile received: %s\n",
                                   save_name);
                        }
                        else
                        {
                            printf("\nFile transfer incomplete.\n");
                        }
                    }
                    else
                    {
                        long total_received = 0;

                        while (total_received < filesize)
                        {
                            char file_buffer[1024];

                            long remaining =
                                filesize - total_received;

                            int to_receive =
                                remaining < sizeof(file_buffer)
                                ? remaining
                                : sizeof(file_buffer);

                            int n = recv(client_fd,
                                         file_buffer,
                                         to_receive,
                                         0);

                            if (n <= 0)
                                break;

                            fwrite(file_buffer,
                                   1,
                                   n,
                                   fp);

                            total_received += n;
                        }

                        fclose(fp);

                        if (total_received == filesize)
                        {
                            printf("\nFile received: %s\n",
                                   save_name);
                        }
                        else
                        {
                            printf("\nFile transfer incomplete.\n");
                        }
                    }
                }
            }
        }
        else
        {
            printf("\n%s", response);
            printf("Enter command: ");
            fflush(stdout);
        }
    }

    return NULL;
}


int main()
{
    struct sockaddr_in server_addr;

    char username[100];
    char message[1024];

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (client_fd < 0)
    {
        perror("socket");
        return 1;
    }

    printf("Client socket created.\n");

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_fd,
                (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("connect");
        return 1;
    }

    printf("connected to server.\n");

    /* REGISTER */

    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);

    username[strcspn(username, "\n")] = '\0';

    snprintf(message,
             sizeof(message),
             "REGISTER %s\n",
             username);

    send(client_fd,
         message,
         strlen(message),
         0);

    memset(message, 0, sizeof(message));

    int bytes_received = recv(client_fd,
                              message,
                              sizeof(message) - 1,
                              0);

    if (bytes_received <= 0)
    {
        printf("Server disconnected.\n");
        close(client_fd);
        return 1;
    }

    message[bytes_received] = '\0';

    printf("server response: %s", message);

    /* Receiver thread */

    pthread_t receive_thread;

    pthread_create(&receive_thread,
                   NULL,
                   receive_messages,
                   NULL);

    /* Command loop */

    while (1)
    {
        printf("Enter command: ");
        fflush(stdout);

        fgets(message, sizeof(message), stdin);

        message[strcspn(message, "\n")] = '\0';

        if (strcmp(message, "QUIT") == 0)
        {
            strcat(message, "\n");

            send(client_fd,
                 message,
                 strlen(message),
                 0);

            break;
        }

        if (strncmp(message, "SENDFILE ", 9) == 0)
{
    char target[50];
    char filename[100];
    long filesize;

    if (sscanf(message, "SENDFILE %49s %99s %ld",
               target, filename, &filesize) == 3)
    {
        FILE *fp = fopen(filename, "rb");

        if (fp == NULL)
        {
            printf("File not found.\n");
            continue;
        }

        strcat(message, "\n");

        send(client_fd, message, strlen(message), 0);

        char file_buffer[1024];
        size_t bytes_read;

        while ((bytes_read = fread(file_buffer, 1,
                                   sizeof(file_buffer), fp)) > 0)
        {
            send(client_fd, file_buffer, bytes_read, 0);
        }

        fclose(fp);
    }
    else
    {
        printf("Invalid SENDFILE format.\n");
    }
}
else
{
    strcat(message, "\n");

    send(client_fd,
         message,
         strlen(message),
         0);
}
    }

    close(client_fd);

    return 0;
}
