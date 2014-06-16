#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "pulsecheck.h"

void intHandle(int sig);

int sfd = -1;

#define P2BUFSIZE 10
int64_t p2RxBuf[P2BUFSIZE];

int main(){
    int conStat = -1, sfd = -1;
    struct addrinfo hints, *res;
    
    signal(SIGINT, intHandle);

    initClientSocket(&hints, &res, P2host, P32port, &sfd);
    connectClientSocket(res, &sfd, 5); 

    time_t lastReset = time(0);
    while(1){
        int r = recv(sfd, &p2RxBuf, P2BUFSIZE*sizeof(int64_t), 0);
        for(int i = 0; i < r; i += sizeof(int64_t)){
            int64_t p = *(int64_t*)(p2RxBuf+i);
            int64_t data = be64toh(p);
            printf("%"PRId64"\n", data);
        }
        if(time(0) > (lastReset + 10)){
            uint8_t c = rand() % 255;
            conStat = send(sfd, (void*)&c, sizeof(uint8_t), 0);
            lastReset = time(0);
            printf("Sent new size of %d\n", c);
        }
        usleep(250000);
    }
    return 0;
}

void intHandle(int sig){
    printf("Caught SIGINT (Ctrl-C), exiting\n");
    if(sfd >= 0){
        close(sfd);
    }
    exit(0);
}
