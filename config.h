/* debug.h: Debugging Macros */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define DATA_SIZE 2400

#define debug(M, ...) \
    fprintf(stderr, "DEBUG %s:%d:%s: " M "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define ERROR \
    { \
    fprintf(stderr, "[ERROR] %s\n", strerror(errno)); \
    debug(); \
    exit(EXIT_FAILURE); \
    }

#define check(r) \
    if (r < 0) {\
        ERROR;\
    }\


