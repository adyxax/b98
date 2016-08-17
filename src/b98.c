#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "b98.h"
#include "dbg.h"
#include "tls.h"

pid_t pid = 0;
int from_b98[2];
int to_b98[2];

#define PARENT_READ  from_b98[0]
#define CHILD_WRITE  from_b98[1]
#define CHILD_READ   to_b98[0]
#define PARENT_WRITE to_b98[1]

int bot_init(void)
{
    if (pipe (from_b98)) {
        perror("from_b98 pipe");
        return 1;
    }
    if (pipe (to_b98)) {
        perror("to_b98 pipe");
        return 1;
    }
    pid = fork ();
    if (pid == (pid_t) 0) {  // Child process
        close(PARENT_WRITE);
        close(PARENT_READ);

        dup2(CHILD_READ,  STDIN_FILENO);  close(CHILD_READ);
        dup2(CHILD_WRITE, STDOUT_FILENO);  close(CHILD_WRITE);

        execvp("fungi", (char *[]) { "fungi", "bot.b98" } );
        perror("execv");
        return 1;
    }
    else if (pid < (pid_t) 0) {
        fprintf (stderr, "Fork failed.\n");
        return 1;
    }
    else {  // Parent process
        close(CHILD_READ);
        close(CHILD_WRITE);
        return 0;
    }
}

int bot_loop(int socket_fd)
{
    fd_set rfds;
    while (1) {
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        FD_ZERO(&rfds);
        FD_SET(PARENT_READ, &rfds);
        FD_SET(socket_fd, &rfds);
        select(20, &rfds, NULL, NULL, &timeout);
        if ( FD_ISSET(PARENT_READ, &rfds) ) {
            static char msg[512]; /* string buffer */
            static int pos = 0; /* position in buffer */
            char sbuf[512];
            ssize_t len = read(PARENT_READ, sbuf, sizeof(sbuf));
            for (int n = 0; n < len; n++) {
                msg[pos] = sbuf[n];

                if ((pos > 0 && msg[pos] == '\n' && msg[pos - 1] == '\r') ) { /* If we got a full message */
                    tls_send(msg, pos + 1);

                    msg[pos - 1] = '\0';
                    printf(">>> %s\n", msg);
                    pos = 0; /* we reinitialise n for the next run */
                } else if (pos == sizeof(msg)) { /* If we got a full buffer without finding a \r\n */
                    fprintf(stderr, "We got a full buffer without finding a \r\n");
                    return 2;
                } else { /* Nothing to do otherwise but roll the next char */
                    pos++;
                }
            }
        } else if ( FD_ISSET(socket_fd, &rfds) ) {
            tls_read(&bot_handle_input);
        }
    }
}

int bot_handle_input(const char* const sbuf, int len)
{
    static char msg[512];
    static int pos = 0; /* position in msg buffer */
    for (int n = 0; n < len; n++) {
        msg[pos] = sbuf[n];
        if ((pos > 0 && msg[pos] == '\n' && msg[pos - 1] == '\r') ) { /* If we got a full message */
            write(PARENT_WRITE, msg, pos + 1);
            msg[pos - 1] = '\0';
            printf("<<< %s\n", msg);
            pos = 0; /* we reinitialise pos for the next run */
        } else if (pos == sizeof(msg)) { /* If we got a full buffer without finding a \r\n */
            fprintf(stderr, "We got a full buffer without finding a \r\n");
            return 2;
        } else { /* Nothing to do otherwise but roll the next char */
            pos++;
        }
    }

    return 0;
}
