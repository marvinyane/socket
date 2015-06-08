#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HELLO_WORLD_SERVER_PORT 6666
#define LENGTH_OF_LISTEN_QUEUE  20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE  512

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("usage : ./%s port! \n", argv[0]);
        exit(1);
    }

    struct sockaddr_in client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);

    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if ( client_socket < 0 )
    {
        printf("create socket failed!\n");
        exit(1);
    }

    if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))
    {
        printf("Client bind port failed! \n");
        exit(1);
    }

    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;

    if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
    {
        printf("Server Ip address Error!\n");
        exit(1);
    }

    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);
    socklen_t server_addr_length = sizeof(server_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        printf("connect failed! \n");
        exit(1);
    }

    while (1)
    {
        send(client_socket, "hello", 5, 0);

        sleep(10);
    }

    return 0;
}
