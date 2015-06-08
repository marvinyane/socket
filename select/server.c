#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<errno.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<signal.h>
#include<sys/wait.h>

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)


int main(void)
{
    
    signal(SIGPIPE, SIG_IGN);
    int listenfd; 
    if ((listenfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        ERR_EXIT("socket error");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(5188);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    
    int on = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) < 0)
        ERR_EXIT("setsockopt error");

    if (bind(listenfd, (struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    if (listen(listenfd, SOMAXCONN) < 0)
        ERR_EXIT("listen error");
    
    struct sockaddr_in peeraddr;
    socklen_t peerlen = sizeof(peeraddr); 
    
    int conn;
    int i;
    int client[FD_SETSIZE];
    int maxi = 0; 
    for (i = 0; i < FD_SETSIZE; i++)
        client[i] = -1;

    int nready;
    int maxfd = listenfd;

    fd_set rset;
    fd_set allset;
    FD_ZERO(&rset);
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    while (1) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);
        if (nready == -1) {
            if (errno == EINTR)
                continue;
            ERR_EXIT("select error");
        }

        if (nready == 0)
            continue;

        if (FD_ISSET(listenfd, &rset)) {
        
            conn = accept(listenfd, (struct sockaddr*)&peeraddr, &peerlen);
            if (conn == -1)
                ERR_EXIT("accept error");
            
            for (i = 0; i < FD_SETSIZE; i++) {
                if (client[i] < 0) {
                    client[i] = conn;
                    if (i > maxi)
                        maxi = i;
                    break;
                } 
            }
            
            if (i == FD_SETSIZE) {
                fprintf(stderr, "too many clients\n");
                exit(EXIT_FAILURE);
            }

            printf("recv connect ip=%s port=%d\n", inet_ntoa(peeraddr.sin_addr),
                ntohs(peeraddr.sin_port));

            FD_SET(conn, &allset);
            if (conn > maxfd)
                maxfd = conn;

            if (--nready <= 0)
                continue;
        }

        for (i = 0; i <= maxi; i++) {
            conn = client[i];
            if (conn == -1)
                continue;

            if (FD_ISSET(conn, &rset)) {
                
                char recvbuf[1024] = {0};
                int ret = readline(conn, recvbuf, 1024);
                if (ret == -1)
                    ERR_EXIT("readline error");
                else if (ret  == 0) {
                    printf("client close \n");
                    FD_CLR(conn, &allset);
                    client[i] = -1;
                    close(conn);
                }
        
                fputs(recvbuf, stdout);
                writen(conn, recvbuf, strlen(recvbuf));
                
                if (--nready <= 0)
                    break; 
            }
        }


    }
        
    return 0;
}
