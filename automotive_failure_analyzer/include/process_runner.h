#ifndef PROCESS_RUNNER_H
#define PROCESS_RUNNER_H

#include <stddef.h>

typedef struct {
    int ran_successfully;      /* Program launched successfully */
    int exited_normally;       /* Program called exit() */
    int exit_code;             /* Valid only if exited_normally == 1 */
    int terminated_by_signal;  /* Program was killed by signal */
    int signal_number;         /* Valid only if terminated_by_signal == 1 */
    int core_dumped;           /* Core dump was generated */
} ProcessResult;

/**
 * Runs a target program and monitors its termination.
 * Uses fork(), execvp(), and waitpid() to observe process behavior.
 * @param program Path to the program to execute
 * @param args Array of arguments (program name + args, terminated by NULL)
 * @param result Output parameter to be populated with termination metadata
 * @return 0 on success, non-zero on error (fork/exec failure)
 */
int run_and_monitor(char *program, char **args, ProcessResult *result);

#endif /* PROCESS_RUNNER_H */

