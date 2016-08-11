#ifndef _TLS_H_
#define _TLS_H_

#include <gnutls/gnutls.h>

void tls_init(void);
int tls_connect(const char* const host, const unsigned short port);
int tls_read(int (*input_handler_callback)(const char* const sbuf, const int len));
void tls_send(char *buf, int len);
void tls_close(void);
void tls_clean(void);

#endif
