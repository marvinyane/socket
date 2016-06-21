#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include <strings.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <poll.h>

int create_socket(short port)
{
    int fd; 
    if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        return -1;

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    
    int on = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
    {
        close(fd);
        return -1;
    }
    
    bind(fd, (struct sockaddr*)&servaddr, sizeof(servaddr));

    return fd;
}

int main(void)
{
    int fd = create_socket(65001);
    if (fd < 0)
    {
        printf("socket create error.\n");
        return 0;
    }

    listen(fd, 64);

    struct pollfd *fds = malloc(65 * sizeof(struct pollfd));
    bzero(fds, sizeof(struct pollfd) * 65);

    int poll_cnt = 1;
    fds[0].fd = fd;
    fds[0].events = POLLIN;

    while (1)
    {
        int ret = poll(fds, poll_cnt, -1);

        if (ret < 0)
        {
            perror("poll error:");
            continue;
        }

        if (fds[0].revents & POLLIN)
        {
            // incoming connection
            int new_fd = accept(fds[0].fd, NULL, 0);
            fds[poll_cnt].fd = new_fd;
            fds[poll_cnt].events = POLLIN;
            poll_cnt ++;
        }

        int i = 1;
        for (; i < poll_cnt; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                // read....
            }
        }
    }

    
    return 0;
}
