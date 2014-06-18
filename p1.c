#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>

#include "pulsecheck.h"

int conStat = -1, sfd = -1;
struct addrinfo hints, *res;

void intHandle(int sig);
void resetState();

int main(){
    signal(SIGINT, intHandle);
    signal(SIGPIPE, SIG_IGN);

    while(1){
        initClientSocket(&hints, &res, P2host, P12port, &sfd);
        connectClientSocket(res, &sfd, 5);
        fprintf(stderr, "Connected to P2\n");

        while(1){
            int64_t n = rand() % 1024;
            int64_t sign = rand() % 2 ? -1 : 1;
            n *= sign;
            printf("%"PRId64"\n", n);
            //It's 2014 and there's no standard hton64? Am I missing something?
            int64_t data = htobe64(n);
            conStat = send(sfd, (void*)&data, sizeof(int64_t), 0);
            if(conStat <= 0){
                fprintf(stderr, "Error sending data (%d): %s\n", errno, strerror(errno));
                resetState();
                break;
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
    freeaddrinfo(res);
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
