#ifndef _DBG_H_
#define _DBG_H_

#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#define DBG(fmt, ...) \
    do { \
        /*fprintf(stderr, "%s:%d:%lu " fmt "\n", __FILE__, __LINE__, pthread_self(), ##__VA_ARGS__);*/ \
        fprintf(stderr, "%s:%d:%lu " fmt "\n", __FILE__, __LINE__, syscall(SYS_gettid), ##__VA_ARGS__); \
    } while (0)

#endif
