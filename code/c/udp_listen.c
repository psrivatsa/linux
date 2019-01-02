//////////////////////////////////////////////////////////////////////////////
// file     : udp_listen.c
// 
// author   : psrivatsa
// 
// Comments : C program to capture UDP Data
//
// Compile  : gcc -I. -o ./udp_listen ./udp_listen.c
//
// Usage    : ./udp_listen <IPV4 ADDR> <PORT>
//           ex ./udp_listen 10.0.1.120 10999
///////////////////////////////////////////////////////////////////////////////

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

///////////////////////////////////////////////////////////////////////////////
// MAIN Function
///////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {

  /////////////////////////////////////////////////////////////////////////////
  // Variables
  /////////////////////////////////////////////////////////////////////////////
  int sock;
  struct sockaddr_in name;
  int bytes, k;
  char packet_r[1032];
  char *packet = malloc(1032);
  char *server_ipv4_c, *server_port_c;
  uint32_t server_ipv4, server_port;
  int num_trace;
//  uint16_t exp_hw_seq_num;
//  int loop_0 = 1;

  /////////////////////////////////////////////////////////////////////////////
  // Check input arguments
  /////////////////////////////////////////////////////////////////////////////
  if (argc != 3) {
    printf("----> ERROR :: Wrong usage of command\n");
    printf(" usage : ./scripts/udp_listen <IPV4> <PORT>\n");
    exit(1);
  }
  server_ipv4_c = argv[1];
  server_port_c = argv[2];

  printf("---->  TRACE CAPTURE. Activating UDP Listen. Opening UDP Socket... \n");

  /////////////////////////////////////////////////////////////////////////////
  // Create socket from which to read
  /////////////////////////////////////////////////////////////////////////////
  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0)   {
    perror("----> ERROR Opening datagram socket");
    exit(1);
  }
 
  /////////////////////////////////////////////////////////////////////////////
  // Bind our local address so that the client can send to us
  /////////////////////////////////////////////////////////////////////////////
  bzero((char *) &name, sizeof(name));
  name.sin_family = AF_INET;
  server_ipv4 = inet_addr(server_ipv4_c); // String to inet addr 
  server_port = strtol(server_port_c, NULL, 10); // BASE10
  //name.sin_addr.s_addr = htonl(0x0AE62A11);
  name.sin_addr.s_addr = server_ipv4; 
  name.sin_port = htons(server_port);
  if (bind(sock, (struct sockaddr *) &name, sizeof(name))) {
    perror("----> ERROR binding datagram socket");
    printf("Error code: %d\n", errno);
    exit(1);
  }
  
  printf("---> Socket has IPV4_ADDR \"%s\" PORT NUMBER \"%d\"\n", server_ipv4_c, server_port);
  // printf("Socket has IPV4_ADDR #%x PORT NUMBER #%d\n", ntohl(name.sin_addr.s_addr), ntohs(name.sin_port));

  /////////////////////////////////////////////////////////////////////////////
  // CORE Trace Extract Logic
  /////////////////////////////////////////////////////////////////////////////
  //printf("hw_seq_num|hw_time|hw_status|hw_otype|hw_sidx|hw_oidx|hw_filter_sqn|hw_cond|hw_goal_price\n");
  // Trace Header(excluded hw_seq_num and hw_time fields)
  printf("hw_seq_num,hw_msg_seq_num,hw_otype,hw_sidx,hw_status,hw_oidx,hw_cond,hw_goal_price\n");

  // Wait on UDP Trace Packets. Max_Size = 1032 (header 8 bytes + 64 traces max * 16 bytes per trace)
  while ((bytes = recv(sock, packet_r, 1032, 0)) > 0) {
    //printf("recv %d bytes : \n", bytes);
    //    for (k = 0; k < bytes; k++) {
    //      printf("%02x", packet_r[k]);
    //    }
    //    printf(i"\n");
    // printf("Bytes = %d:: DATA = ", bytes);
    num_trace = (bytes-8)/16;
    packet = packet_r;
    // WORD0 
    // SRC ID/HW Sequence number
    packet+=2;
    uint16_t hw_seq_num = ntohs(*(uint16_t *)packet) & 0xFFF;
    //printf("HW Sequence Number = %d \n",hw_seq_num);
    // WORD1 
    // HW Timestamp      
    packet+=2;
    unsigned int hw_time = ntohl(*(unsigned int *)packet);
    //printf("HW Timestamp = %x \n",hw_time);
//    //####################################################################################
//    // DEBUG CODE : Check if 1st entry in loop
//    //####################################################################################
//    if (loop_0 == 1) {
//       exp_hw_seq_num = hw_seq_num;
//       loop_0 = 0;
//       exp_hw_seq_num++;
//       exp_hw_seq_num = exp_hw_seq_num & 0xFFF; // 12-bit
//     } else {
//       if (exp_hw_seq_num != hw_seq_num) { 
//         printf("\nMissed UDP Packet %d %d\n", exp_hw_seq_num, hw_seq_num);
//         exp_hw_seq_num = hw_seq_num;
//         exit(1);
//       }
//       exp_hw_seq_num++;
//       exp_hw_seq_num = exp_hw_seq_num & 0xFFF; // 12-bit
//     }
//    //####################################################################################
 
    // for each trace
    for (k = 0; k < num_trace; k++) {
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
      // Per Trace Data
      //printf("%d,%d,%d,%x,%d,%d,%d\n",hw_filter_sqn,hw_otype,hw_sidx,hw_status,hw_oidx,hw_cond,hw_goal_price);
      printf("%d,%d,%d,%d,%x,%d,%d,%d\n",hw_seq_num,hw_filter_sqn,hw_otype,hw_sidx,hw_status,hw_oidx,hw_cond,hw_goal_price);
    } // end for
  
  } // end while

  // Close Socket
  close(sock);

} // end main

