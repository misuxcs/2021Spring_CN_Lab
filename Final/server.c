#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <netinet/in.h>

#define MTU 1024
#define ERROR(a) {perror(a); exit(1);}

void execute(char* cmd);
void setIptable();
void cleanIptable();
int max(int a, int b);


int main(int argc, char** argv){
    if(argc != 2){
        ERROR("[ERROR] the usage is : ./server [port_num]")
    }

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

    printf("[Info] setting ifconfig...");
    /* setup ifconfig */
    char cmd[1024];
    sprintf(cmd, "ifconfig mytun 10.8.0.1/16 mtu %d up", MTU);
    execute(cmd);

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
    socklen_t serverAddrLen = sizeof(serverAddr);

    if (listen(tcp_fd, 3) < 0){
        ERROR("listen failed");
    }
    
    /*read write data*/
    fd_set readset;
    int max_fd = max(tunc_fd, tcp_fd) + 1;
    char tun_buff[MTU], tcp_buff[MTU];
    bzero(tun_buff, MTU);
    bzero(tcp_buff, MTU);
    
    while(1) {
        FD_ZERO(&readset);
        FD_SET(tunc_fd, &readset);
        FD_SET(tcp_fd, &readset);
        if( select(max_fd, &readset, NULL, NULL, NULL) < 0 ){
            ERROR("select failed");
        }

        if(FD_ISSET(tunc_fd, &readset)){
            int r = read(tunc_fd, tun_buff, MTU);
            if (r < 0) {
                perror("read from tun_fd error");
                break;
            }

            printf("Writing to TCP %d bytes ...\n", r);
            r = sendto(tcp_fd, tcp_buff, r, 0, (const struct sockaddr *)&serverAddr, serverAddrLen);
            if (r < 0) {
                perror("sendto tcp_fd error");
                break;
            }
        else if(FD_ISSET(tcp_fd, &readset)){
            int r = recvfrom(tcp_fd, tcp_buff, MTU, 0, (struct sockaddr *)&serverAddr, &serverAddrLen);
            if (r < 0) {
                perror("recvfrom tcp_fd error");
                break;
            }

            printf("Writing to tun %d bytes ...\n", r);
            r = write(tunc_fd, tun_buff, r);
            if (r < 0) {
                perror("write tun_fd error");
                break;
            }
        }
        else continue;
        }

        cleanIptable();
        close(tunc_fd);
        
        return 0;
    }
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
void cleanIptable(){
    execute("iptables -t nat -D POSTROUTING -s 10.8.0.0/16 ! -d 10.8.0.0/16 -m comment --comment 'vpndemo' -j MASQUERADE");
    execute("iptables -D FORWARD -s 10.8.0.0/16 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    execute("iptables -D FORWARD -d 10.8.0.0/16 -j ACCEPT");

}

int max(int a, int b){
    return ((a > b) ? a : b);
}
