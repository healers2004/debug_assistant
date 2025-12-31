#ifndef FAILURE_RULES_H
#define FAILURE_RULES_H

#include <stddef.h>

typedef enum {
    FAILURE_MEMORY_CORRUPTION,
    FAILURE_INVALID_STATE,
    FAILURE_RESOURCE_EXHAUSTION,
    FAILURE_TIMING_RACE
} FailureType;

typedef struct {
    FailureType failure_type;
    const char *root_cause;
    const char *debug_steps;
} FailureReport;

/**
 * Evaluates failure based on signal, errno, and log data.
 * Populates the FailureReport structure.
 * @param signal_num Signal number (if available, -1 otherwise)
 * @param err_val Errno value (if available, 0 otherwise)
 * @param log_file Path to log file (if available, NULL otherwise)
 * @param report Output parameter to be populated with failure analysis
 * @return 0 on success, non-zero on error
 */
int evaluate_failure(int signal_num, int err_val, const char *log_file, FailureReport *report);

#endif /* FAILURE_RULES_H */

