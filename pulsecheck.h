#ifndef _PULSECHECK_H_
#define _PULSECHECK_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define BACKLOG_DEPTH 5

//smallest value representable in 64bits
//'ll' modifier might be gcc specific
#define MIN64 ( 1ll << 63 )

#define P2host "localhost"
#define P12port "10000"
#define P32port "10002"

int initSrvSocket(struct addrinfo *hints, struct addrinfo **res, char* port,
                  int* sfd, int backlog);
int initClientSocket(struct addrinfo *hints, struct addrinfo **res, char* host,
                     char* port, int* sfd);
int connectClientSocket(struct addrinfo *target, int* sfd, int retryDelay);
int connectSrvSocket(int listnerFD, int *acceptedFD);
#endif
