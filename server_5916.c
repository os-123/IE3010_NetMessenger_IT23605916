
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/stat.h>
#include <time.h>

#define PORT 11916
#define MAX_CLIENTS 10

int clients[MAX_CLIENTS];
char usernames[MAX_CLIENTS][50];

int client_count = 0;

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

#define MAX_ROOMS 10

char rooms[MAX_ROOMS][50];
int room_count = 0;

int client_rooms[MAX_CLIENTS];


void log_event(const char *event)
{
    FILE *log = fopen("netmsg_IT23605916.log", "a");

    if (log == NULL)
    {
        return;
    }

    time_t now = time(NULL);
    struct tm *time_info = localtime(&now);

    char timestamp[50];

    strftime(timestamp,
             sizeof(timestamp),
             "%Y-%m-%d %H:%M:%S",
             time_info);

    fprintf(log, "[%s] %s\n", timestamp, event);

    fclose(log);
}
void *handle_client(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);

    char buffer[1024];
    char username[50];

    /* REGISTER */
    memset(buffer, 0, sizeof(buffer));

    int bytes_received = recv(client_fd, buffer,
                              sizeof(buffer) - 1, 0);

    if (bytes_received <= 0)
    {
        close(client_fd);
        return NULL;
    }

    buffer[bytes_received] = '\0';

    if (strncmp(buffer, "REGISTER ", 9) == 0)
    {
        sscanf(buffer + 9, "%49s", username);

        pthread_mutex_lock(&clients_mutex);

for (int i = 0; i < MAX_CLIENTS; i++)
{
    if (clients[i] != 0 &&
        strcmp(usernames[i], username) == 0)
    {
        pthread_mutex_unlock(&clients_mutex);

        char response[] =
            "ERR 001 USERNAME_TAKEN NID:6059\n";

        send(client_fd, response,
             strlen(response), 0);

        close(client_fd);
        return NULL;
    }
}
        int index = -1;

        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] == 0)
            {
                clients[i] = client_fd;
                strcpy(usernames[i], username);
                index = i;
                break;
            }
        }

        pthread_mutex_unlock(&clients_mutex);

        if (index == -1)
        {
            char response[] =
                "ERR 003 SERVER_FULL NID:6059\n";

            send(client_fd, response, strlen(response), 0);
            close(client_fd);
            return NULL;
        }

        char response[200];

        snprintf(response, sizeof(response),
                 "OK REGISTERED %s NID:6059\n",
                 username);

        send(client_fd, response, strlen(response), 0);

        printf("Client registered: %s\n", username);

char log_message[200];

snprintf(log_message, sizeof(log_message),
         "Client registered: %s", username);

log_event(log_message);

pthread_mutex_lock(&clients_mutex);

char presence[200];
snprintf(presence, sizeof(presence),
         "MSG PRESENCE %s JOINED\n", username);

for (int i = 0; i < MAX_CLIENTS; i++)
{
    if (clients[i] != 0 && clients[i] != client_fd)
    {
        send(clients[i], presence, strlen(presence), 0);
    }
}

pthread_mutex_unlock(&clients_mutex);
}

    /* COMMAND LOOP */
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        bytes_received = recv(client_fd, buffer,
                              sizeof(buffer) - 1, 0);

        if (bytes_received <= 0)
        {
            printf("Client disconnected: %s\n", username);

char log_message[200];

snprintf(log_message, sizeof(log_message),
         "Client disconnected: %s", username);

log_event(log_message);
            break;
        }

        buffer[bytes_received] = '\0';

        /* LIST */
        if (strncmp(buffer, "LIST", 4) == 0)
        {
            char response[1024] = "OK USERS ";

            pthread_mutex_lock(&clients_mutex);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] != 0)
                {
                    strcat(response, usernames[i]);
            strcat(response, ",");
                }
            }

            pthread_mutex_unlock(&clients_mutex);

            strcat(response, "NID:6059\n");

            send(client_fd, response, strlen(response), 0);
        }

        /* JOIN */
        else if (strncmp(buffer, "JOIN ", 5) == 0)
        {
            char room_name[50];
            int room_found = 0;
            int room_index = -1;

            sscanf(buffer + 5, "%49s", room_name);

            pthread_mutex_lock(&clients_mutex);

            /* Check whether room already exists */
            for (int i = 0; i < room_count; i++)
            {
                if (strcmp(rooms[i], room_name) == 0)
                {
                    room_found = 1;
                    room_index = i;
                    break;
                }
            }

            /* Create new room if it does not exist */
            if (!room_found && room_count < MAX_ROOMS)
            {
                strcpy(rooms[room_count], room_name);
                room_index = room_count;
                room_count++;
                room_found = 1;
            }

            /* Find this client's index */
            int client_index = -1;

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == client_fd)
                {
                    client_index = i;
                    break;
                }
            }

            if (room_found && client_index != -1)
            {
                client_rooms[client_index] = room_index;
            }

            pthread_mutex_unlock(&clients_mutex);

            if (room_found && client_index != -1)
            {
                char response[200];

snprintf(response, sizeof(response),
         "OK JOINED %s NID:6059\n",
         room_name);

                send(client_fd,
                     response,
                     strlen(response),
                     0);
            }
            else
            {
                char response[] =
                    "ERR 003 ROOM_FULL NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);
            }
        }

        /* LEAVE */
        else if (strncmp(buffer, "LEAVE ", 6) == 0)
        {
            char room_name[50];
            int room_found = 0;
            int room_index = -1;
            int client_index = -1;

            sscanf(buffer + 6, "%49s", room_name);

            pthread_mutex_lock(&clients_mutex);

            for (int i = 0; i < room_count; i++)
            {
                if (strcmp(rooms[i], room_name) == 0)
                {
                    room_found = 1;
                    room_index = i;
                    break;
                }
            }

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == client_fd)
                {
                    client_index = i;
                    break;
                }
            }

            if (room_found &&
                client_index != -1 &&
                client_rooms[client_index] == room_index)
            {
                client_rooms[client_index] = -1;
            }

            pthread_mutex_unlock(&clients_mutex);

            if (room_found && client_index != -1)
            {
                char response[200];

snprintf(response, sizeof(response),
         "OK LEFT %s NID:6059\n",
         room_name);

                send(client_fd,
                     response,
                     strlen(response),
                     0);
            }
            else
            {
                char response[] =
                    "ERR 001 ROOM_NOT_FOUND NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);
            }
        }

        /* ROOMS */
        else if (strncmp(buffer, "ROOMS", 5) == 0)
        {
            char response[1024] = "OK ROOMS ";

            pthread_mutex_lock(&clients_mutex);

            for (int i = 0; i < room_count; i++)
            {
                if (i > 0)
    strcat(response, ",");

strcat(response, rooms[i]);
            }

            pthread_mutex_unlock(&clients_mutex);

            strcat(response, "NID:6059\n");

            send(client_fd,
                 response,
                 strlen(response),
                 0);
        }

        /* RMSG */
        else if (strncmp(buffer, "RMSG ", 5) == 0)
        {
            char room_name[50];
            char room_message[900];

            int room_index = -1;
            int client_index = -1;

            sscanf(buffer + 5, "%49s %[^\n]",
                   room_name, room_message);

            pthread_mutex_lock(&clients_mutex);

            /* Find room */
            for (int i = 0; i < room_count; i++)
            {
                if (strcmp(rooms[i], room_name) == 0)
                {
                    room_index = i;
                    break;
                }
            }

            /* Find sender */
            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] == client_fd)
                {
                    client_index = i;
                    break;
                }
            }

            if (room_index != -1 &&
                client_index != -1 &&
                client_rooms[client_index] == room_index)
            {
                char room_message_send[1024];

                snprintf(room_message_send,
                         sizeof(room_message_send),
                         "MSG ROOM %s %s %s\n",
                         room_name,
                         username,
                         room_message);

                /* Send to other members */
                for (int i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i] != 0 &&
                        clients[i] != client_fd &&
                        client_rooms[i] == room_index)
                    {
                        send(clients[i],
                             room_message_send,
                             strlen(room_message_send),
                             0);
                    }
                }

                pthread_mutex_unlock(&clients_mutex);

                char response[] =
                    "OK SENT NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);

char log_message[300];

snprintf(log_message, sizeof(log_message),
         "RMSG from %s in room %s: %s",
         username, room_name, room_message);

log_event(log_message);
            }
            else
            {
                pthread_mutex_unlock(&clients_mutex);

                char response[] =
                    "ERR 001 ROOM_NOT_FOUND NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);
            }
        }
        /* PMSG */
        else if (strncmp(buffer, "PMSG ", 5) == 0)
        {
            char target[50];
            char message[900];
            int found = 0;

            sscanf(buffer + 5, "%49s %[^\n]", target, message);

            char private_message[1024];

            snprintf(private_message,
                     sizeof(private_message),
                     "MSG PRIV %s %s\n",
                     username,
                     message);

            pthread_mutex_lock(&clients_mutex);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] != 0 &&
                    strcmp(usernames[i], target) == 0)
                {
                    send(clients[i],
                         private_message,
                         strlen(private_message),
                         0);

                    found = 1;
                    break;
                }
            }

            pthread_mutex_unlock(&clients_mutex);

            if (found)
            {
                char response[] =
                    "OK SENT NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);

char log_message[300];

snprintf(log_message, sizeof(log_message),
         "PMSG from %s to %s: %s",
         username, target, message);

log_event(log_message);
            }
            else
            {
                char response[] =
                    "ERR 002 USER_NOT_FOUND NID:6059\n";

                send(client_fd,
                     response,
                     strlen(response),
                     0);

            }
        }
        /* BCAST */
        else if (strncmp(buffer, "BCAST ", 6) == 0)
        {
            char message[900];

            strcpy(message, buffer + 6);

            char response[] =
                "OK SENT NID:6059\n";

            send(client_fd, response, strlen(response), 0);

            char broadcast[1024];

            snprintf(broadcast, sizeof(broadcast),
                     "MSG BCAST %s %s",
                     username,
                     message);

            pthread_mutex_lock(&clients_mutex);

            for (int i = 0; i < MAX_CLIENTS; i++)
            {
                if (clients[i] != 0 &&
                    clients[i] != client_fd)
                {
                    send(clients[i],
                         broadcast,
                         strlen(broadcast),
                         0);
                }
            }

            pthread_mutex_unlock(&clients_mutex);

char log_message[300];

snprintf(log_message, sizeof(log_message),
         "BCAST from %s: %s", username, message);

log_event(log_message);
        }
/* SENDFILE */
else if (strncmp(buffer, "SENDFILE ", 9) == 0)
{
    char target[50];
    char filename[100];
    long filesize;

    if (sscanf(buffer, "SENDFILE %49s %99s %ld",
               target, filename, &filesize) == 3)
{
int target_found = 0;
int target_fd = -1;

pthread_mutex_lock(&clients_mutex);

for (int i = 0; i < MAX_CLIENTS; i++)
{
    if (clients[i] != 0 &&
        strcmp(usernames[i], target) == 0)
    {
        target_found = 1;
        target_fd = clients[i];
        break;
    }
}

pthread_mutex_unlock(&clients_mutex);

if (!target_found)
{
    char response[] =
        "ERR 002 USER_NOT_FOUND NID:6059\n";

    send(client_fd, response,
         strlen(response), 0);

    continue;
}
    
        if (filesize > 10 * 1024 * 1024)
        {
            char response[] = "ERR 004 FILE_TOO_LARGE NID:6059\n";
            send(client_fd, response, strlen(response), 0);
        }
        else
        {
            char filepath[300];

            mkdir("storage", 0777);
            mkdir("storage/IT23605916", 0777);

            char sender_path[200];
            snprintf(sender_path, sizeof(sender_path),
                     "storage/IT23605916/%s", username);
            mkdir(sender_path, 0777);

            snprintf(filepath, sizeof(filepath),
                     "%s/%s", sender_path, filename);

            FILE *fp = fopen(filepath, "wb");

            if (fp == NULL)
            {
                char response[] = "ERR 004 FILE_TOO_LARGE NID:6059\n";
                send(client_fd, response, strlen(response), 0);
            }
            else
            {
                char file_buffer[1024];
                long total_received = 0;

                while (total_received < filesize)
                {
                    long remaining = filesize - total_received;
                    int to_receive = remaining < sizeof(file_buffer)
                                     ? remaining
                                     : sizeof(file_buffer);

                    int n = recv(client_fd, file_buffer,
                                 to_receive, 0);

                    if (n <= 0)
                        break;

                    fwrite(file_buffer, 1, n, fp);
                    total_received += n;
                }

                fclose(fp);

char send_header[200];

snprintf(send_header, sizeof(send_header),
         "SENDFILE %s %s %ld\n",
         target, filename, filesize);

send(target_fd, send_header, strlen(send_header), 0);

FILE *send_fp = fopen(filepath, "rb");

if (send_fp != NULL)
{
    char send_buffer[1024];
    size_t send_n;

    while ((send_n = fread(send_buffer, 1,
                           sizeof(send_buffer), send_fp)) > 0)
    {
        send(target_fd, send_buffer, send_n, 0);
    }

    fclose(send_fp);
}

                if (total_received == filesize)
                {
                    char response[200];

                    snprintf(response, sizeof(response),
                             "OK FILE_RECEIVED %s NID:6059\n",
                             filename);

                    send(client_fd, response,
                         strlen(response), 0);

char log_message[300];

snprintf(log_message, sizeof(log_message),
         "SENDFILE from %s: %s (%ld bytes)",
         username, filename, filesize);

log_event(log_message);
                }
            }
        }
    }
}

        /* QUIT */
        else if (strncmp(buffer, "QUIT", 4) == 0)
        {
            char response[] =
                "OK BYE NID:6059\n";

            send(client_fd, response, strlen(response), 0);

            break;
        }

else if (strncmp(buffer, "HELP", 4) == 0)
{
    char response[] =
        "OK HELP NID:6059\n"
        "REGISTER <username>\n"
        "LIST\n"
        "BCAST <message>\n"
        "PMSG <username> <message>\n"
        "JOIN <room>\n"
        "LEAVE <room>\n"
        "ROOMS\n"
        "RMSG <room> <message>\n"
        "SENDFILE <target> <filename> <filesize>\n"
        "QUIT\n";

    send(client_fd, response, strlen(response), 0);
}

        /* UNKNOWN COMMAND */
        else
        {
            char response[] =
                "ERR 004 UNKNOWN_COMMAND NID:6059\n";

            send(client_fd, response, strlen(response), 0);
        }
    }

    /* Remove client */
    pthread_mutex_lock(&clients_mutex);

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == client_fd)
        {
            clients[i] = 0;
            usernames[i][0] = '\0';
            break;
        }
    }

    pthread_mutex_unlock(&clients_mutex);

    close(client_fd);

    return NULL;
}

int main()
{
    int server_fd;

    struct sockaddr_in server_addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0)
    {
        perror("socket");
        return 1;
    }

    printf("Server socket created.\n");

int opt = 1;

setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
           &opt, sizeof(opt));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd,
             (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0)
    {
        perror("bind");
        return 1;
    }

    printf("Server bound to port %d.\n", PORT);

    if (listen(server_fd, 5) < 0)
    {
        perror("listen");
        return 1;
    }

    printf("Server is listening...\n");

    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i] = 0;
    }

    while (1)
    {
        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0)
        {
            perror("accept");
            continue;
        }

        printf("Client connected.\n");

        int *new_client = malloc(sizeof(int));

        *new_client = client_fd;

        pthread_t thread;

        pthread_create(&thread,
                       NULL,
                       handle_client,
                       new_client);

        pthread_detach(thread);
    }

    close(server_fd);

    return 0;
}
