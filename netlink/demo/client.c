#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>

#define NETLINK_TEST 17
#define MSG_LEN 100

struct u_packet_info
{
    struct nlmsghdr hdr;
    char msg[MSG_LEN];
};

int main(int argc, char* argv[]) 
{
    int ret = 0;
    char *data = "hello world";

    struct sockaddr_nl local;

    struct nlmsghdr *message;
    struct u_packet_info info;
    char *retval;

    // TODO:
    message = (struct nlmsghdr *)malloc(1);

    int skfd = socket(PF_NETLINK, SOCK_RAW, NETLINK_GENERIC);

    if(skfd < 0){
        printf("can not create a netlink socket\n");
        return -1;
    }

    memset(&local, 0, sizeof(local));
    local.nl_family = AF_NETLINK;
    local.nl_pid = 0;
    local.nl_groups = 0x21;

    if(connect(skfd, (struct sockaddr *)&local, sizeof(local)) != 0){
        perror("connect error\n");
        return -1;
    }

    memset(message, '\0', sizeof(struct nlmsghdr));
    message->nlmsg_len = NLMSG_SPACE(strlen(data));
    message->nlmsg_flags = 0;
    message->nlmsg_type = 0;
    message->nlmsg_seq = 0;
    message->nlmsg_pid = local.nl_pid;

    retval = memcpy(NLMSG_DATA(message), data, strlen(data));

    printf("message sendto kernel are:%s, len:%d\n", (char *)NLMSG_DATA(message), message->nlmsg_len);
    ret = send(skfd, message, message->nlmsg_len, 0);
    if(!ret){
        perror("send pid:");
        exit(-1);
    }

#if 0
    //接受内核态确认信息
    ret = recvfrom(skfd, &info, sizeof(struct u_packet_info),0, (struct sockaddr*)&kpeer, &kpeerlen);
    if(!ret){
        perror("recv form kerner:");
        exit(-1);
    }

    printf("message receive from kernel:%s\n",(char *)info.msg);
    //内核和用户进行通信
#endif
    close(skfd);
    return 0;
}
