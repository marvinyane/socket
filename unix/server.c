#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <poll.h>

int main()
{
    int fd; 
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    struct sockaddr_un addr;
    bzero(&addr, sizeof(addr));

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, "my-socket", sizeof(addr.sun_path)-1);
    if (fd < 0)
    {
        printf("socket create error.\n");
        return 0;
    }

    bind(fd, (struct sockaddr*)&addr, sizeof(addr));

    // max 64 connections
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
            struct sockaddr_un ca;
            socklen_t ca_len;
            bzero(&ca, sizeof(ca));
            int new_fd = accept(fds[0].fd, (struct sockaddr*)&ca, &ca_len);
            printf("%s connected.\n", ca.sun_path);
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
