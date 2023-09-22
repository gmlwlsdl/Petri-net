#include <stdlib.h>
#include <stdio.h>
#include <pcap.h>
#include <arpa/inet.h>

#include "myheadr.h"

void got_packet(u_char *args, const struct pcap_pkthdr *header,
                              const u_char *packet)
{
  struct ethheader *eth = (struct ethheader *)packet;

  // 출력 대상 MAC 주소를 문자열로 변환
  char dest_mac[18];
  char source_mac[18];
  printf("\n");
  sprintf(dest_mac, "%02x:%02x:%02x:%02x:%02x:%02x", eth->ether_dhost[0], eth->ether_dhost[1], eth->ether_dhost[2], eth->ether_dhost[3], eth->ether_dhost[4], eth->ether_dhost[5]);
  sprintf(source_mac, "%02x:%02x:%02x:%02x:%02x:%02x", eth->ether_shost[0], eth->ether_shost[1], eth->ether_shost[2], eth->ether_shost[3], eth->ether_shost[4], eth->ether_shost[5]);

  printf("Destination MAC: %s\n", dest_mac);
  printf("Source MAC: %s\n", source_mac);

  if (ntohs(eth->ether_type) == 0x0800) { // 0x0800 is IP type
    struct ipheader * ip = (struct ipheader *)
                           (packet + sizeof(struct ethheader)); 

    printf("       From: %s\n", inet_ntoa(ip->iph_sourceip));   
    printf("         To: %s\n", inet_ntoa(ip->iph_destip));    

    /* determine protocol */
    switch(ip->iph_protocol) {                                 
        case IPPROTO_TCP:
            printf("   Protocol: TCP\n");
            return;
        case IPPROTO_UDP:
            printf("   Protocol: UDP\n");
            return;
        case IPPROTO_ICMP:
            printf("   Protocol: ICMP\n");
            return;
        default:
            printf("   Protocol: others\n");
            return;
    }

    if (ip->iph_protocol == IPPROTO_TCP) { // If it's a TCP packet
      struct tcpheader *tcp = (struct tcpheader *)(packet + sizeof(struct ethheader) + sizeof(struct ipheader));

      printf("   Source Port: %d\n", ntohs(tcp->tcp_sport));
      printf("   Destination Port: %d\n", ntohs(tcp->tcp_dport));
      printf("   Sequence Number: %u\n", ntohl(tcp->tcp_seq));
      printf("   Acknowledgment Number: %u\n", ntohl(tcp->tcp_ack));
      printf("   Flags: 0x%02x\n", tcp->tcp_flags);
      printf("   Window Size: %d\n", ntohs(tcp->tcp_win));
      printf("   Checksum: 0x%04x\n", ntohs(tcp->tcp_sum));
      printf("   Urgent Pointer: %d\n", ntohs(tcp->tcp_urp));
    }
  }
}

int main()
{
  pcap_t *handle;
  char errbuf[PCAP_ERRBUF_SIZE];
  struct bpf_program fp;
  char filter_exp[] = "icmp";
  bpf_u_int32 net;

  // Step 1: Open live pcap session on NIC with name enp0s3
  handle = pcap_open_live("enp0s3", BUFSIZ, 1, 1000, errbuf);

  // Step 2: Compile filter_exp into BPF psuedo-code
  pcap_compile(handle, &fp, filter_exp, 0, net);
  if (pcap_setfilter(handle, &fp) !=0) {
      pcap_perror(handle, "Error:");
      exit(EXIT_FAILURE);
  }

  // Step 3: Capture packets
  pcap_loop(handle, -1, got_packet, NULL);

  pcap_close(handle);   //Close the handle
  return 0;
}

