/* Test program: Causes SIGSEGV by dereferencing NULL pointer */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int *p = NULL;
    *p = 42;  /* Segmentation fault */
    return 0;
}

