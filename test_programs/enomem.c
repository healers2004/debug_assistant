/* Test program: Attempts to trigger ENOMEM by allocating excessive memory */
/* Note: This may not always trigger ENOMEM depending on system limits */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(void) {
    size_t huge_size = (size_t)-1;  /* Maximum size_t value */
    void *p = malloc(huge_size);
    
    if (p == NULL) {
        if (errno == ENOMEM) {
            printf("ENOMEM detected: %s\n", strerror(errno));
            return 0;  /* Successfully triggered ENOMEM */
        } else {
            printf("malloc failed with errno: %d\n", errno);
            return 1;
        }
    }
    
    free(p);
    printf("Warning: ENOMEM not triggered (system allowed allocation)\n");
    return 1;
}

