#include "b98.h"
#include "config.h"
#include "tls.h"

int main()
{
    int err = bot_init();
    if (err)
        return err;

    tls_init();
    int socket_fd = tls_connect(HOST, PORT);

    bot_loop(socket_fd);

    tls_clean();
    return 0;
}
