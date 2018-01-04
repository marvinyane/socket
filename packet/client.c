#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h> /* the L2 protocols */

#include <sys/ioctl.h>
#include <net/if.h>

#include <netinet/ether.h>

// d4:be:d9:a2:65:83

#define MY_DEST_MAC0    0xd4
#define MY_DEST_MAC1    0xbe
#define MY_DEST_MAC2    0xd9
#define MY_DEST_MAC3    0xa2
#define MY_DEST_MAC4    0x65
#define MY_DEST_MAC5    0x83


int main(int argc, char** argv)  {
    struct ifreq device;
    int ifindex = -1;

    int packet_socket = socket(AF_PACKET, SOCK_RAW, htons(0x86DC));

    if (packet_socket < 0) {
        perror("open packet socket failed:");
        return 0;
    }

    printf("create socket success %d.\n", packet_socket);

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

    struct ifreq if_mac;
    memset(&if_mac, 0, sizeof(struct ifreq));
    strncpy(if_mac.ifr_name, iface, IFNAMSIZ-1);
    if (ioctl(packet_socket, SIOCGIFHWADDR, &if_mac) < 0)
        perror("SIOCGIFHWADDR");

    char sendbuf[1024];
    struct ether_header *eh = (struct ether_header*)sendbuf;
    memset(sendbuf, 0, sizeof(sendbuf));

    eh->ether_shost[0] = ((unsigned char*)&if_mac.ifr_hwaddr.sa_data)[0];
    eh->ether_shost[1] = ((unsigned char *)&if_mac.ifr_hwaddr.sa_data)[1];
    eh->ether_shost[2] = ((unsigned char *)&if_mac.ifr_hwaddr.sa_data)[2];
    eh->ether_shost[3] = ((unsigned char *)&if_mac.ifr_hwaddr.sa_data)[3];
    eh->ether_shost[4] = ((unsigned char *)&if_mac.ifr_hwaddr.sa_data)[4];
    eh->ether_shost[5] = ((unsigned char *)&if_mac.ifr_hwaddr.sa_data)[5];
    eh->ether_dhost[0] = MY_DEST_MAC0;
    eh->ether_dhost[1] = MY_DEST_MAC1;
    eh->ether_dhost[2] = MY_DEST_MAC2;
    eh->ether_dhost[3] = MY_DEST_MAC3;
    eh->ether_dhost[4] = MY_DEST_MAC4;
    eh->ether_dhost[5] = MY_DEST_MAC5;

    eh->ether_type = htons(0x86DC);
    int tx_len = sizeof(struct ether_header);
    sendbuf[tx_len++] = 'h';
    sendbuf[tx_len++] = 'e';
    sendbuf[tx_len++] = 'l';
    sendbuf[tx_len++] = 'l';
    sendbuf[tx_len++] = 'o';
    sendbuf[tx_len++] = 0;

    struct sockaddr_ll ifsock_addr;
    memset(&ifsock_addr, 0, sizeof(ifsock_addr));
    ifsock_addr.sll_family = AF_PACKET;
    ifsock_addr.sll_ifindex = ifindex;
    ifsock_addr.sll_addr[0] = MY_DEST_MAC0;
    ifsock_addr.sll_addr[1] = MY_DEST_MAC1;
    ifsock_addr.sll_addr[2] = MY_DEST_MAC2;
    ifsock_addr.sll_addr[3] = MY_DEST_MAC3;
    ifsock_addr.sll_addr[4] = MY_DEST_MAC4;
    ifsock_addr.sll_addr[5] = MY_DEST_MAC5;
    ifsock_addr.sll_halen = ETH_ALEN;

    error = sendto(packet_socket, sendbuf, tx_len, 0, (struct sockaddr*)&ifsock_addr, sizeof(ifsock_addr));
    if (error < 0) {
        perror("sentto failed.");
        return 0;
    }

    printf("send to with result %d.\n", error);

    return 0;

}
