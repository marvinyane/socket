#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LENGTH_OF_LISTEN_QUEUE  20
#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE  512

int main(int argc, char** argv)
{
    char name[100];
    sprintf(name, "my-client-%s", argv[1]);

    struct sockaddr_un client_addr;
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sun_family = AF_UNIX;
    strncpy(client_addr.sun_path, name, sizeof(client_addr.sun_path)-1);

    int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if ( client_socket < 0 )
    {
        printf("create socket failed!\n");
        exit(1);
    }

    if (bind(client_socket, (struct sockaddr*)&client_addr, sizeof(client_addr)) < 0)
    {
        perror("Client bind port failed:");
        exit(1);
    }

    struct sockaddr_un server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sun_family = AF_UNIX;
    strncpy(server_addr.sun_path, "my-socket", sizeof(server_addr.sun_path)-1);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof server_addr) < 0)
    {
        printf("connect failed! \n");
        exit(1);
    }


    {
        write(client_socket, "hello", 5);

        char buf[1024];
        bzero(buf, sizeof buf);
        read(client_socket, buf, sizeof buf);
        printf("client recv: %s \n", buf);

    }

    close(client_socket);
    unlink(name);

    return 0;
}
