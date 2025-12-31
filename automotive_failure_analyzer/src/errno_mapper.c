#include "errno_mapper.h"

const char *map_errno(int err_val) {
    switch (err_val) {
        case EFAULT:
            return "Memory Corruption";
        case EINVAL:
            return "Invalid State";
        case ENOMEM:
            return "Resource Exhaustion";
        case EPIPE:
            return "Invalid State";
        default:
            return "Unknown Error";
    }
}

