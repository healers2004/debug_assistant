#include "failure_rules.h"
#include "signal_analyzer.h"
#include "errno_mapper.h"
#include "log_parser.h"
#include <string.h>

static const char *failure_type_strings[] = {
    "Memory Corruption",
    "Invalid State",
    "Resource Exhaustion",
    "Timing/Race"
};

int evaluate_failure(int signal_num, int err_val, const char *log_file, FailureReport *report) {
    if (report == NULL) {
        return -1;
    }

    // Initialize report
    report->failure_type = FAILURE_MEMORY_CORRUPTION;
    report->root_cause = "Unknown";
    report->debug_steps = "No debug steps available";

    // Analyze signal if provided
    if (signal_num != -1) {
        const SignalInfo *signal_info = analyze_signal(signal_num);
        if (signal_info != NULL) {
            if (signal_num == SIGSEGV || signal_num == SIGBUS) {
                report->failure_type = FAILURE_MEMORY_CORRUPTION;
                report->root_cause = signal_info->description;
                report->debug_steps = "Check memory access patterns, use valgrind";
            } else if (signal_num == SIGABRT) {
                report->failure_type = FAILURE_INVALID_STATE;
                report->root_cause = signal_info->description;
                report->debug_steps = "Check assertion failures, review abort conditions";
            } else if (signal_num == SIGFPE) {
                report->failure_type = FAILURE_INVALID_STATE;
                report->root_cause = signal_info->description;
                report->debug_steps = "Check arithmetic operations, division by zero";
            }
        }
    }

    // Analyze errno if provided
    if (err_val != 0) {
        const char *errno_category = map_errno(err_val);
        if (strcmp(errno_category, "Memory Corruption") == 0) {
            report->failure_type = FAILURE_MEMORY_CORRUPTION;
            report->root_cause = errno_category;
        } else if (strcmp(errno_category, "Resource Exhaustion") == 0) {
            report->failure_type = FAILURE_RESOURCE_EXHAUSTION;
            report->root_cause = errno_category;
            report->debug_steps = "Check resource limits, memory allocation";
        } else if (strcmp(errno_category, "Invalid State") == 0) {
            report->failure_type = FAILURE_INVALID_STATE;
            report->root_cause = errno_category;
        }
    }

    // Parse log file if provided
    if (log_file != NULL) {
        parse_log_file(log_file);
        // Future: extract additional context from log file
    }

    return 0;
}

