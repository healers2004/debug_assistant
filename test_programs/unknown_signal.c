/* Test program: Raises SIGKILL (signal 9) which is not in supported signal set */
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int main(void) {
    printf("Raising SIGKILL (signal 9)...\n");
    kill(getpid(), SIGKILL);  /* Kill signal - not in supported set */
    return 0;
}

