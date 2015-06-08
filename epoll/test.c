#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EPOLL_MAX_EVENT_SIZE (256)
#define EPOLL_MAX_RECV_SIZE (20)
#define EPOLL_TIME_OUT (500)


int main()
{ 
    struct epoll_event ev;
    struct epoll_event events[EPOLL_MAX_RECV_SIZE];

    int epfd = epoll_create(EPOLL_MAX_EVENT_SIZE);

    if(epfd < 0)
    {
        perror("epoll create failed: ");
        return 0;
    }


   FILE* fp = fopen("11", "rw");

   if(fp < 0)
   {
       perror("file open failed:");
       return 0;
   }

   printf("create file success : %d \n", (int)fp);

   ev.events = EPOLLOUT;
   ev.data.fd = (int)fp;

   epoll_ctl(epfd, EPOLL_CTL_ADD, (int)fp, &ev);

   while(1)
   {
       int nfds = epoll_wait(epfd, events, EPOLL_MAX_RECV_SIZE, EPOLL_TIME_OUT);

       int i = 0;

       for(; i < nfds; i++)
       {
           printf("receiced data from %d \n",  events[i].data.fd);
       }
   }


   return 0;

}
