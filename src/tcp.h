#ifndef _TCP_H_
#define _TCP_H_

int hostname_to_ip(const char* const hostname, char *ip);
int tcp_connect(const char* const host, const int port);
void tcp_close(const int sd);

#endif
