#include "pulsecheck.h"
#include <string.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

int initSrvSocket(struct addrinfo *hints, struct addrinfo **res, char* port,
                  int* sfd, int backlog)
{
    memset(hints, 0, sizeof(struct addrinfo));

    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, port, hints, res);

    *sfd = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    fcntl(*sfd, F_SETFL, O_NONBLOCK);
    bind(*sfd, (*res)->ai_addr, (*res)->ai_addrlen);
    listen(*sfd, BACKLOG_DEPTH);
}

int connectSrvSocket(int listnerFD, int *acceptedFD)
                     //struct sockaddr_storage acceptedAddr, socklen_t size)
{
    struct sockaddr_storage address;
    socklen_t addrSize = sizeof(struct sockaddr_storage);

    *acceptedFD = -1;
    *acceptedFD = accept(listnerFD, (struct sockaddr *)&address, &addrSize);
    if(*acceptedFD < 0){
        if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
            printf("Waiting for connect...\n");
        }else{
            printf("Error on P1 socket: %s\n", strerror(errno));
            return -1;
        }
    }else{
        return 0;
    }
}

int initClientSocket(struct addrinfo *hints, struct addrinfo **res, char* host,
                     char* port, int* sfd)
{
    memset(hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, hints, res);
    *sfd = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
}

int connectClientSocket(struct addrinfo *target, int* sfd, int retryDelay){
    int conStat = -1;
    while(conStat < 0){
        conStat = connect(*sfd, target->ai_addr, target->ai_addrlen);
        if(conStat < 0){
            fprintf(stderr, "Could not open socket (%d): %s\n", errno,
                    strerror(errno));
            switch(errno){
            }
            fprintf(stderr, "Retrying...\n");
            sleep(retryDelay);
        }
    }
    return 0;
}
