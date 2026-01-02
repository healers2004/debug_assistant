/* Test program: Causes SIGBUS by misaligned memory access (architecture-dependent) */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    /* Attempt to cause bus error through misaligned access */
    /* Note: This may not work on all architectures */
    char *p = malloc(10);
    if (p == NULL) {
        return 1;
    }
    
    /* On some architectures, accessing misaligned pointers causes SIGBUS */
    int *unaligned = (int *)(p + 1);
    *unaligned = 0x12345678;  /* May cause bus error */
    
    free(p);
    return 0;
}

