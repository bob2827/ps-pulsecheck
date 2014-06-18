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
#include <sys/select.h>

#include "pulsecheck.h"

//The largest possible cache is under 2K, so I think statically allocated
//memory is accpetable here. If we had some significant maxium size I'd probably
//dynamically size this as changes were submitted by prog 3
#define STORESIZE 255

#define P1BUFSIZE 10
#define P3BUFSIZE 10

int64_t storage[STORESIZE];
unsigned char N = 20;

int sfd1 = -1, sfd3 = -1;
int sfd1Live = -1, sfd3Live = -1;
struct addrinfo *P1res, *P3res;

/* Checks to see if a new value belongs in the top N set and adds it if
 * appropriate. The prompt does not define appropriate behavior for duplicate
 * values. I have chosen to not store duplicates in the list, but to still
 * notify prog3 if a duplicate top N is received
 * returns 1 if the new element falls into the top N, 0 otherwise
 * storage[0] - largest element
 * storage[N] - smallest element
*/
int addAndOrder(int64_t newVal){
    int i = N-1;
    //Walk up the storage list until we find the point at which newval stops
    //being bigger than things already on the list
    for(; (i >= 0) && (newVal > storage[i]); i--){}
    i++; //adjust by +1 after loop termination, so i represents newval's
         //desired location

    if(i == N){ //newval matches the final element or didn't make the cut
        if(newVal == storage[N-1]){
            return 1;
        }else{
            return 0;
        }
    }else{ //newval falls within the storage area
        if(newVal == storage[i]){ //we match something already on the list
            //we don't need to do anything other than tell prog 3
        }else if(i == (N-1)){ //if we're replacing the final element, we
                              //don't need the memmove
            storage[N-1] = newVal;
        }else{ //we have to make space for newval & insert
            //location where newval is to be inserted
            void* src = (void*)storage + sizeof(int64_t)*i;
            void* dest = src+sizeof(int64_t); //one space past there
            size_t n = (N-i)*sizeof(int64_t);
            memmove(dest, src, n);
            storage[i] = newVal;
        }
        return 1;
    }
}

/* Resets the boundary of the storage buffer used for the top N values
 * newsize - the new size of the storage buffer
 */
void resizeStorage(uint8_t newsize){
    if(newsize > N){
        for(int i = N; i < newsize-1; i++){
            storage[i] = MIN64;
        }
    }
    N = newsize;
}

/* Prints out the storage buffer */
void showStorage(){
    for(int i = 0; i < N; i++){
        fprintf(stdout, "%"PRId64", ", storage[i]);
    }
    fprintf(stdout, "\n");
}

int processP1(int sfd1Live, int sfd3Live){
    int64_t p1RxBuf[P1BUFSIZE];
    int r = recv(sfd1Live, p1RxBuf, sizeof(int64_t)*P1BUFSIZE, 0);
    if(r <= 0){
        return -1;
    }
    for(int i = 0; i < r/sizeof(int64_t); i++){
        int64_t n = *(int64_t*)(p1RxBuf+i);
        int64_t data = be64toh(n);
        if(addAndOrder(data)){
            #ifdef DBGINFO
            fprintf(stdout, "Got new top N of %"PRId64"\n" , data);
            showStorage();
            #endif
            //re-transmit the data to Prog 3
            int conStat = send(sfd3Live, (void*)&n, sizeof(int64_t), 0);
            if(conStat <= 0){
                fprintf(stderr, "Error sending data (%d): %s\n", errno,
                        strerror(errno));
                return -1;
            }
        }else{
            #ifdef DBGINFO
            fprintf(stdout, "ignored %"PRId64"\n" , data);
            #endif
        }
    }
    return 0;
}

int processP3(int sfd3Live){
    uint8_t p3RxBuf[P3BUFSIZE];
    int r = recv(sfd3Live, p3RxBuf, sizeof(uint8_t)*P3BUFSIZE, 0);
    if(r <= 0){
        return -1;
    }
    for(int i = 0; i < r/sizeof(uint8_t); i++){
        int8_t n = *(int8_t*)(p3RxBuf+i);
        #ifdef DBGINFO
        fprintf(stdout, "Got %d from Prog 3, resetting storage", n);
        #endif
        resizeStorage(n);
    }
    return 0;
}

void clearConnections(){
    if(sfd1Live >= 0){
        close(sfd1Live);
        sfd1Live = -1;
    }
    if(sfd3Live >= 0){
        close(sfd3Live);
        sfd3Live = -1;
    }
}

void intHandle(int sig){
    fprintf(stderr, "Caught SIGINT (Ctrl-C), exiting\n");
    clearConnections();
    close(sfd1);
    close(sfd3);
    freeaddrinfo(P1res);
    freeaddrinfo(P3res);
    exit(EXIT_SUCCESS);
}

int main(){
    //Initialize the storage array
    int64_t unfilled = MIN64;
    for(int i = 0; i < STORESIZE; i++){
        storage[i] = unfilled;
    }

    //Initialize network data structures
    struct addrinfo P1hints, P3hints;
    initSrvSocket(&P1hints, &P1res, P12port, &sfd1, BACKLOG_DEPTH);
    initSrvSocket(&P3hints, &P3res, P32port, &sfd3, BACKLOG_DEPTH);
    fd_set socks;

    signal(SIGINT, intHandle);

    while(1){
        //obtain connections to prog1 and prog3
        while((sfd1Live < 0) || (sfd3Live < 0)){
            if(sfd1Live < 0){
                connectSrvSocket(sfd1, &sfd1Live, "P1");
            }
            if(sfd3Live < 0){
                connectSrvSocket(sfd3, &sfd3Live, "P3");
            }
            usleep(500000);
        }

        //main loop - check for input on either socket and process. If either
        //socket indicates error, clear both and return to outer connection
        //loop
        while(1){
            FD_ZERO(&socks);
            FD_SET(sfd3Live, &socks);
            FD_SET(sfd1Live, &socks);
            int highestfd = sfd1Live > sfd3Live ? sfd1Live : sfd3Live;
            int r = select(highestfd+1, &socks, NULL, NULL, NULL);
            if(r <= 0){
                clearConnections();
                break;
            }
            if(r){ //data available
                if(FD_ISSET(sfd3Live, &socks)){
                    if(processP3(sfd3Live) < 0){
                        clearConnections();
                        break;
                    }
                }
                if(FD_ISSET(sfd1Live, &socks)){
                    if(processP1(sfd1Live, sfd3Live) < 0){
                        clearConnections();
                        break;
                    }
                }
            }
        }
    }
    return 0;
}
