#ifndef _B98_H_
#define _B98_H_

int bot_init(void);
int bot_loop(int socket_fd);
int bot_handle_input(const char* const sbuf, int len);

#endif
