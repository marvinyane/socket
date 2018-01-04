#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <sys/ioctl.h>
#include <net/if.h>


int main(int argc, char** argv)  {
    struct ifreq device;
    int ifindex = -1;

    int packet_socket = socket(AF_PACKET, SOCK_RAW, htons(0x86DC));

    if (packet_socket < 0) {
        perror("open packet socket failed:");
        return 0;
    }

    char* iface = strdup(argv[1]);
    memset(&device, 0, sizeof(device));
    memcpy(device.ifr_name, iface, IFNAMSIZ);

    int error = ioctl(packet_socket, SIOCGIFINDEX, &device);
    if (error < 0) {
        perror("ioctl get index error.");
        return 0;
    }

    ifindex = device.ifr_ifindex;
    printf("device interface index is %d.\n", ifindex);


    /*add membership*/
    struct packet_mreq mreq;
    mreq.mr_ifindex = ifindex;
    mreq.mr_type = PACKET_MR_MULTICAST;
    mreq.mr_alen = 6;
    mreq.mr_address[0] = 0x03;
    mreq.mr_address[1] = 0x00;
    mreq.mr_address[2] = 0x00;
    mreq.mr_address[3] = 0x01;
    mreq.mr_address[4] = 0x04;
    mreq.mr_address[5] = 0x00;

    error = setsockopt(packet_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (error) {
        perror("set socketopt failed:");
    }

    struct sockaddr_ll ifsock_addr;
    memset(&ifsock_addr, 0, sizeof(ifsock_addr));
    ifsock_addr.sll_family = AF_PACKET;
    ifsock_addr.sll_ifindex = ifindex;
    ifsock_addr.sll_protocol = htons(0x86DC);
    ifsock_addr.sll_pkttype = PACKET_HOST | PACKET_MULTICAST;
    error = bind(packet_socket, (struct sockaddr *)&ifsock_addr, sizeof(ifsock_addr));
    if (error < 0) {
        perror("bind socket error:");
        return 0;
    }

    printf("bind socket success;\n");

    while (1) {
        socklen_t size = 0;
        unsigned char frame[1024];
        memset(frame, 0, sizeof(frame));
        error = recvfrom(packet_socket, frame, sizeof(frame), 0, (struct sockaddr*)&ifsock_addr, &size);
        if (error > 0 ) {
            printf("received from %02X-%02X-%02X-%02X-%02X-%02X.\n", ifsock_addr.sll_addr[0],
                                                                    ifsock_addr.sll_addr[1],
                                                                    ifsock_addr.sll_addr[2],
                                                                    ifsock_addr.sll_addr[3],
                                                                    ifsock_addr.sll_addr[4],
                                                                    ifsock_addr.sll_addr[5]
                    );

            printf("size is %d %d.\n", size, error);
            int i = 0;
            for(i = 0;  i < error; i++) {
                printf("%02X ", frame[i]);
            }
            printf("\n---------------------------\n");
        }
        else  {
            perror("recvfrom packet socket failed:");
        }
    }

    return 0;

}
