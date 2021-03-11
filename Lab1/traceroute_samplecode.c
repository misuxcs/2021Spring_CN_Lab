#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<errno.h>
#include<netinet/in.h>
#include<netinet/ip.h>
#include<netinet/ip_icmp.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/time.h>

typedef struct{
    unsigned char Type;
    unsigned char Code;
    unsigned char Checksum[2];
    unsigned char Identifier[2];
    signed char Sequence_Number[2];
}ICMPHeader;

char *DNSLookup(char *host){
    // TODO
    struct addrinfo *addr;
    int result = getaddrinfo(host, NULL, NULL, &addr);
    if(result != 0){
        printf("Error from getaddrinfo\n");
        exit(1);
    }
    struct sockaddr_in *internetAddr = (struct sockaddr_in*) addr->ai_addr;
    return inet_ntoa(internetAddr->sin_addr);
}

unsigned char calcChecksum(unsigned char type, unsigned char id, signed char seq){
    unsigned char checksum[2];
    
    
    return checksum;
}

int main(int argc, char *argv[]){
    char *dest = argv[1];
    char *ip = DNSLookup(dest);
    if(ip == NULL){
        printf("traceroute: unknown host %s\n", dest);
        exit(1);
    }
    int icmpfd;
    if((icmpfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0){
        printf("Can not open socket with error number %d\n", errno);
        exit(1);
    }
    
    struct sockaddr_in sendAddr;
    sendAddr.sin_port = htons (7);
    sendAddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(sendAddr.sin_addr));
    
    // Set timeout
    // TODO
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    if(setsockopt(icmpfdd, SOL_SOLKET, SO_RCVTIMEO, sizeof(timeout)) < 0){
    	error("setsockopt failed\n");
    }
    if(setsockopt(icmpfdd, SOL_SOLKET, SO_SNDTIMEO, sizeof(timeout)) < 0){
    	error("setsockopt failed\n");
    }

    int finish = 0; // if the packet reaches the destination
    int maxHop = 64; // maximum hops
    struct icmp sendICMP; 
    struct timeval begin, end; // used to record RTT
    int seq = 0; // increasing sequence number for icmp packet
    int count = 3; // sending count for each ttl
    printf("traceroute to %s (%s), %d hops max\n", dest, ip, maxHop);
    for(int h = 1; h < maxHop; h++){
        // Set TTL
        // TODO
	int TTL = h;
	setsockopt(icmpfd, IPPROTO_IP, IP_TTL, &TTL, sizeof(TTL));
        
	for(int c = 0; c < count; c++){
            // Set ICMP Header
            // TODO
		ICMPHeader icmpHeader;
		icmpHeader.Type = 8;
		icmpHeader.Code = 0;
		icmpHeader.Identifier = 318;
		icmpHeader.Sequence_Number = c*h;  
            // Checksum
            // TODO 
		icmpHeader.Checksum = ;
            // Send the icmp packet to destination
            // TODO
        
            // Recive ICMP reply, need to check the identifier and sequence number
            struct ip *recvIP;
            struct icmp *recvICMP;
            struct sockaddr_in recvAddr;
            u_int8_t icmpType;
            unsigned int recvLength = sizeof(recvAddr);
            char recvBuf[1500];
            char hostname[4][128];
            char srcIP[4][32];
            float interval[4] = {};
            memset(&recvAddr, 0, sizeof(struct sockaddr_in));
            // TODO

            // Get source hostname and ip address 
            getnameinfo((struct sockaddr *)&recvAddr, sizeof(recvAddr), hostname[c], sizeof(hostname[c]), NULL, 0, 0); 
            strcpy(srcIP[c], inet_ntoa(recvIP->ip_src));
            if(icmpType == 0){
                finish = 1;
            }

            // Print the result
            // TODO
        }    
        if(finish){
            break;
        }
    }
    close(icmpfd);
    return 0;
}
