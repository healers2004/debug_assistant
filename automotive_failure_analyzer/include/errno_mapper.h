#ifndef ERRNO_MAPPER_H
#define ERRNO_MAPPER_H

#include <errno.h>
#include <stddef.h>

typedef enum {
    ERRNO_EFAULT = EFAULT,
    ERRNO_EINVAL = EINVAL,
    ERRNO_ENOMEM = ENOMEM,
    ERRNO_EPIPE = EPIPE
} SupportedErrno;

/**
 * Maps an errno value to a failure category string.
 * @param err_val The errno value to map
 * @return Pointer to a string describing the failure category
 */
const char *map_errno(int err_val);

#endif /* ERRNO_MAPPER_H */

