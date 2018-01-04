#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH_OF_LISTEN_QUEUE  20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE  512

int main(int argc, char** argv)
{
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(atoi(argv[1]));

    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if ( server_socket < 0 )
    {
        printf("create socket failed!\n");
        exit(1);
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Server bind port : %d failed! \n", server_addr.sin_port);
        exit(1);
    }

    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Server listen failed! \n");
        exit(1);
    }

    while (1)
    {
        struct sockaddr_in client_addr;
        socklen_t length =  sizeof(client_addr);

        printf("wait for connecting....\n");
        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);

        if (new_server_socket < 0)
        {
            printf("socket accept failed! \n");
            break;
        }
        
        printf("clint connected!\n");

        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);

        while (1)
        {
            char buffer[1024];
            bzero(buffer, sizeof(buffer));
            scanf("send:%s", buffer);
            send(new_server_socket, buffer, strlen(buffer), 0);
#if 1
            length = recv(new_server_socket, buffer, BUFFER_SIZE, 0);

            if (length < 0)
            {
                printf("server received data failed! \n");
                break;
            }

            if (length == 0)
            {
                printf("socket disconnected!\n");
                break;
            }

            printf("received data: %s \n", buffer);
#endif
        }

    }

    return 0;
}
