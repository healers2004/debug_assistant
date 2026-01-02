/* Test program: Causes SIGABRT by calling abort() */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("About to abort...\n");
    abort();  /* Abort signal */
    return 0;
}

