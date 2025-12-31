#ifndef SIGNAL_ANALYZER_H
#define SIGNAL_ANALYZER_H

#include <signal.h>
#include <stddef.h>

typedef enum {
    SUPPORTED_SIGSEGV = SIGSEGV,
    SUPPORTED_SIGABRT = SIGABRT,
    SUPPORTED_SIGBUS = SIGBUS,
    SUPPORTED_SIGFPE = SIGFPE
} SupportedSignal;

typedef struct {
    int signal_num;
    const char *name;
    const char *description;
} SignalInfo;

/**
 * Analyzes a signal number and returns information about it.
 * @param signal_num The signal number to analyze
 * @return Pointer to SignalInfo struct containing signal details
 */
const SignalInfo *analyze_signal(int signal_num);

#endif /* SIGNAL_ANALYZER_H */

