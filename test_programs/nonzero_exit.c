/* Test program: Exits normally with non-zero exit code */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Program failed with exit code 1.\n");
    return 1;
}

