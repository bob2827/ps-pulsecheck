
int initSrvSocket(addrinfo *hints, addrinfo **res, char* port, int* sfd,
                  int backlog)
{
    memset(hints, 0, sizeof(struct addrinfo));

    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    hints->ai_flags = AI_PASSIVE;
    getaddrinfo(NULL, port, P1hints, res);
    *sfd = socket(res->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
    fcntl(*sfd, F_SETFL, O_NONBLOCK);
    bind(*sfd, (*res)->ai_addr, (*res)->ai_addrlen);
    listen(*sfd, BACKLOG_DEPTH);
}

int initClientSocket(addrinfo *hints, addrinfo **res, char* host, char* port,
                     int* sfd)
{
    memset(&hints, 0, sizeof(struct addrinfo));
    hints->ai_family = AF_INET;
    hints->ai_socktype = SOCK_STREAM;
    getaddrinfo(host, port, hints, res);
    *sfd = socket((*res)->ai_family, (*res)->ai_socktype, (*res)->ai_protocol);
}
