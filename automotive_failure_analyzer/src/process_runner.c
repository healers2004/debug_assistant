#define _GNU_SOURCE
#include "process_runner.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int run_and_monitor(char *program, char **args, ProcessResult *result) {
    if (program == NULL || args == NULL || result == NULL) {
        return -1;
    }

    /* Initialize result structure defensively */
    result->ran_successfully = 0;
    result->exited_normally = 0;
    result->exit_code = 0;
    result->terminated_by_signal = 0;
    result->signal_number = 0;
    result->core_dumped = 0;

    pid_t pid = fork();
    if (pid < 0) {
        /* fork() failed */
        return -1;
    }

    if (pid == 0) {
        /* Child process: execute the target program */
        execvp(program, args);
        /* If execvp returns, it failed */
        _exit(127);  /* Standard exit code for exec failure */
    }

    /* Parent process: wait for child to terminate */
    int status;
    pid_t waited_pid = waitpid(pid, &status, 0);

    if (waited_pid < 0) {
        /* waitpid() failed */
        return -1;
    }

    /* Mark that the program ran successfully (we got past fork/exec) */
    result->ran_successfully = 1;

    /* Analyze termination status */
    if (WIFEXITED(status)) {
        /* Program called exit() */
        result->exited_normally = 1;
        result->exit_code = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        /* Program was terminated by a signal */
        result->terminated_by_signal = 1;
        result->signal_number = WTERMSIG(status);
        result->core_dumped = WCOREDUMP(status) ? 1 : 0;
    } else {
        /* Unknown termination state (should not happen normally) */
        /* Leave flags as 0 to indicate unknown state */
    }

    return 0;
}

