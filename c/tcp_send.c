#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "tcp_send.h"

int TCP_ACK_send(uint32_t src_ip, uint32_t dst_ip, uint16_t sport, uint16_t dport, uint32_t seq, uint32_t ack){
    int one = 1;
    int rawsocket = 0;

    char buffer[ sizeof(struct tcphdr) + sizeof(struct ip) +1 ];   
    struct ip *ipheader = (struct ip *)buffer;   
    struct tcphdr *tcpheader = (struct tcphdr *) (buffer + sizeof(struct ip)); 

    /* TPC Pseudoheader used in checksum    */
    tcp_phdr_t pseudohdr;  
    char tcpcsumblock[ sizeof(tcp_phdr_t) + 20 ];
    
    struct sockaddr_in dstaddr;  
    memset(&pseudohdr,0,sizeof(tcp_phdr_t));
    memset(&buffer, 0, sizeof(buffer));
    memset(&dstaddr, 0, sizeof(dstaddr));   

    dstaddr.sin_family = AF_INET; 
    dstaddr.sin_port = dport; 
    dstaddr.sin_addr.s_addr = dst_ip;

    if ( (rawsocket = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) < 0 ){
        perror("TCP_ACK_send():socket()"); 
        return -1;
    }

    if( setsockopt(rawsocket, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one)) < 0){
        perror("TCP_ACK_send():setsockopt()"); 
        return -1;
    }

    /* IP Header */
    ipheader->ip_hl = 5; 
    ipheader->ip_v = 4;
    ipheader->ip_tos = 0;
    ipheader->ip_len = htons( sizeof (struct ip) + sizeof (struct tcphdr) );         
    ipheader->ip_off = 0;  
    ipheader->ip_ttl = 64; 
    ipheader->ip_p = 6;  
    ipheader->ip_sum = 0;  /* checksum */
    ipheader->ip_id = htons( 1000 ); 
    ipheader->ip_src.s_addr = src_ip; 
    ipheader->ip_dst.s_addr = dst_ip; 

    /* TCP Header */   
    tcpheader->th_seq = seq;
    tcpheader->th_ack = ack;
    tcpheader->th_x2 = 0;     
    tcpheader->th_off = 5;	
    tcpheader->th_flags = TH_ACK;
    tcpheader->th_win = htons(6400);
    tcpheader->th_urp = 0;    
    tcpheader->th_sport = sport;
    tcpheader->th_dport = dport;
    tcpheader->th_sum = 0;  /* checksum */

    pseudohdr.src = ipheader->ip_src.s_addr;
    pseudohdr.dst = ipheader->ip_dst.s_addr;
    pseudohdr.zero = 0;
    pseudohdr.protocol = ipheader->ip_p;
    pseudohdr.tcplen = htons( sizeof(struct tcphdr) );

    memcpy(tcpcsumblock, &pseudohdr, sizeof(tcp_phdr_t));   
    memcpy(tcpcsumblock + sizeof(tcp_phdr_t), tcpheader, sizeof(struct tcphdr));

    tcpheader->th_sum = cksum((unsigned short *)(tcpcsumblock), sizeof(tcpcsumblock)); /* (RFC 793) */
    ipheader->ip_sum = cksum((unsigned short *)ipheader, sizeof(struct ip)); /* (RFC 791) */

    if ( sendto(rawsocket, buffer, ntohs(ipheader->ip_len), 0, (struct sockaddr *) &dstaddr, sizeof (dstaddr)) < 0)	
        return -1;        

    printf("send 1 packet!\n");
    close(rawsocket);
    return 0;
}  

unsigned short cksum(unsigned short *addr,int len){ 
    register int sum = 0;
    u_short answer = 0;
    register u_short *w = addr;
    register int nleft = len;
    
    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }
    if (nleft == 1) {
        *(u_char *)(&answer) = *(u_char *)w ;
        sum += answer;
    }
    sum = (sum >> 16) + (sum &0xffff); 
    sum += (sum >> 16); 
    answer = ~sum; 
    return(answer);
} 