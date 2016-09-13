#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // ioctl
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <string.h>
#include <arpa/inet.h> // inet_ntoa

// man 7 netdevice

int print_if_addr(int fd, char* if_name)
{
    struct sockaddr_in *ip;
    struct ifreq ifr;

    strcpy(ifr.ifr_name, if_name);

    if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
    {
        perror("ioctl SIOCGIFADDR error");
        return -1;
    }

    ip = (struct sockaddr_in*)&ifr.ifr_addr;
    printf(" IP : %s\n", inet_ntoa(ip->sin_addr));
    return 0;
}

int print_interface_info(char* if_name)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error");
        return -1;
    }

    printf("%s:\n", if_name);
    print_if_addr(sockfd, if_name);
    close(sockfd);
    return 0;
}

int set_if_up(char* if_name)
{
    struct ifreq ifr;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error");
        return -1;
    }

    strcpy(ifr.ifr_name, if_name);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl SIOCGIFFLAGS error");
        return -1;
    }

    ifr.ifr_flags |= IFF_UP;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl SIOCSIFFLAGS error");
        return -1;
    }

    close(sockfd);

    return 0;
}

int set_if_down(char* if_name)
{
    struct ifreq ifr;
    int sockfd;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket error");
        return -1;
    }

    strcpy(ifr.ifr_name, if_name);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) < 0)
    {
        perror("ioctl SIOCGIFFLAGS error");
        return -1;
    }

    ifr.ifr_flags &= ~IFF_UP;

    if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) < 0)
    {
        perror("ioctl SIOCSIFFLAGS error");
        return -1;
    }

    close(sockfd);

    return 0;
}


int main(int argc, char** argv)
{
    switch(argc)
    {
        case 1:
            print_interface_info("eth0");
            break;
        case 2:
            print_interface_info(argv[1]);
            break;
        case 3:
            if (strcmp("up", argv[2]) == 0) 
            {
                set_if_up(argv[1]);
            }
            else if (strcmp("down", argv[2]) == 0)
            {
                set_if_down(argv[1]);
            }
            else
            {
                printf("unknown command %s.\n", argv[2]);
            }
            break;
    }

    return 0;
}
