#ifndef TCP_SEND_H_
#define TCP_SEND_H_

#ifndef __USE_BSD
#define __USE_BSD
#endif 

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>

#ifndef __FAVOR_BSD
#define __FAVOR_BSD
#endif

#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

typedef struct pseudoheader {
    uint32_t src;
    uint32_t dst;
    u_char zero;
    u_char protocol;
    uint16_t tcplen;
} tcp_phdr_t;

int TCP_ACK_send(uint32_t src_ip, uint32_t dst_ip, uint16_t sport, uint16_t dport, uint32_t seq, uint32_t ack);
unsigned short cksum(unsigned short *addr, int len);

#endif