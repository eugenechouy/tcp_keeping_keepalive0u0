#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcp_send.h"
#include <linux/types.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

char server_ip[] = "192.168.0.101";
char client_ip[] = "192.168.0.109";
int ack_count;
int prev_id;

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg, struct nfq_data *nfa, void *data){
    uint32_t id = 0, len = 0;
    struct nfqnl_msg_packet_hdr *ph;
    unsigned char *full_packet;

    ph = nfq_get_msg_packet_hdr(nfa);
    if (ph) {
        len = nfq_get_payload(nfa, &full_packet);
        id = ntohl(ph->packet_id);

        char *pkt_data_ptr = NULL;	
        pkt_data_ptr = full_packet + sizeof(struct iphdr);
        struct ip *iphdr = (struct ip *) full_packet;
        struct tcphdr *tcphdr = (struct tcphdr *) pkt_data_ptr;

        printf("Received Packet No.%d:\n", id);
        printf("    ACK: %u\n", ntohl(tcphdr->th_ack));
        printf("    SEQ: %u\n", ntohl(tcphdr->th_seq));
        printf("    SRC IP: %s\n", inet_ntoa(iphdr->ip_src));
        printf("    SRC PORT: %d\n", ntohs(tcphdr->th_sport));
        printf("    DST IP: %s\n", inet_ntoa(iphdr->ip_dst));
        printf("    DST PORT: %d\n", ntohs(tcphdr->th_dport));

        if( tcphdr->ack && !strcmp(inet_ntoa(iphdr->ip_src), server_ip))
            ack_count++;
        else 
            ack_count = 0;

        if(ack_count > 3)
            TCP_ACK_send(iphdr->ip_dst.s_addr, iphdr->ip_src.s_addr, tcphdr->th_dport, tcphdr->th_sport, tcphdr->th_ack, htonl(ntohl(tcphdr->th_seq)+1));
    }
   
    return nfq_set_verdict(qh, id, NF_ACCEPT, len, full_packet);
}

int main(int argc, char **argv){
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd;
    int rv;
    char buf[4096];

    h = nfq_open();
    if(!h){
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
    if(nfq_unbind_pf(h, AF_INET) < 0){
        fprintf(stderr, "error during nfq_unbind_pf()\n");
		exit(1);
    }
    
    printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
    if (nfq_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nfq_bind_pf()\n");
		exit(1);
	}

    printf("binding this socket to queue '0'\n");
    qh = nfq_create_queue(h, 0, &cb, NULL);
    if(!qh){
        fprintf(stderr, "error during nfq_create_queue()\n");
		exit(1);
    }
   
    printf("setting copy_packet mode\n");
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet_copy mode\n");
		exit(1);
	}

    fd = nfq_fd(h);

    while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
        nfq_handle_packet(h, buf, rv);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}