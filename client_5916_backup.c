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

        printf("\n%s", response);
        printf("Enter command: ");
        fflush(stdout);
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
