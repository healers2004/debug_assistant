#include "failure_rules.h"
#include "signal_analyzer.h"
#include "errno_mapper.h"
#include "log_parser.h"
#include <string.h>

/* Static strings for root causes and debug steps */
static const char *ROOT_CAUSE_MEMORY_CORRUPTION = "Invalid memory access - null pointer dereference or buffer overflow";
static const char *ROOT_CAUSE_INVALID_STATE = "Invalid operation or state violation";
static const char *ROOT_CAUSE_RESOURCE_EXHAUSTION = "System resource limit exceeded";
static const char *ROOT_CAUSE_TIMING_RACE = "Concurrency issue - race condition or deadlock";

static const char *DEBUG_STEPS_MEMORY = "1. Run with valgrind: valgrind --leak-check=full <program>\n2. Use AddressSanitizer: gcc -fsanitize=address <sources>\n3. Check stack traces with gdb: gdb <program> core\n4. Review pointer arithmetic and array bounds";
static const char *DEBUG_STEPS_INVALID_STATE = "1. Review assertion failures and abort conditions\n2. Check function preconditions and state validation\n3. Enable core dumps: ulimit -c unlimited\n4. Use strace to trace system calls";
static const char *DEBUG_STEPS_RESOURCE = "1. Check memory limits: ulimit -v\n2. Monitor resource usage: top, ps aux\n3. Review memory allocation patterns\n4. Check for memory leaks with valgrind --leak-check=full";
static const char *DEBUG_STEPS_TIMING = "1. Review thread synchronization (mutexes, semaphores)\n2. Use thread sanitizer: gcc -fsanitize=thread <sources>\n3. Add logging around critical sections\n4. Check for deadlock patterns in code";

int evaluate_failure(int signal_num, int err_val, const char *log_file, FailureReport *report) {
    if (report == NULL) {
        return -1;
    }

    /* Initialize with defaults */
    report->failure_type = FAILURE_MEMORY_CORRUPTION;
    report->root_cause = "Insufficient information to determine root cause";
    report->debug_steps = "Provide signal number (-s) or errno value (-e) for analysis";

    /* Parse log file first to get context */
    LogAnalysis log_analysis = {0, 0, 0, 0};
    if (log_file != NULL) {
        if (parse_log_file(log_file, &log_analysis) != 0) {
            /* Log file parsing failed, continue with signal/errno analysis */
        }
    }

    /* Deterministic rule-based evaluation: combine signal + errno + log context */

    /* Rule 1: SIGSEGV (11) or SIGBUS (7) -> Memory Corruption */
    if (signal_num == SIGSEGV || signal_num == SIGBUS) {
        report->failure_type = FAILURE_MEMORY_CORRUPTION;
        const SignalInfo *sig_info = analyze_signal(signal_num);
        if (sig_info != NULL) {
            report->root_cause = sig_info->description;
        } else {
            report->root_cause = ROOT_CAUSE_MEMORY_CORRUPTION;
        }
        report->debug_steps = DEBUG_STEPS_MEMORY;

        /* Refine with errno if available */
        if (err_val == EFAULT) {
            report->root_cause = "Invalid memory access - bad address (EFAULT)";
        }
        return 0;
    }

    /* Rule 2: SIGFPE (8) -> Invalid State */
    if (signal_num == SIGFPE) {
        report->failure_type = FAILURE_INVALID_STATE;
        const SignalInfo *sig_info = analyze_signal(signal_num);
        if (sig_info != NULL) {
            report->root_cause = sig_info->description;
        } else {
            report->root_cause = ROOT_CAUSE_INVALID_STATE;
        }
        report->debug_steps = DEBUG_STEPS_INVALID_STATE;
        return 0;
    }

    /* Rule 3: SIGABRT (6) -> Invalid State (typically assertion failure) */
    if (signal_num == SIGABRT) {
        report->failure_type = FAILURE_INVALID_STATE;
        const SignalInfo *sig_info = analyze_signal(signal_num);
        if (sig_info != NULL) {
            report->root_cause = sig_info->description;
        } else {
            report->root_cause = "Assertion failure or abort() call";
        }
        report->debug_steps = DEBUG_STEPS_INVALID_STATE;
        return 0;
    }

    /* Rule 4: ENOMEM (12) -> Resource Exhaustion */
    if (err_val == ENOMEM) {
        report->failure_type = FAILURE_RESOURCE_EXHAUSTION;
        report->root_cause = ROOT_CAUSE_RESOURCE_EXHAUSTION;
        report->debug_steps = DEBUG_STEPS_RESOURCE;
        return 0;
    }

    /* Rule 5: EFAULT (14) -> Memory Corruption */
    if (err_val == EFAULT) {
        report->failure_type = FAILURE_MEMORY_CORRUPTION;
        report->root_cause = "Invalid memory address passed to system call (EFAULT)";
        report->debug_steps = DEBUG_STEPS_MEMORY;
        return 0;
    }

    /* Rule 6: EINVAL (22) or EPIPE (32) -> Invalid State */
    if (err_val == EINVAL || err_val == EPIPE) {
        report->failure_type = FAILURE_INVALID_STATE;
        if (err_val == EINVAL) {
            report->root_cause = "Invalid argument passed to system call (EINVAL)";
        } else {
            report->root_cause = "Broken pipe - write to closed file descriptor (EPIPE)";
        }
        report->debug_steps = DEBUG_STEPS_INVALID_STATE;
        return 0;
    }

    /* Rule 7: Log-based detection (timeout/deadlock keywords) */
    if (log_analysis.has_timeout_keywords) {
        report->failure_type = FAILURE_TIMING_RACE;
        report->root_cause = ROOT_CAUSE_TIMING_RACE;
        report->debug_steps = DEBUG_STEPS_TIMING;
        return 0;
    }

    /* Rule 8: Log-based detection (resource keywords) */
    if (log_analysis.has_resource_keywords && err_val == 0 && signal_num == -1) {
        report->failure_type = FAILURE_RESOURCE_EXHAUSTION;
        report->root_cause = ROOT_CAUSE_RESOURCE_EXHAUSTION;
        report->debug_steps = DEBUG_STEPS_RESOURCE;
        return 0;
    }

    /* Rule 9: Log-based detection (memory keywords) with no signal/errno */
    if (log_analysis.has_memory_keywords && err_val == 0 && signal_num == -1) {
        report->failure_type = FAILURE_MEMORY_CORRUPTION;
        report->root_cause = ROOT_CAUSE_MEMORY_CORRUPTION;
        report->debug_steps = DEBUG_STEPS_MEMORY;
        return 0;
    }

    /* If we reach here, no clear match - return default */
    return 0;
}

