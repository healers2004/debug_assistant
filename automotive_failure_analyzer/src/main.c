#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "signal_analyzer.h"
#include "errno_mapper.h"
#include "log_parser.h"
#include "failure_rules.h"

static const char *failure_type_to_string(FailureType type) {
    switch (type) {
        case FAILURE_MEMORY_CORRUPTION:
            return "Memory Corruption";
        case FAILURE_INVALID_STATE:
            return "Invalid State";
        case FAILURE_RESOURCE_EXHAUSTION:
            return "Resource Exhaustion";
        case FAILURE_TIMING_RACE:
            return "Timing/Race";
        default:
            return "Unknown";
    }
}

void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s -s <signal> -e <errno> -l <log_file>\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -s <int>    Signal number (e.g., 11 for SIGSEGV)\n");
    fprintf(stderr, "  -e <int>    Errno value (e.g., 14 for EFAULT)\n");
    fprintf(stderr, "  -l <path>   Path to log file\n");
}

int main(int argc, char *argv[]) {
    int signal_num = -1;
    int err_val = 0;
    const char *log_file = NULL;
    int opt;

    /* Parse command line arguments using getopt */
    while ((opt = getopt(argc, argv, "s:e:l:")) != -1) {
        switch (opt) {
            case 's':
                signal_num = atoi(optarg);
                if (signal_num <= 0 || signal_num > 64) {
                    fprintf(stderr, "Error: Invalid signal number: %d (valid range: 1-64)\n", signal_num);
                    return EXIT_FAILURE;
                }
                break;
            case 'e':
                err_val = atoi(optarg);
                if (err_val < 0 || err_val > 255) {
                    fprintf(stderr, "Error: Invalid errno value: %d (valid range: 0-255)\n", err_val);
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                log_file = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    /* Validate that at least one input was provided */
    if (signal_num == -1 && err_val == 0 && log_file == NULL) {
        fprintf(stderr, "Error: At least one of -s, -e, or -l must be provided\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Initialize modules and analyze failure */
    FailureReport report;
    int result = evaluate_failure(signal_num, err_val, log_file, &report);

    if (result != 0) {
        fprintf(stderr, "Error: Failed to evaluate failure\n");
        return EXIT_FAILURE;
    }

    /* Print structured output */
    printf("\n=== Failure Analysis Report ===\n\n");
    printf("Failure Type: %s\n", failure_type_to_string(report.failure_type));
    printf("Root Cause:   %s\n", report.root_cause);
    printf("\nDebug Steps:\n%s\n", report.debug_steps);
    printf("================================\n\n");

    return EXIT_SUCCESS;
}

