#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdint.h>

int main(){
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    char data[1024] = "hello data\n";

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo("localhost", "10001", &hints, &res);
    sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    connect(sfd, res->ai_addr, res->ai_addrlen);

    signal(SIGINT, intHandle);

    while(1){
        int64_t a = rand();
        int64_t b = rand();
        int64_t n = (a << 32) | b;
        snprintf(data, 1024, "%"PRId64"\n", n);
        send(sfd, data, strlen(data), 0);
        usleep(250000);
    }
    return 0;

}
