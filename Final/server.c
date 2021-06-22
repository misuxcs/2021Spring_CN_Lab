#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl>


int main(int argc, char** argv){
    int tunc_fd;
    const int SERVER_PROT = atoi(argv[1]);
    
    if((tunc_fd = open("/dev/net/tun", O_RDWR)) < 0){
        perror("Cannot open /dev/net/tun")
    }

    struct ifreq ifr;

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
    strcpy(ifr.ifr_name, "mytun");

    int e
    if( (e=ioctl(tunc_fd, TUNSETIFF, (void*) &ifr)) < 0 ){
        close(tunc_fd);
        perror("ioctl error");
    }
    
    close(tun_fd);
    
    return 0;
}
