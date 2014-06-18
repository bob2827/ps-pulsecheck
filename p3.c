#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
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
 #include <unistd.h>
#include <fcntl.h>

#include "pulsecheck.h"

void intHandle(int sig);
void resetState();

int conStat = -1, sfd = -1;
struct addrinfo hints, *res;

#define P2BUFSIZE 10
int64_t p2RxBuf[P2BUFSIZE];

int main(){
    signal(SIGINT, intHandle);
    signal(SIGPIPE, SIG_IGN);

    while(1){
        initClientSocket(&hints, &res, P2host, P32port, &sfd);
        connectClientSocket(res, &sfd, 5);
        fprintf(stderr, "Connected to P2\n");

        time_t lastReset = time(0);
        while(1){
            int r = recv(sfd, &p2RxBuf, P2BUFSIZE*sizeof(int64_t), 0);
            if(r <= 0){
                if((errno != EAGAIN) && (errno != EWOULDBLOCK)){
                    fprintf(stderr, "Error or close on socket\n");
                    resetState();
                    break;
                }
            }

            for(int i = 0; i < r; i += sizeof(int64_t)){
                int64_t p = *(int64_t*)(p2RxBuf+i);
                int64_t data = be64toh(p);
                printf("%"PRId64"\n", data);
            }

            //Check if we're around the one minute mark and send a new state to
            //Prog 2 if needed
            if(time(0) > (lastReset + 60)){
                uint8_t c = rand() % 255;
                conStat = send(sfd, (void*)&c, sizeof(uint8_t), 0);
                if(conStat <= 0){
                    fprintf(stderr, "Error sending data (%d): %s\n", errno, strerror(errno));
                    resetState();
                    break;
                }
                lastReset = time(0);
                printf("Sent new size of %d (%d)\n", c, conStat);
            }
            usleep(250000);
        }
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

void resetState(){
    if(sfd >= 0){
        close(sfd);
        sfd = -1;
    }
    if(conStat >= 0){
        close(conStat);
        conStat = -1;
    }
    if(res){
        freeaddrinfo(res);
        res = NULL;
    }
}
