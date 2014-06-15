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

void intHandle(int sig);

int sfd = -1;

int main(){
    //char data[1024] = "hello data\n";
    int conStat = -1;
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("localhost", "10000", &hints, &res);
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

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
        int64_t a = rand();
        int64_t b = rand();
        int64_t n = (a << 32) | b;
        printf("%"PRId64"\n", n);
        //It's 2014 and there's no standard hton64? Am I missing something?
        int64_t data = htobe64(n);
        conStat = send(sfd, (void*)&data, sizeof(int64_t), 0);
        printf("Stat: %d\n", conStat);
        if(conStat <= 0){
            fprintf(stderr, "Error sending data (%d): %s\n", errno, strerror(errno));
        }
        usleep(250000);
    }
    return 0;
}

void intHandle(int sig){
    printf("Caught SIGINT (Ctrl-C), exiting\n");
    close(sfd);
    exit(0);
}
