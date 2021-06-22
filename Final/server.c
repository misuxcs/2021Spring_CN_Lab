#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <netinet/in.h>

#define MTU 1400
#define ERROR(a) {perror(a); exit(1);}

void execute(char* cmd);
void setIptable();
void cleanIptable();


int main(int argc, char** argv){
    int tunc_fd;
    const int SERVER_PROT = atoi(argv[1]);
    
    if((tunc_fd = open("/dev/net/tun", O_RDWR)) < 0){
        ERROR("[Error] Cannot open /dev/net/tun");
    }

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strcpy(ifr.ifr_name, "mytun");

    int e;
    if( (e=ioctl(tunc_fd, TUNSETIFF, (void*) &ifr)) < 0 ){
        close(tunc_fd);
        ERROR("[Error] ioctl error");
    }

    /* setup ifconfig */
    execute("ifconfig mytun 10.8.0.1/16 mtu %d up", MTU);

    setIptable();

    /* create socket */
    int tcp_fd;
    if ((tcp_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        ERROR("[Error] socket failed");
    }

    /* bind socket */
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PROT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(tcp_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        ERROR("bind failed");
    }

    if (listen(tcp_fd, 3) < 0){
        ERROR("listen failed");
    }
    

    cleanIptable();
    close(tunc_fd);
    
    return 0;
}

void execute(char* cmd){
    printf("[Info] Executing: %s\n", cmd);
    if (system(cmd) < 0){
        ERROR(cmd);
        
    }
}

void setIptable(){
    execute("iptables -t nat -A POSTROUTING -s 10.8.0.0/16 ! -d 10.8.0.0/16 -m comment --comment 'vpndemo' -j MASQUERADE");
    execute("iptables -A FORWARD -s 10.8.0.0/16 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    execute("iptables -A FORWARD -d 10.8.0.0/16 -j ACCEPT");
}
void cleanIptable();