#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HELLO_WORLD_SERVER_PORT 6668
#define LENGTH_OF_LISTEN_QUEUE  20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE  512

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        printf("usage : ./%s ip port! \n", argv[0]);
        exit(1);
    }

    /*struct sockaddr_in client_addr;*/
    /*bzero(&client_addr, sizeof(client_addr));*/
    /*client_addr.sin_family = AF_INET;*/
    /*client_addr.sin_addr.s_addr = htons(INADDR_ANY);*/
    /*client_addr.sin_port = htons(0);*/

    while (1)
    {
        int client_socket = socket(AF_INET, SOCK_STREAM, 0);
        if ( client_socket < 0 )
        {
            printf("create socket failed!\n");
            exit(1);
        }

        /*if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)))*/
        /*{*/
            /*printf("Client bind port failed! \n");*/
            /*exit(1);*/
        /*}*/

        struct sockaddr_in server_addr;
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;

        if (inet_aton(argv[1], &server_addr.sin_addr) == 0)
        {
            printf("Server Ip address Error!\n");
            return 0;
        }

        server_addr.sin_port = htons(atoi(argv[2]));
        socklen_t server_addr_length = sizeof(server_addr);

        if (connect(client_socket, (struct sockaddr*)&server_addr, server_addr_length) < 0)
        {
            printf("connect failed! \n");
            return 0;
        }

        printf("connected!\n");

        while (1)
        {
            char buffer[BUFFER_SIZE];
            bzero(buffer, BUFFER_SIZE);

            printf("start recv....\n");
            int length = recv(client_socket, buffer, BUFFER_SIZE, 0);

            if (length == 0)
            {
                break;
            }
            else
            {
                printf("received data: %s \n", buffer);
            }

            send(client_socket, "data received!", 15, 0);
        }

        close(client_socket);
    }

    return 0;
}
