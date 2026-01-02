/* Test program: Causes SIGFPE by division by zero */
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    int a = 10;
    int b = 0;
    int result = a / b;  /* Floating-point exception */
    (void)result;  /* Suppress unused variable warning */
    return 0;
}

