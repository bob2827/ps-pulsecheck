#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>

#include "pulsecheck.h"

//The largest possible cache is under 2K, so I think statically allocated
//memory is accpetable If we had some significant maxium size I'd probably
//dynamically size this as changes were submitted by prog 3
#define STORESIZE 255
long long int storage[STORESIZE]; 
unsigned char N = 20;

int sfd1 = -1, sfd3 = -1;

int main(){
    //Initialize the storage array
    long long int unfilled = 1ll << 63; //'ll' modifier might be gcc specific
    for(int i = 0; i < STORESIZE; i++){
        storage[i] = unfilled;
    }

    //Initialize network data structures
    struct addrinfo P1hints, P3hints, *P1res, *P3res;
    memset(&P1hints, 0, sizeof(P1hints));
    memset(&P3hints, 0, sizeof(P3hints));

    struct sockaddr_storage p1remote, p3remote;
    socklen_t p1r_size, p3r_size;

    P1hints.ai_family = P3hints.ai_family = AF_INET;
    P1hints.ai_socktype = P3hints.ai_socktype = SOCK_STREAM;
    P1hints.ai_flags = P3hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, P1port, &P1hints, &P1res);
    getaddrinfo(NULL, P3port, &P3hints, &P3res);

    sfd1 = socket(P1res->ai_family, P1res->ai_socktype, P1res->ai_protocol);
    sfd3 = socket(P3res->ai_family, P3res->ai_socktype, P3res->ai_protocol);
    fcntl(sfd1, F_SETFL, O_NONBLOCK);
    fcntl(sfd3, F_SETFL, O_NONBLOCK);
    bind(sfd1, P1res->ai_addr, P1res->ai_addrlen);
    bind(sfd3, P3res->ai_addr, P3res->ai_addrlen);
    listen(sfd1, BACKLOG_DEPTH);
    listen(sfd3, BACKLOG_DEPTH);

    int sfd1Live = accept(sfd1, (struct sockaddr *)&p1remote, &p1r_size);
    int sfd3Live = accept(sfd3, (struct sockaddr *)&p3remote, &p3r_size);
    if(sfd3Live < 0){
        fprintf(stderr, "Error accepting (%d): %s\n", errno, strerror(errno));
    }
    if((errno == EAGAIN) || (errno == EWOULDBLOCK)){
    printf("!!!\n");
    }
    printf("%d\n", sfd1Live);

    return 0;
}

//The prompt does not define appropriate behavior for duplicate values. I have
//chosen to not store duplicates in the list, but to still notify prog3 if a
//duplicate of the maxium is received.

/*
void addAndResort(long long int newVal){
    if(newVal > storage[N-1]){
        storage[N-1] = newVal;
    }
}

resizeStorage(){
    if(shrinking)
        zero the elements leaving the structure
}
*/
