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

#include "pulsecheck.h"

void intHandle(int sig);

int sfd = -1;

int main(){
    char data[1024];
    int conStat = -1, sfd = -1;
    struct addrinfo hints, *res;
    initClientSocket(&hints, &res, "localhost", P3port, &sfd);

    while(conStat < 0){
        conStat = connect(sfd, res->ai_addr, res->ai_addrlen);
        if(conStat < 0){
            fprintf(stderr, "Could not open socket (%d): %s\n", errno, strerror(errno));
            switch(errno){
            }
            fprintf(stderr, "Retrying...\n");
            sleep(5);
        }
    }

    signal(SIGINT, intHandle);

    while(1){
        int r = recv(sfd, &data, 1024, 0);

    }
    return 0;
}

void intHandle(int sig){
    printf("Caught SIGINT (Ctrl-C), exiting\n");
    close(sfd);
    exit(0);
}
