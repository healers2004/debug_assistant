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

    // Parse command line arguments using getopt
    while ((opt = getopt(argc, argv, "s:e:l:")) != -1) {
        switch (opt) {
            case 's':
                signal_num = atoi(optarg);
                break;
            case 'e':
                err_val = atoi(optarg);
                break;
            case 'l':
                log_file = optarg;
                break;
            default:
                print_usage(argv[0]);
                return EXIT_FAILURE;
        }
    }

    // Initialize modules and analyze failure
    FailureReport report;
    int result = evaluate_failure(signal_num, err_val, log_file, &report);

    if (result != 0) {
        fprintf(stderr, "Error: Failed to evaluate failure\n");
        return EXIT_FAILURE;
    }

    // Print output
    printf("=== Failure Analysis Report ===\n");
    printf("Failure Type: %s\n", failure_type_to_string(report.failure_type));
    printf("Root Cause: %s\n", report.root_cause);
    printf("Debug Steps: %s\n", report.debug_steps);
    printf("===============================\n");

    return EXIT_SUCCESS;
}

