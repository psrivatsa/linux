///////////////////////////////////////////////////////////////////////////////
// TRACE PARSER for UDP Trace Data
//
// Author  : psrivatsa
//
// Compile : gcc -I. -o pcap_parser pcap_parser.c -lpcap
//
// Usage   : N packets   -> ./pcap_parser ../pcap_000.pcap 4
//           All packets -> ./pcap_parser ../pcap_000.pcap 0
//             
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <netinet/if_ether.h> /* includes net/ethernet.h */

///////////////////////////////////////////////////////////////////////////////
// MAIN Function
///////////////////////////////////////////////////////////////////////////////
int main (int argc, char **argv) {

  // Variables
  const char *pcapFile=argv[1];
  int limit=atoi(argv[2]);
  int count=0;  
  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *p ;// = malloc(sizeof(struct pcap));
  const u_char *packet;
  struct pcap_pkthdr *h = malloc(sizeof(struct pcap_pkthdr));
  struct ether_header *eptr=malloc(sizeof(struct ether_header));  /* net/ethernet.h */

  // Main Logic
  //printf("hw_seq_num|hw_time|hw_status|hw_otype|hw_sidx|hw_oidx|hw_filter_sqn|hw_cond|hw_goal_price\n");
  printf("hw_status|hw_otype|hw_sidx|hw_oidx|hw_filter_sqn|hw_cond|hw_goal_price\n");
  // OPEN PCAP file 
  p=pcap_open_offline(pcapFile,errbuf);  
  // Fetch Packets one-by-one
  while (packet = pcap_next(p,h)) {
    // Wait for count given. If count=0, waits till end of packets
    if ((count >= limit) && !(limit<=0))
      exit(0);
    count++;
    //printf("Got Packet \n");
    eptr = (struct ether_header *) packet;
    // Check if Packet is UDP. Discard others
    if ((ntohs (eptr->ether_type) == ETHERTYPE_IP)  && (packet[23]==0x11) ) {
      //printf("UDP PACKET\n");
      // Get UDP Payload + Header Length
      packet+=38;
      uint16_t udp_length = ntohs(*(uint16_t *)packet);
      // WORD0 
      // SRC ID/HW Sequence number
      packet+=6;
      uint16_t hw_seq_num = ntohs(*(uint16_t *)packet) & 0xFFF;
      //printf("HW Sequence Number = %d \n",hw_seq_num);
      // WORD1 
      // HW Timestamp      
      packet+=2;
      unsigned int hw_time = ntohl(*(unsigned int *)packet);
      //printf("HW Timestamp = %x \n",hw_time);
      // WORD2 
      // Status/OTYPE/SIDX/OIDX
      packet+=4;
      unsigned int hw_word2 = ntohl(*(unsigned int *)packet);
      //uint16_t hw_status_otype = ntohs(*(uint16_t *)packet);
      uint8_t  hw_status = hw_word2 >> 24;
      uint8_t  hw_otype  = (hw_word2 >> 18) & 0x7;
      uint16_t hw_sidx   = (hw_word2 >> 3)  & 0x7FFF;
      uint8_t  hw_oidx   = hw_word2 & 0x7;
      //printf("HW Status = %x \n",hw_status);
      //printf("HW OTYPE  = %d \n",hw_otype);
      //printf("HW SIDX   = %d \n",hw_sidx);
      //printf("HW OIDX   = %d \n",hw_oidx);
      // WORD3 
      packet+=4;
      unsigned int hw_filter_sqn = ntohl(*(unsigned int *)packet);
      //printf("HW FILTER SQN = %d \n",hw_filter_sqn);
      // WORD4 
      packet+=4;
      unsigned int hw_cond = ntohl(*(unsigned int *)packet);
      //printf("HW COND = %d \n",hw_cond);
      // WORD5 
      packet+=4;
      unsigned int hw_goal_price = ntohl(*(unsigned int *)packet);
      //printf("HW GOAL PRICE = %d \n",hw_goal_price);
      //printf("%d|%d|%x|%d|%d|%d|%d|%d|%d|\n",hw_seq_num,hw_time,hw_status,hw_otype,hw_sidx,hw_oidx,hw_filter_sqn,hw_cond,hw_goal_price);
      printf("%x|%d|%d|%d|%d|%d|%d|\n",hw_status,hw_otype,hw_sidx,hw_oidx,hw_filter_sqn,hw_cond,hw_goal_price);
        
    } // end if
  } // end while
} // end main


