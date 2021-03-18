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


typedef unsigned short int      uint16_t;  
typedef unsigned int            uint32_t;

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

uint16_t checksum(uint32_t type, uint32_t identifier, uint32_t sequence){
    uint32_t sum1 = type + identifier + sequence;
	uint16_t higherbit = sum1 >> 16;
	uint16_t lowerbit = sum1 & 65535;
	uint16_t sum2 = higherbit + lowerbit;
	uint16_t complement = ~sum2;
	return htons(complement);
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
    if(setsockopt(icmpfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    	perror("setsockopt failed\n");
    }
    if(setsockopt(icmpfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0){
    	perror("setsockopt failed\n");
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
        struct timeval rtt[3];
	    int TTL = h;
	    setsockopt(icmpfd, IPPROTO_IP, IP_TTL, &TTL, sizeof(TTL));
	    for(int c = 0; c < count; c++){
            // Set ICMP Header
            // TODO
            sendICMP.icmp_type = 8;
            sendICMP.icmp_code = 0;
            sendICMP.icmp_hun.ih_idseq.icd_id = htons(318);
            sendICMP.icmp_hun.ih_idseq.icd_seq = htons(seq);
            
	        // Checksum
            // TODO
            sendICMP.icmp_cksum = checksum(2048, 318, seq);
            
            // Send the icmp packet to destination
            // TODO
       	    if(sendto(icmpfd, (void* )&sendICMP, sizeof(sendICMP), 0, (struct sockaddr*)&sendAddr, sizeof(struct sockaddr)) < 0){
		        perror("Send icmp packet failed!");
	        }
	        gettimeofday(&begin, NULL);
            
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
            char testIP;
	        // TODO
	        // Receive icmp packet
            int recvOrNot = 0;
            while(recvOrNot == 0){
                int recvLen = recvfrom(icmpfd, recvBuf, sizeof(recvBuf), 0, (struct sockaddr*)&recvAddr, &recvLength);
                if( recvLen < 0){
                    perror("Receive icmp packet failed!");
                }
                printf("\nrecv len: %d\n", recvLen);
                // Check identifier and sequence number
                /*
                for(int i = 0 ; i < recvLen ; i++){
                    testIP = *(recvBuf+i);
                    printf("%d %x\n", i,testIP);
                    //if(i % 8 == 0 && i != 0) printf("\n");
                }
                */
                char myId[2];
                myId[0] = *(recvBuf+24);
                myId[1] = *(recvBuf+25);
                int recvId = myId[0]*256 + myId[1];
                //printf("ID: %d\n", myId[0]*256 + myId[1]);
                
                char mySeq[2];
                mySeq[0] = *(recvBuf+26);
                mySeq[1] = *(recvBuf+27);
                int recvSeq = mySeq[0]*256 + mySeq[1];
                //printf("SEQ: %d\n", mySeq[0]*256 + mySeq[1]);
                if(recvId == 318 && recvSeq == seq){
                    recvOrNot = 1;
                }
            }
            seq++;

            char mySrcIP[4];
            mySrcIP[0] = *(recvBuf+12);
            mySrcIP[1] = *(recvBuf+13);
            mySrcIP[2] = *(recvBuf+14);
            mySrcIP[3] = *(recvBuf+15);
            char recvSrcIp[50];
            sprintf(recvSrcIp, "%d.%d.%d.%d", mySrcIP[0]&255,mySrcIP[1]&255,mySrcIP[2]&255,mySrcIP[3]&255);

            // Calculate the response time
            gettimeofday(&end, NULL);
            
            
            rtt[c].tv_sec = end.tv_sec - begin.tv_sec;
            rtt[c].tv_usec = end.tv_usec - begin.tv_usec;
            
	        // Get source hostname and ip address 
            getnameinfo((struct sockaddr *)&recvAddr, sizeof(recvAddr), hostname[c], sizeof(hostname[c]), NULL, 0, 0); 
            //strcpy(srcIP[c], inet_ntoa(recvIP->ip_src));
            if(icmpType == 0){
                finish = 1;
            }
            // Print the result
            // TODO
            if(c == 0){
                printf("%d, %s", h, recvSrcIp);
            }
            printf(", %ld.%03d", rtt[c].tv_sec, rtt[c].tv_usec);
        }
        printf("\n");
        if(finish){
            break;
        }
    }
    close(icmpfd);
    return 0;
}
