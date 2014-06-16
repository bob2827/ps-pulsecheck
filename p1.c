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

int sfd = -1;
void intHandle(int sig);

int main(){
    int conStat = -1, sfd = -1;
    struct addrinfo hints, *res;

    signal(SIGINT, intHandle);

    initClientSocket(&hints, &res, "localhost", P1port, &sfd);
    connectClientSocket(res, &sfd, 5);

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

