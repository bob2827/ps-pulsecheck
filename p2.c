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

#define P1BUFSIZE 10
#define P3BUFSIZE 10

//The largest possible cache is under 2K, so I think statically allocated
//memory is accpetable here. If we had some significant maxium size I'd probably
//dynamically size this as changes were submitted by prog 3
#define STORESIZE 255
int64_t storage[STORESIZE];
unsigned char N = 20;

int sfd1 = -1, sfd3 = -1;
int sfd1Live = -1, sfd3Live = -1;
fd_set socks;

int64_t p1RxBuf[P1BUFSIZE];
uint8_t p3RxBuf[P3BUFSIZE];

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
        fprintf(stderr, "%"PRId64", ", storage[i]);
    }
    fprintf(stderr, "\n");
}

int main(){
    //Initialize the storage array
    int64_t unfilled = MIN64;
    for(int i = 0; i < STORESIZE; i++){
        storage[i] = unfilled;
    }

    //Initialize network data structures
    struct addrinfo P1hints, P3hints, *P1res, *P3res;

    initSrvSocket(&P1hints, &P1res, P12port, &sfd1, BACKLOG_DEPTH);
    initSrvSocket(&P3hints, &P3res, P32port, &sfd3, BACKLOG_DEPTH);
    FD_ZERO(&socks);

    while(1){
        while((sfd1Live < 0) || (sfd3Live < 0)){
            if(sfd1Live < 0){
                connectSrvSocket(sfd1, &sfd1Live);
            }
            if(sfd3Live < 0){
                connectSrvSocket(sfd3, &sfd3Live);
            }
            usleep(500000);
        }
        FD_SET(sfd3Live, &socks);
        FD_SET(sfd1Live, &socks);
        int highestfd = sfd1Live > sfd3Live ? sfd1Live : sfd3Live;

        while(1){
            int r = select(highestfd+1, &socks, NULL, NULL, NULL);
            if(r < 0){
                //error
                fprintf(stderr, "Unhandled RX error on one of the sockets, exiting.\n");
                exit(EXIT_FAILURE);
            }
            if(r){
                //data available
                if(FD_ISSET(sfd3Live, &socks)){
                    int r = recv(sfd3Live, p3RxBuf, sizeof(uint8_t)*P3BUFSIZE, 0);
                    fprintf(stderr, "got new buffer size assignment\n");
                    for(int i = 0; i < r/sizeof(uint8_t); i++){
                        int8_t n = *(int8_t*)(p3RxBuf+i);
                        fprintf(stdout, "Got %d from Prog 3, resetting storage", n);
                        resizeStorage(n);
                    }
                }
                if(FD_ISSET(sfd1Live, &socks)){
                    int r = recv(sfd1Live, p1RxBuf, sizeof(int64_t)*P1BUFSIZE, 0);
                    for(int i = 0; i < r/sizeof(int64_t); i++){
                        int64_t n = *(int64_t*)(p1RxBuf+i);
                        int64_t data = be64toh(n);
                        //printf("%"PRId64"\n", data);
                        if(addAndOrder(data)){
                            fprintf(stderr, "Got new top N of %"PRId64"\n" , data);
                            showStorage();
                            //re-transmit the data to Prog 3
                            int conStat = send(sfd3Live, (void*)&n, sizeof(int64_t), 0);
                            if(conStat <= 0){
                                fprintf(stderr,
                                    "Error sending data (%d): %s\n", errno,
                                    strerror(errno));
                            }
                        }else{
                            fprintf(stderr, "dropped %"PRId64"\n" , data);
                        }
                    }
                }
            }
            if(!r){
                printf("select() without timeot returned 0, something is wrong\n");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}
