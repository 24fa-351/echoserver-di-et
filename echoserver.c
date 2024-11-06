#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define DEFAULT_PORT 46645
#define LISTEN_BACKLOG 5

void* handle_connection(void* client_socket_fd_ptr)
{
    int client_socket_fd = (intptr_t)client_socket_fd_ptr;  
    char buffer [1024];
    int bytes_read;

    while((bytes_read = read(client_socket_fd, buffer, sizeof(buffer)- 1)) > 0)
    {
        buffer[bytes_read] = '\0';
        printf("Received: %s\n", buffer);
        write(client_socket_fd, buffer, bytes_read);
    }
    close(client_socket_fd);
    return NULL;
}

int main(int argc, char *argv[])
{
    int port = DEFAULT_PORT;
    if (argc == 3 && strcmp(argv[1], "-p") == 0)
    {
        port = atoi(argv[2]);
        if(port <= 0 || port > 65535){
            fprintf(stderr, "invalid port number. Input a number between the range 1 and 65535");
            return 1;
        }
    }

    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd == -1)
    {
        perror("socket creation failed");
        return 1;

    }

    struct sockaddr_in socket_address;
    memset (&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(DEFAULT_PORT);

    if(bind(socket_fd, (struct sockaddr*) &socket_address, sizeof(socket_address)) == -1)
    {
        perror("bind fail\n");
        close(socket_fd);
        return 1;
    }

    if(listen(socket_fd, LISTEN_BACKLOG) == -1)
    {
        perror("listen fail\n");
        close(socket_fd);
        return 1;
    }
    printf("Server listening on port %d\n", port);

    while (1)
    {
        struct sockaddr_in client_address;
        socklen_t client_address_len = sizeof(client_address);
        int* client_fd = malloc(sizeof(int));
        *client_fd = accept(socket_fd, (struct sockaddr*)&client_address, &client_address_len);

        if (*client_fd == -1)
        {
            perror("Accept fail\n");
            free(client_fd);
            continue;
        }
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_connection, client_fd) != 0)
        {
            perror("threads creation failed\n");
            close(*client_fd);
            free(client_fd);
        } else {
            pthread_detach(thread_id);
        }        
    }
    close(socket_fd);
    return 0;
}

/*
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 46645
#define LISTEN_BACKLOG 5

void handle_connection(int a_client)
{
    char buffer [1024];
    int bytes_read = read(a_client, buffer, sizeof(buffer));
    printf("Received: %s\n", buffer);
    write(a_client, buffer, bytes_read);
}

int main(int argc, char *argv[])
{
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in socket_address;
    memset (&socket_address, '\0', sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
    socket_address.sin_port = htons(PORT);

    int returnval;

    returnval = bind(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address));

    returnval = listen(socket_fd, LISTEN_BACKLOG);

    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_fd = accept(socket_fd, (struct sockaddr*)&client_address, &client_address_len);
    handle_connection(client_fd);    
    return 0;
}
*/