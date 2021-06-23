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
#include <arpa/inet.h>

#define MTU 1024
#define ERROR(a) {perror(a); exit(1);}

void execute(char* cmd);
void setIptable();
void cleanIptable();
int max(int a, int b);


int main(int argc, char** argv){

    if(argc != 3){
        ERROR("[ERROR] the usage is : ./client [server_ip] [port_num]")
    }

    int tunc_fd;
    char SERVER_IP[1024];
    strcpy(SERVER_IP, argv[1]);
    const int SERVER_PROT = atoi(argv[2]);
    
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
    sprintf(cmd, "ifconfig mytun 10.8.0.2/16 mtu %d up", MTU);
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
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    socklen_t serverAddrLen = sizeof(serverAddr);
   

    if (connect(tcp_fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0){
        ERROR("connect failed");
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

void execute(char* cmd){
    printf("[Info] Executing: %s\n", cmd);
    if (system(cmd) < 0){
        ERROR(cmd);
        
    }
}

void setIptable(){
    execute("iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE");
    execute("iptables -I FORWARD 1 -i tun0 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    execute("iptables -I FORWARD 1 -o tun0 -j ACCEPT");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ip route add %s via $(ip route show 0/0 | sed -e 's/.* via \([^ ]*\).*/\1/')", "");
    execute(cmd);
    execute("ip route add 0/1 dev tun0");
    execute("ip route add 128/1 dev tun0");
}

void cleanIptable(){
    execute("iptables -t nat -D POSTROUTING -o tun0 -j MASQUERADE");
    execute("iptables -D FORWARD -i tun0 -m state --state RELATED,ESTABLISHED -j ACCEPT");
    execute("iptables -D FORWARD -o tun0 -j ACCEPT");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ip route del %s", "");
    execute(cmd);
    execute("ip route del 0/1");
    execute("ip route del 128/1");
}

int max(int a, int b){
    return ((a > b) ? a : b);
}
