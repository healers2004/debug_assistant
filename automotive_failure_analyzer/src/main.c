#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "signal_analyzer.h"
#include "errno_mapper.h"
#include "log_parser.h"
#include "failure_rules.h"
#include "process_runner.h"

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
    fprintf(stderr, "Usage: %s [-s <signal>] [-e <errno>] [-l <log_file>] [--run <program> [args...]]\n", program_name);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -s <int>       Signal number (e.g., 11 for SIGSEGV)\n");
    fprintf(stderr, "  -e <int>       Errno value (e.g., 14 for EFAULT)\n");
    fprintf(stderr, "  -l <path>      Path to log file\n");
    fprintf(stderr, "  --run <prog>   Run and monitor a program\n");
}

int main(int argc, char *argv[]) {
    int signal_num = -1;
    int err_val = 0;
    const char *log_file = NULL;
    int use_run_mode = 0;
    char *run_program = NULL;
    char **run_args = NULL;
    int run_args_count = 0;

    /* Manual argument parsing to handle --run specially */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--run") == 0) {
            /* Found --run, extract program and its arguments */
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: --run requires a program name\n");
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            use_run_mode = 1;
            run_program = argv[i + 1];
            /* Collect remaining arguments as program arguments */
            if (i + 2 < argc) {
                run_args_count = argc - (i + 2);
                run_args = &argv[i + 2];
            }
            /* Skip processing --run and its arguments */
            i = argc;  /* Exit loop */
        } else if (argv[i][0] == '-' && strlen(argv[i]) == 2) {
            /* Short option */
            if (i + 1 >= argc) {
                fprintf(stderr, "Error: Option %s requires an argument\n", argv[i]);
                print_usage(argv[0]);
                return EXIT_FAILURE;
            }
            switch (argv[i][1]) {
                case 's':
                    signal_num = atoi(argv[i + 1]);
                    if (signal_num <= 0 || signal_num > 64) {
                        fprintf(stderr, "Error: Invalid signal number: %d (valid range: 1-64)\n", signal_num);
                        return EXIT_FAILURE;
                    }
                    i++;  /* Skip argument */
                    break;
                case 'e':
                    err_val = atoi(argv[i + 1]);
                    if (err_val < 0 || err_val > 255) {
                        fprintf(stderr, "Error: Invalid errno value: %d (valid range: 0-255)\n", err_val);
                        return EXIT_FAILURE;
                    }
                    i++;  /* Skip argument */
                    break;
                case 'l':
                    log_file = argv[i + 1];
                    i++;  /* Skip argument */
                    break;
                default:
                    fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
                    print_usage(argv[0]);
                    return EXIT_FAILURE;
            }
        } else if (argv[i][0] == '-' && argv[i][1] == '-') {
            /* Long option (other than --run, which is handled above) */
            fprintf(stderr, "Error: Unknown option: %s\n", argv[i]);
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    /* Handle --run mode */
    if (use_run_mode) {
        if (run_program == NULL) {
            fprintf(stderr, "Error: --run requires a program name\n");
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }

        /* Prepare arguments for target program */
        char **target_args;
        int target_argc = run_args_count + 2;  /* program name + args + NULL */
        target_args = malloc(target_argc * sizeof(char *));
        if (target_args == NULL) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            return EXIT_FAILURE;
        }

        target_args[0] = run_program;
        if (run_args_count > 0 && run_args != NULL) {
            for (int i = 0; i < run_args_count; i++) {
                target_args[i + 1] = run_args[i];
            }
        }
        target_args[target_argc - 1] = NULL;

        /* Run and monitor the program */
        ProcessResult proc_result;
        int run_result = run_and_monitor(run_program, target_args, &proc_result);
        free(target_args);

        if (run_result != 0) {
            fprintf(stderr, "Failed to execute target program.\n");
            fprintf(stderr, "Reason: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }

        if (!proc_result.ran_successfully) {
            fprintf(stderr, "Failed to execute target program.\n");
            fprintf(stderr, "Reason: Program launch failed\n");
            return EXIT_FAILURE;
        }

        /* Check termination status */
        if (proc_result.exited_normally && proc_result.exit_code == 0) {
            printf("Program exited normally. No failure detected.\n");
            return EXIT_SUCCESS;
        }

        /* Check for exec failure (exit code 127 typically indicates command not found) */
        if (proc_result.exited_normally && proc_result.exit_code == 127) {
            fprintf(stderr, "Failed to execute target program.\n");
            fprintf(stderr, "Reason: Command not found or exec failed\n");
            return EXIT_FAILURE;
        }

        /* Program failed - analyze the failure */
        if (proc_result.terminated_by_signal) {
            signal_num = proc_result.signal_number;
            printf("\nObserved Termination:\n");
            printf("- Signal: %d", signal_num);
            
            /* Check if signal is known */
            const SignalInfo *sig_info = analyze_signal(signal_num);
            if (sig_info != NULL) {
                printf(" (%s)\n", sig_info->name);
            } else {
                printf(" (Unknown)\n");
            }
            printf("- Core dump: %s\n\n", proc_result.core_dumped ? "yes" : "no");
        } else if (proc_result.exited_normally) {
            /* Non-zero exit code */
            printf("\nObserved Termination:\n");
            printf("- Exit code: %d\n", proc_result.exit_code);
            printf("- Signal: none\n\n");
            printf("Failure detected, but no terminating signal was reported.\n");
            printf("Classification: Unknown Failure\n");
            return EXIT_SUCCESS;
        } else {
            /* Unknown termination state */
            printf("\nObserved Termination:\n");
            printf("- Termination state: unknown\n\n");
            printf("Failure detected, but no terminating signal was reported.\n");
            printf("Classification: Unknown Failure\n");
            return EXIT_SUCCESS;
        }
    }

    /* V1 mode: Validate that at least one input was provided */
    if (!use_run_mode && signal_num == -1 && err_val == 0 && log_file == NULL) {
        fprintf(stderr, "Error: At least one of -s, -e, -l, or --run must be provided\n\n");
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    /* Handle unknown signals in run mode */
    if (use_run_mode && signal_num != -1) {
        const SignalInfo *sig_info = analyze_signal(signal_num);
        if (sig_info == NULL) {
            /* Unknown signal */
            printf("Unknown signal encountered (signal %d)\n\n", signal_num);
            printf("=== Failure Analysis Report ===\n\n");
            printf("Failure Type: Unknown Failure\n");
            printf("Root Cause:   Signal %d is not in the supported signal set\n", signal_num);
            printf("\nDebug Steps:\n");
            printf("This failure does not match known classifications.\n");
            printf("Review signal %d documentation and investigate manually.\n", signal_num);
            printf("================================\n\n");
            return EXIT_SUCCESS;
        }
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

