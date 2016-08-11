#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "tcp.h"

int tcp_connect(const char* const host, const int port)
{
    char ip[INET6_ADDRSTRLEN];
    hostname_to_ip(host , ip);
    printf("%s resolved to %s\n" , host, ip);

    struct sockaddr_in sa;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sa, '\0', sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, ip, &sa.sin_addr);

    int err = connect(sd, (struct sockaddr *) &sa, sizeof(sa));
    if (err < 0) {
        fprintf(stderr, "Couldn't connect %s:%d : %s\n", host, port, strerror(errno));
        return -1;
    }

    return sd;
}

void tcp_close(const int sd)
{
    shutdown(sd, SHUT_RDWR);
    close(sd);
}

int hostname_to_ip(const char* const hostname, char *ip)
{
    struct addrinfo hints, *servinfo; //, *p;
    struct sockaddr_in *h;
    int rv, ret = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;

    if ( (rv = getaddrinfo( hostname , "http" , &hints , &servinfo)) != 0) 
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        ret = 1;
    } else {
        //// loop through all the results and connect to the first we can
        //for(p = servinfo; p != NULL; p = p->ai_next)
        //{
        //    h = (struct sockaddr_in *) p->ai_addr;
        //    strcpy(ip , inet_ntoa( h->sin_addr ) );
        //}
        h = (struct sockaddr_in *) servinfo->ai_addr;
        strcpy(ip, inet_ntoa(h->sin_addr));
    }

    freeaddrinfo(servinfo); // all done with this structure
    return ret;
}
